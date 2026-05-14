#!/usr/bin/env bash
set -euo pipefail

mkdir -p gl

URLS=(
  "https://www.opengl.org/registry/api/GL/glcorearb.h"
  "https://registry.khronos.org/OpenGL/api/GL/glcorearb.h"
  "https://raw.githubusercontent.com/KhronosGroup/OpenGL-Registry/main/api/GL/glcorearb.h"
)

downloaded=0
for url in "${URLS[@]}"; do
  if curl -fL -A "Mozilla/5.0 (compatible; glcp-bot/1.0)" "$url" -o gl/glcorearb.h; then
    downloaded=1
    break
  fi
done

if [ "$downloaded" -ne 1 ]; then
  echo "Failed to download glcorearb.h from official registry endpoints." >&2
  exit 1
fi

ruby glcp.rb

python3 - <<'PY'
from pathlib import Path
for p in [Path('gl/glcorearb.h'), Path('glcp/glcp.c'), Path('glcp/glcp.h')]:
    if not p.exists():
        continue
    t = p.read_text(encoding='utf-8', errors='replace')
    t = t.replace('\r\n','\n').replace('\r','\n')
    p.write_text(t.replace('\n','\r\n'), encoding='utf-8', newline='')
PY
