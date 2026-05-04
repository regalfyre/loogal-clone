#!/usr/bin/env bash
set -euo pipefail

echo "[test] building loogal with action module"
make >/tmp/loogal-action-build.log

TMP="data/.loogal_action_test_file.txt"
mkdir -p data
echo "hello loogal action" > "$TMP"

echo "[test] checking action help"
./loogal action help | grep -q "LOOGAL ACTION"

echo "[test] checking reveal endpoint dry-run json"
./loogal action reveal --path "$TMP" --dry-run --json > /tmp/loogal-action-reveal.json
grep -q '"type": "action.result"' /tmp/loogal-action-reveal.json
grep -q '"verb": "reveal"' /tmp/loogal-action-reveal.json
grep -q '"status": "ok"' /tmp/loogal-action-reveal.json

echo "[test] checking open endpoint dry-run json"
./loogal action open --path "$TMP" --dry-run --json > /tmp/loogal-action-open.json
grep -q '"verb": "open"' /tmp/loogal-action-open.json

echo "[test] checking copy-path endpoint dry-run json"
./loogal action copy-path --path "$TMP" --dry-run --json > /tmp/loogal-action-copy.json
grep -q '"verb": "copy-path"' /tmp/loogal-action-copy.json

echo "[test:ok] loogal-action module works"
