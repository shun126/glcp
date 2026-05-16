#!/usr/bin/env bash
set -euo pipefail

glcp_version="${GLCP_VERSION:-}"
if [ -z "$glcp_version" ]; then
  echo "GLCP_VERSION is required." >&2
  exit 1
fi

header="## version ${glcp_version}"
body="- Updated \`gl/glcorearb.h\` from OpenGL Registry and regenerated \`glcp/glcp.c\`, \`glcp/glcp.h\`, \`glcp/glcp_compat.c\`, and \`glcp/glcp_compat.h\`."

python3 - "$header" "$body" <<'PY'
from pathlib import Path
import sys

header = sys.argv[1]
body = sys.argv[2]
path = Path('CHANGELOG.md')

existing = ''
if path.exists():
    existing = path.read_text(encoding='utf-8', errors='replace')

lines = existing.replace('\r\n', '\n').replace('\r', '\n').split('\n')
normalized = []
seen_title = False
for line in lines:
    if line == '# CHANGELOG':
        if seen_title:
            continue
        seen_title = True
    normalized.append(line)

while normalized and normalized[-1] == '':
    normalized.pop()

if not seen_title:
    normalized.insert(0, '# CHANGELOG')

content = '\n'.join(normalized).strip()
if header not in content.split('\n'):
    if content == '# CHANGELOG':
        updated = f"# CHANGELOG\n\n{header}\n{body}\n"
    else:
        remainder = content
        if remainder.startswith('# CHANGELOG\n'):
            remainder = remainder[len('# CHANGELOG\n'):].lstrip('\n')
        elif remainder == '# CHANGELOG':
            remainder = ''
        updated = f"# CHANGELOG\n\n{header}\n{body}\n"
        if remainder:
            updated += f"\n{remainder}\n"
else:
    updated = content + '\n'

path.write_text(updated, encoding='utf-8', newline='\n')
PY

python3 - <<'PY'
from pathlib import Path
p = Path('CHANGELOG.md')
t = p.read_text(encoding='utf-8', errors='replace')
t = t.replace('\r\n','\n').replace('\r','\n')
p.write_text(t.replace('\n','\r\n'), encoding='utf-8', newline='')
PY
