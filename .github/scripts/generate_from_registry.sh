#!/usr/bin/env bash
set -euo pipefail

mkdir -p gl

SOURCE_URL="https://registry.khronos.org/OpenGL/api/GL/glcorearb.h"
curl -fL -A "Mozilla/5.0 (compatible; glcp-bot/1.0)" "$SOURCE_URL" -o gl/glcorearb.h

ruby glcp.rb

python3 - <<'PY2'
from pathlib import Path
for p in [Path('gl/glcorearb.h'), Path('glcp/glcp.c'), Path('glcp/glcp.h')]:
    if not p.exists():
        continue
    t = p.read_text(encoding='utf-8', errors='replace')
    t = t.replace('\r\n', '\n').replace('\r', '\n')
    p.write_text(t.replace('\n', '\r\n'), encoding='utf-8', newline='')
PY2
