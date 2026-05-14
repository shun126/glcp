#!/usr/bin/env bash

mkdir -p dist
version="$(date -u +%Y%m%d)"
archive="dist/glcp-${version}.zip"

python3 - <<'PY'
from pathlib import Path
for p in [Path('glcp/README.md'), Path('LICENSE.txt'), Path('glcp/glcp.c'), Path('glcp/glcp.h')]:
    text = p.read_text(encoding='utf-8', errors='replace')
    text = text.replace('\r\n', '\n').replace('\r', '\n')
    p.write_text(text.replace('\n', '\r\n'), encoding='utf-8', newline='')
PY

zip -j "$archive" glcp/README.md LICENSE.txt glcp/glcp.c glcp/glcp.h
