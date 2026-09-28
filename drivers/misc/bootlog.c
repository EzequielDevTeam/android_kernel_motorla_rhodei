// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/kmsg_dump.h>
#include <soc/qcom/kmsg_dump.h>

#define BOOTLOG_PATH "/data/misc/bootlog.txt"
#define BOOTLOG_RETRY 40

static void bootlog_flush(struct kmsg_dumper *dumper, enum kmsg_dump_type type)
{
	struct file *f;
	char *buf, *p;
	size_t len = 0, cap = 262144;

	buf = kzalloc(cap, GFP_KERNEL);
	if (!buf)
		return;
	p = buf;
	len = kmsg_dump_copy_buffer(dumper, true, p, cap, &len);
	if (len == 0) {
		kfree(buf);
		return;
	}
	f = filp_open(BOOTLOG_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (!IS_ERR(f)) {
		vfs_write(f, buf, len, &file->f_pos);
		filp_close(f, NULL);
		pr_info("bootlog: wrote %zu bytes to %s\n", len, BOOTLOG_PATH);
	}
	kfree(buf);
}

static void bootlog_work(struct work_struct *w)
{
	struct kmsg_dumper dumper;
	int i;

	for (i = 0; i < BOOTLOG_RETRY; i++) {
		struct file *f = filp_open(BOOTLOG_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (!IS_ERR(f)) {
			filp_close(f, NULL);
			break;
		}
		msleep(1500);
	}

	kmsg_dump_register(&dumper, true);
	bootlog_flush(&dumper, KMSG_DUMP_BOOT);
	kmsg_dump_unregister(&dumper);
}

static struct workqueue_struct *wq;
static int __init bootlog_init(void)
{
	struct work_struct *w;

	if (!IS_ENABLED(CONFIG_BOOTLOG_DMESG))
		return 0;

	wq = alloc_ordered_workqueue("bootlog", WQ_MEM_RECLAIM);
	if (!wq)
		return -ENOMEM;
	w = kzalloc(sizeof(*w), GFP_KERNEL);
	if (!w)
		return -ENOMEM;
	INIT_WORK(w, bootlog_work);
	queue_work(wq, w);
	return 0;
}
late_initcall(bootlog_init);

static void __exit bootlog_exit(void)
{
	if (wq)
		destroy_workqueue(wq);
}
module_exit(bootlog_exit);

MODULE_LICENSE("GPL");
