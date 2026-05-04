#!/usr/bin/env bash
set -euo pipefail

echo "[test] building loogal with thumbnail module"
make >/dev/null

echo "[test] checking thumbnail help"
./loogal thumbnail help | grep -q "LOOGAL THUMBNAIL"

echo "[test] checking thumbnail path json"
./loogal thumbnail path --path /tmp/query.png --size 160 --json > /tmp/loogal-thumbnail-path.json
grep -q '"type": "thumbnail.path"' /tmp/loogal-thumbnail-path.json
grep -q 'data/thumbnails' /tmp/loogal-thumbnail-path.json

echo "[test] checking thumbnail status json"
./loogal thumbnail status --json > /tmp/loogal-thumbnail-status.json
grep -q '"type": "thumbnail.status"' /tmp/loogal-thumbnail-status.json

echo "[test:ok] loogal-thumbnail module works"
