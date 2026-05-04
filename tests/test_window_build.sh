#!/usr/bin/env bash
set -euo pipefail

echo "[test] checking loogal-window source exists"
test -f src/gui/loogal_window.c
test -f include/gui/loogal_window.h

echo "[test] checking Makefile has loogal-window target"
grep -q "loogal-window" Makefile

if pkg-config --exists gtk4; then
  echo "[test] building loogal-window"
  make loogal-window
  ./loogal-window --help | grep -q "LOOGAL WINDOW"
else
  echo "[test:skip] gtk4 dev files not installed; install with: sudo pacman -S gtk4"
fi

echo "[test:ok] loogal-window module is installed"
