#!/usr/bin/env bash
set -euo pipefail

echo "[test] building loogal with similar module"
make >/tmp/loogal-similar-build.log

echo "[test] checking similar help"
./loogal similar help | grep -q "LOOGAL SIMILAR"

echo "[test] checking similar dry-run json with place"
./loogal similar --path /tmp/query.png --place /tmp --limit 50 --dry-run --json > /tmp/loogal-similar-dry.json
grep -q '"type": "similar.dry_run"' /tmp/loogal-similar-dry.json
grep -q 'session create' /tmp/loogal-similar-dry.json

echo "[test] checking similar dry-run json with memory"
./loogal similar --path /tmp/query.png --memory --dry-run --json > /tmp/loogal-similar-memory.json
grep -q '"memory"' /tmp/loogal-similar-memory.json
grep -q -- '--memory' /tmp/loogal-similar-memory.json

echo "[test:ok] loogal-similar module works"
