#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
rm -rf /tmp/loogal-smoke data/loogal.bin data/records.jsonl data/logs/loogal.jsonl data/manifests/*.json
mkdir -p /tmp/loogal-smoke/a /tmp/loogal-smoke/b
magick -size 64x64 xc:white -fill black -draw 'rectangle 10,10 40,40' /tmp/loogal-smoke/a/box.png
cp /tmp/loogal-smoke/a/box.png /tmp/loogal-smoke/b/box-copy.jpg
magick -size 64x64 xc:white -fill black -draw 'circle 32,32 32,10' /tmp/loogal-smoke/a/circle.png
./loogal index /tmp/loogal-smoke
./loogal search /tmp/loogal-smoke/a/box.png 90
./loogal stats
./loogal dedupe --keep 1 --dry-run
printf '\nSMOKE TEST OK\n'
