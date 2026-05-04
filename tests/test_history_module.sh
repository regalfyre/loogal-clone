#!/usr/bin/env bash
set -euo pipefail

echo "[test] building loogal with history module"
make >/tmp/loogal-history-build.log

echo "[test] checking history help"
./loogal history help | grep -q "LOOGAL HISTORY"

echo "[test] clearing history"
./loogal history clear --json > /tmp/loogal-history-clear.json
grep -q '"type": "history.cleared"' /tmp/loogal-history-clear.json

echo "[test] pushing history entries"
./loogal history push --query /tmp/a.png --session session_a --json > /tmp/loogal-history-push-a.json
./loogal history push --query /tmp/b.png --session session_b --offset 10 --selected 2 --json > /tmp/loogal-history-push-b.json
grep -q '"session": "session_b"' /tmp/loogal-history-push-b.json

echo "[test] checking back and forward"
./loogal history back --json > /tmp/loogal-history-back.json
grep -q '"session": "session_a"' /tmp/loogal-history-back.json
./loogal history forward --json > /tmp/loogal-history-forward.json
grep -q '"session": "session_b"' /tmp/loogal-history-forward.json

echo "[test] checking list json"
./loogal history list --json > /tmp/loogal-history-list.json
grep -q '"type": "history.list"' /tmp/loogal-history-list.json

echo "[test:ok] loogal-history module works"
