Click.Drop.Done
==========================================
## LOOGAL 


WHAT IS LOOGAL?

Loogal is like:

  Google Images search
  + Finder / File Explorer
  + your own memory

But entirely local.

You give it an image.
It tells you where you’ve seen it before.

No cloud.
No upload.
No guessing filenames.

--------------------------------------------------------------------------------

CORE IDEA

Instead of:

  "what was that file called?"
  "what folder was it in?"

You do:

  "this is what it looked like"

Loogal finds:

  ✔ exact matches
  ✔ near-duplicates
  ✔ visually similar screenshots
  ✔ images you forgot existed

--------------------------------------------------------------------------------

BUILD

make
make loogal-window

--------------------------------------------------------------------------------

QUICK START

1. Index your images:

   ./loogal index ~/Pictures ~/Downloads

2. Open interactive window:

   ./loogal-window

3. Drag & drop an image into the window

Loogal will:

  → create a search session
  → open a NEW window (no interruption)
  → show matching images instantly

--------------------------------------------------------------------------------

CORE COMMANDS (THE ENGINE)

./loogal index <directories...>

  Scans directories and builds a visual index.
  Think:
    "Google crawling your disk"

--------------------------------------------------------------------------------

./loogal search <image> [MIN_PERCENT]

  Search for visually similar images.

  Example:
    ./loogal search meme.png
    ./loogal search meme.png 90

  MIN_PERCENT:
    100 → exact matches only
    90  → near identical
    60  → similar scenes

  Think:
    "Google reverse image search — but local and instant"

--------------------------------------------------------------------------------

./loogal stats

  Shows what's indexed.

  Think:
    "how much memory Loogal has"

--------------------------------------------------------------------------------

./loogal dedupe --keep N [--dry-run] [--move-removed DIR] [--protect DIR...]

  Finds duplicate/near-duplicate images.

  --keep 1
    keep best version

  --dry-run
    preview only

  --move-removed
    move duplicates instead of deleting

  Think:
    "Google Photos duplicate cleanup — but controllable"

--------------------------------------------------------------------------------

./loogal rebuild <directories...>

  Rebuild index from scratch.

  Think:
    "reindex everything cleanly"

--------------------------------------------------------------------------------

./loogal verify

  Validate index integrity.

--------------------------------------------------------------------------------

./loogal bench

  Performance test.

--------------------------------------------------------------------------------

./loogal doctor

  Diagnose issues.

--------------------------------------------------------------------------------

./loogal config

  Show configuration.

--------------------------------------------------------------------------------

./loogal where

  Show where Loogal stores data.

--------------------------------------------------------------------------------

./loogal status

  System status overview.

--------------------------------------------------------------------------------

SESSION SYSTEM (STATEFUL SEARCH)

./loogal session create --query <image> --place <dir> [--json]

  Creates a search session.

  Returns:
    session_id

  Think:
    "opening a search tab"

--------------------------------------------------------------------------------

./loogal history push --query <image> --session <id>

  Stores search history into a session.

--------------------------------------------------------------------------------

SIMILARITY MODE

./loogal similar --path <image> --place <dir> [--json]

  Finds visually similar images directly.

--------------------------------------------------------------------------------

WINDOW SYSTEM (THIS BRANCH)

./loogal-window
./loogal-window --session <id> --place <dir>

  Interactive UI for Loogal.

--------------------------------------------------------------------------------

DROP ANYWHERE (MAIN FEATURE)

Run:

  ./loogal-window

Then:

  drag any image into the window

Behavior:

  1. image is accepted
  2. session is created
  3. NEW window opens automatically
  4. results appear

No interruption.
No losing your place.

Think:

  "Chrome opening a new tab — but for visual memory"

--------------------------------------------------------------------------------

SUPPORTED IMAGE TYPES

  .png
  .jpg
  .jpeg
  .webp

--------------------------------------------------------------------------------

STORAGE MODEL

data/loogal.bin        → binary index (FAST search)
data/records.jsonl     → truth layer
data/logs/             → logs
data/manifests/        → dedupe data
data/sessions/         → session memory

--------------------------------------------------------------------------------

HOT PATH RULE

Search uses ONLY:

  data/loogal.bin

No JSON parsing during search.

This is why Loogal is fast.

--------------------------------------------------------------------------------

PERFORMANCE

~80–90 images/sec (CPU dependent)

Deterministic.
No network.
No latency.

--------------------------------------------------------------------------------

WHAT YOU JUST BUILT

Loogal is:

  ✔ a visual search engine
  ✔ a memory system
  ✔ a local-first alternative to cloud tools

--------------------------------------------------------------------------------

COMING NEXT — PDF MAVERICK

Loogal will soon:

  see inside PDFs

Planned:

  ./loogal index ~/Documents --include pdf
  ./loogal search query.png ~/Documents --include pdf
  ./loogal search query.png ~/Documents --include pdf --discover

Meaning:

  → extract images from PDFs
  → index them
  → search them like normal images

Even if they’re buried inside documents.

--------------------------------------------------------------------------------

FUTURE DIRECTIONS

  ✔ PDF visual extraction
  ✔ rename-safe identity tracking
  ✔ background indexing (systemd)
  ✔ UI refinement (aesthetic mode)
  ✔ panel extraction (comics / layouts)

--------------------------------------------------------------------------------

FINAL LINE

Loogal does not search files.

It remembers what you’ve seen.



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
