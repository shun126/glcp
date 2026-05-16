#!/usr/bin/env bash

mkdir -p external/gl external/KHR

GLCOREARB_URLS=(
  "https://registry.khronos.org/OpenGL/api/GL/glcorearb.h"
  "https://www.opengl.org/registry/api/GL/glcorearb.h"
  "https://raw.githubusercontent.com/KhronosGroup/OpenGL-Registry/main/api/GL/glcorearb.h"
)

KHRPLATFORM_URLS=(
  "https://registry.khronos.org/EGL/api/KHR/khrplatform.h"
  "https://raw.githubusercontent.com/KhronosGroup/EGL-Registry/main/api/KHR/khrplatform.h"
)

downloaded=0
for url in "${GLCOREARB_URLS[@]}"; do
  if curl -fL -A "Mozilla/5.0 (compatible; glcp-bot/1.0)" "$url" -o external/gl/glcorearb.h; then
    downloaded=1
    break
  fi
done

if [ "$downloaded" -ne 1 ]; then
  echo "Failed to download glcorearb.h from official registry endpoints." >&2
  exit 1
fi

downloaded=0
for url in "${KHRPLATFORM_URLS[@]}"; do
  if curl -fL -A "Mozilla/5.0 (compatible; glcp-bot/1.0)" "$url" -o external/KHR/khrplatform.h; then
    downloaded=1
    break
  fi
done

if [ "$downloaded" -ne 1 ]; then
  echo "Failed to download khrplatform.h from official registry endpoints." >&2
  exit 1
fi

python3 - <<'PY'
from pathlib import Path
for p in [Path('external/gl/glcorearb.h'), Path('external/KHR/khrplatform.h')]:
    if not p.exists():
        continue
    t = p.read_text(encoding='utf-8', errors='replace')
    t = t.replace('\r\n','\n').replace('\r','\n')
    p.write_text(t.replace('\n','\r\n'), encoding='utf-8', newline='')
PY
