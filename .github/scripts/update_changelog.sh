#!/usr/bin/env bash
set -euo pipefail

date_utc="$(date -u +%Y-%m-%d)"
header="## ${date_utc}"
body="- Updated \`gl/glcorearb.h\` from OpenGL Registry and regenerated \`glcp/glcp.c\` and \`glcp/glcp.h\`."

if [ ! -f CHANGELOG.md ]; then
  cat > CHANGELOG.md <<EOF
# CHANGELOG

${header}
${body}
EOF
else
  if ! rg -F "${header}" CHANGELOG.md >/dev/null; then
    tmp="$(mktemp)"
    {
      echo "# CHANGELOG"
      echo
      echo "${header}"
      echo "${body}"
      echo
      sed '1{/^# CHANGELOG$/d;}' CHANGELOG.md
    } > "$tmp"
    mv "$tmp" CHANGELOG.md
  fi
fi

python3 - <<'PY'
from pathlib import Path
p = Path('CHANGELOG.md')
t = p.read_text(encoding='utf-8', errors='replace')
t = t.replace('\r\n','\n').replace('\r','\n')
p.write_text(t.replace('\n','\r\n'), encoding='utf-8', newline='')
PY
