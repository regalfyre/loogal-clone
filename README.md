# LOOGAL V1 — Local Visual Memory Engine

Drop an image. Search your indexed machine. Find visual matches by appearance.

Loogal V1 is the first working module of the local visual memory stack.

It follows the doctrine:

- CLI first
- C first
- local first
- binary hot path
- JSONL truth layer
- logs always
- no silent failure
- built alone, designed to join

## Commands

```bash
make
./loogal index ~/Pictures ~/Downloads
./loogal search meme.png
./loogal search meme.png 90
./loogal stats
./loogal dedupe --keep 1 --dry-run
./loogal dedupe --keep 1 --move-removed ~/.local/share/loogal/removed
```

## V1 Supported Files

- `.png`
- `.jpg`
- `.jpeg`
- `.webp`

V1 uses ImageMagick through `magick identify` and `magick convert` to normalize images into tiny grayscale raw buffers, then computes deterministic dHash v1 in C.

Install dependency on Arch/Garuda:

```bash
sudo pacman -S imagemagick
```

## Storage

Runtime data lives inside the repo for V1:

```text
data/loogal.bin              binary index used for search
data/records.jsonl           human/machine truth layer
data/logs/loogal.jsonl       event logs
data/manifests/              dedupe receipts
```

## Hot Path Rule

Search reads `data/loogal.bin` only.

JSONL is not parsed during search.

## Philosophy

Loogal V1 does not help you remember where images are.

It removes the need to remember them.
