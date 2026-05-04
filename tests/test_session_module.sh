#!/usr/bin/env bash
set -euo pipefail

echo "[test] building loogal with session module"
make >/tmp/loogal-session-build.log

echo "[test] checking session help"
./loogal session help | grep -q "LOOGAL SESSION"

echo "[test] checking session dry-run json"
./loogal session create --query data/fake.png --place data --limit 5 --dry-run --json > /tmp/loogal-session-dry.json
grep -q '"type": "session.dry_run"' /tmp/loogal-session-dry.json
grep -q '"command"' /tmp/loogal-session-dry.json

echo "[test] checking session list json"
./loogal session list --json > /tmp/loogal-session-list.json
grep -q '"type": "session.list"' /tmp/loogal-session-list.json

echo "[test:ok] loogal-session module works"
