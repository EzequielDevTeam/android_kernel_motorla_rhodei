// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/kmsg_dump.h>

#define BL_PATH "/data/misc/bootlog.txt"
#define BL_RETRY 60

struct bl_ctx {
	struct kmsg_dumper dumper;
	char *buf;
	size_t cap;
	size_t len;
};

static void bl_dump(struct kmsg_dumper *dumper, enum kmsg_dump_reason reason)
{
	struct bl_ctx *c = container_of(dumper, struct bl_ctx, dumper);
	char *line;
	size_t l = 0;

	if (!c->buf)
		return;
	line = kmalloc(1024, GFP_KERNEL);
	if (!line)
		return;
	while (kmsg_dump_get_line_nolock(dumper, true, line, 1024, &l)) {
		size_t n = strlen(line);
		if (c->len + n + 1 >= c->cap)
			break;
		memcpy(c->buf + c->len, line, n);
		c->len += n;
		c->buf[c->len++] = '\n';
		l = 0;
	}
	kfree(line);
}

static void bl_write(const char *path, const char *data, size_t len)
{
	struct file *f;
	loff_t pos = 0;

	f = filp_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (IS_ERR(f))
		return;
	vfs_write(f, data, len, &pos);
	filp_close(f, NULL);
	pr_info("bootlog: wrote %zu bytes to %s\n", len, path);
}

static void bl_work(struct work_struct *w)
{
	struct bl_ctx *c;
	int i;
	struct file *f;

	/* espera /data existir */
	for (i = 0; i < BL_RETRY; i++) {
		f = filp_open(BL_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (!IS_ERR(f)) {
			filp_close(f, NULL);
			break;
		}
		msleep(1500);
	}

	c = kzalloc(sizeof(*c), GFP_KERNEL);
	if (!c)
		return;
	c->cap = 512 * 1024;
	c->buf = kzalloc(c->cap, GFP_KERNEL);
	if (!c->buf) {
		kfree(c);
		return;
	}

	c->dumper.dump = bl_dump;
	c->dumper.max_reason = KMSG_DUMP_POWEROFF;
	if (kmsg_dump_register(&c->dumper) == 0) {
		kmsg_dump(KMSG_DUMP_POWEROFF);
		kmsg_dump_unregister(&c->dumper);
	}
	if (c->len)
		bl_write(BL_PATH, c->buf, c->len);
	kfree(c->buf);
	kfree(c);
}

static int __init bl_init(void)
{
	struct workqueue_struct *wq;
	struct work_struct *w;

	wq = alloc_ordered_workqueue("bootlog", WQ_MEM_RECLAIM);
	if (!wq)
		return -ENOMEM;
	w = kzalloc(sizeof(*w), GFP_KERNEL);
	if (!w)
		return -ENOMEM;
	INIT_WORK(w, bl_work);
	queue_work(wq, w);
	return 0;
}
late_initcall(bl_init);
MODULE_INFO(bootlog, "v1");

MODULE_LICENSE("GPL");
