#!/usr/bin/env bash
set -euo pipefail

echo "[test] building loogal with search-json module"
make >/tmp/loogal-search-json-build.log

echo "[test] checking CLI advertises search command"
./loogal help >/tmp/loogal-search-json-help.log || true
if ! grep -q "loogal search" /tmp/loogal-search-json-help.log; then
  echo "[test:error] help output does not mention loogal search" >&2
  cat /tmp/loogal-search-json-help.log >&2
  exit 1
fi

echo "[test:ok] search-json module builds"
