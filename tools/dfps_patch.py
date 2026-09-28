import sys

path = sys.argv[1] if len(sys.argv) > 1 else "techpack/display/msm/dsi/dsi_display.c"
s = open(path).read()
orig = s

# 1) set_mode: falha de DFPS nao deve abortar o modo
old1 = """	rc = dsi_display_dfps_update(display, mode);
		if (rc) {
			DSI_ERR("[%s]DSI dfps update failed, rc=%d\\n",
					display->name, rc);
			goto error;
		}"""
new1 = """	rc = dsi_display_dfps_update(display, mode);
		if (rc) {
			DSI_WARN("[%s]DFPS update falhou (rc=%d); seguindo com refresh fixo\\n",
					display->name, rc);
			rc = 0;
		}"""

# 2) post_kickoff / seamless: idem
old2 = """	rc = dsi_ctrl_async_timing_update(m_ctrl->ctrl, timing);
	if (rc) {
		DSI_ERR("[%s] failed to dfps update host_%d, rc=%d\\n",
				display->name, i, rc);
		goto error;
	}"""
new2 = """	rc = dsi_ctrl_async_timing_update(m_ctrl->ctrl, timing);
	if (rc) {
		DSI_WARN("[%s] dfps host_%d rc=%d; ignorando troca de refresh\\n",
				display->name, i, rc);
		rc = 0;
	}"""

old3 = """		rc = dsi_ctrl_async_timing_update(ctrl->ctrl, timing);
		if (rc) {
			DSI_ERR("[%s] failed to dfps update host_%d, rc=%d\\n",
					display->name, i, rc);
			goto error;
		}"""
new3 = """		rc = dsi_ctrl_async_timing_update(ctrl->ctrl, timing);
		if (rc) {
			DSI_WARN("[%s] dfps host_%d rc=%d; ignorando troca de refresh\\n",
					display->name, i, rc);
			rc = 0;
		}"""

n = 0
for o, w in ((old1, new1), (old2, new2), (old3, new3)):
    if o in s:
        s = s.replace(o, w)
        n += 1

if n == 0:
    print("PATCH: nenhum ponto encontrado (arvore mudou?)")
    sys.exit(1)

open(path, "w").write(s)
print("PATCH: %d ponto(s) ajustados" % n)
