#!/usr/bin/env bash
set -euo pipefail

echo "[test] building loogal with window-api module"
make >/tmp/loogal-window-api-build.log

echo "[test] checking window-api help"
./loogal window-api help | grep -q "LOOGAL WINDOW-API"

echo "[test] checking page dry-run json"
./loogal window-api page --session fake_session --offset 0 --limit 10 --dry-run --json > /tmp/loogal-window-api-page.json
grep -q '"type": "window_api.dry_run"' /tmp/loogal-window-api-page.json
grep -q 'session page' /tmp/loogal-window-api-page.json

echo "[test] checking current dry-run json"
./loogal window-api current --dry-run --json > /tmp/loogal-window-api-current.json
grep -q 'history current' /tmp/loogal-window-api-current.json

echo "[test] checking similar dry-run json"
./loogal window-api similar --path /tmp/query.png --place /tmp --push-history --dry-run --json > /tmp/loogal-window-api-similar.json
grep -q 'similar --path' /tmp/loogal-window-api-similar.json
grep -q -- '--push-history' /tmp/loogal-window-api-similar.json

echo "[test:ok] loogal-window-api module works"
