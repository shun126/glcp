#!/usr/bin/env bash

set -euo pipefail

if [ -z "${GLCP_VERSION:-}" ]; then
  echo "GLCP_VERSION is required" >&2
  exit 1
fi

mkdir -p dist
date_stamp="$(date -u +%Y%m%d)"
archive="dist/glcp-${GLCP_VERSION}-${date_stamp}.zip"

python3 - <<'PY'
from pathlib import Path
for p in [Path('glcp/README.md'), Path('LICENSE.txt'), Path('glcp/glcp.c'), Path('glcp/glcp.h'), Path('glcp/glcp_compat.c'), Path('glcp/glcp_compat.h')]:
    text = p.read_text(encoding='utf-8', errors='replace')
    text = text.replace('\r\n', '\n').replace('\r', '\n')
    p.write_text(text.replace('\n', '\r\n'), encoding='utf-8', newline='')
PY

zip -j "$archive" glcp/README.md LICENSE.txt glcp/glcp.c glcp/glcp.h glcp/glcp_compat.c glcp/glcp_compat.h
