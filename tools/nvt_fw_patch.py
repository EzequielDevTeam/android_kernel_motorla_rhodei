import sys
path = sys.argv[1]
s = open(path).read()

old = """	ret = request_firmware(&fw_entry, filename, &ts->client->dev);
	if (ret) {
		NVT_ERR("firmware load failed, ret=%d\\n", ret);
		return ret;
	}"""

new = """	/* Motorola deixa o FW do touch em /vendor/firmware (fora do path
	 * padrao do firmware_class). Tenta o path do vendor antes de falhar. */
	ret = request_firmware(&fw_entry, filename, &ts->client->dev);
	if (ret) {
		char alt[128];
		const char *vdir;

		vdir = of_get_property(ts->client->dev.of_node, "firmware-path",
					NULL);
		if (!vdir)
			vdir = "/vendor/firmware";

		snprintf(alt, sizeof(alt), "%s/%s", vdir, filename);
		NVT_LOG("retry firmware from %s\\n", alt);
		ret = request_firmware(&fw_entry, alt, &ts->client->dev);
	}
	if (ret) {
		NVT_ERR("firmware load failed, ret=%d\\n", ret);
		return ret;
	}"""

if old not in s:
    print("PATCH: ancora nao encontrada")
    sys.exit(1)

s = s.replace(old, new, 1)
if "#include <linux/of.h>" not in s:
    s = s.replace("#include <linux/firmware.h>", "#include <linux/firmware.h>\n#include <linux/of.h>", 1)
open(path, "w").write(s)
print("PATCH: rota de firmware do touch aplicada")
