```
╔══════════════════════════════════════════════════════════════════════╗
║                          LOOGAL V1 COMMANDS                         ║
║                 Local Visual Memory Engine (CLI First)              ║
╚══════════════════════════════════════════════════════════════════════╝
```
<img width="1254" height="1254" alt="390ab73f-eb41-4f9e-b043-4e1797db919f" src="https://github.com/user-attachments/assets/9c0e1a0d-4e68-4b7e-a5c6-0917713457c7" />

CORE INDEXING + SEARCH
──────────────────────────────────────────────────────────────────────
```text
loogal index <directories...> ```


→ Scans directories and builds/updates binary visual index
```
loogal learn <directories...>
    → Incremental learning (adds new images without full rebuild)

loogal search <image> [MIN_PERCENT]
    → Searches for visually similar images
    → Optional threshold (e.g. 90 = stricter matches)

loogal stats
    → Shows index stats (image count, size, etc.)
```

DEDUPLICATION
──────────────────────────────────────────────────────────────────────
```text
loogal dedupe --keep N [--dry-run] [--move-removed DIR] [--protect DIR...]
```


    → Finds near-duplicate images
    → --keep N           keep N copies
    → --dry-run          preview only
    → --move-removed     move extras instead of deleting
    → --protect          never touch certain directories

THUMBNAILS + ACTIONS
──────────────────────────────────────────────────────────────────────
```text
loogal thumbnail <path|create|session|status>
    → Manages thumbnails for fast UI rendering

loogal action reveal --path <file>
    → Opens file location in file manager

loogal action open --path <file>
    → Opens file with system default app

loogal action copy-path --path <file>
    → Copies file path to clipboard
```


SESSION SYSTEM (VISUAL SEARCH MEMORY)
──────────────────────────────────────────────────────────────────────
```text
loogal session create --query <image> --place <dir> [--json]
    → Creates a search session tied to an image

loogal history push --query <image> --session <id>
    → Logs query into history system

loogal similar --path <image> --place <dir> [--json]
    → Alternate search entry (similar to session create)

```

WINDOW API (GUI BACKEND)
──────────────────────────────────────────────────────────────────────
```text
loogal window-api page --session <id> [--json]
    → Returns paginated results for GUI window
```


WATCH SYSTEM (SCHEDULED INDEXING)
──────────────────────────────────────────────────────────────────────
```text
loogal watch-add <path> --daily HH:MM
    → Index path every day at given time

loogal watch-add <path> --hourly
    → Index path every time watch-run executes

loogal watch-add <path> --weekly <day> HH:MM
    → Index path on specific weekday + time

loogal watch-add <path> --yearly MM-DD HH:MM
    → Index path once per year

loogal watch-list
    → Lists all watched paths + schedules

loogal watch-remove <path>
    → Removes path from watcher

loogal watch-enable <path>
    → Enables a watcher entry

loogal watch-disable <path>
    → Disables a watcher entry

loogal watch-run
    → Executes scheduled indexing (checks what is due now)

loogal watch-run --dry-run
    → Shows what WOULD run without executing

loogal watch-run --all
    → Forces indexing of ALL watched paths

```

SYSTEM + DEBUGGING
──────────────────────────────────────────────────────────────────────
```text
loogal doctor
    → Runs diagnostics on environment + dependencies

loogal config
    → Displays current configuration

loogal why <path>
    → Explains why an image matched (debug insight)

loogal verify
    → Verifies index integrity

loogal bench
    → Runs performance benchmarks

loogal rebuild <directories...>
    → Full rebuild of index from scratch

loogal where
    → Shows where Loogal data is stored

loogal status
    → Overall system status


```
GUI (WINDOW SYSTEM)
──────────────────────────────────────────────────────────────────────
```text
make loogal-window
./loogal-window
    → Opens visual interface
```
Features:
    - grid layout
    - double-click viewer
    - arrow navigation
    - zoom in/out
    - session-based browsing



CURRENT CAPABILITIES
──────────────────────────────────────────────────────────────────────

✔ Local-first visual search

✔ Binary index (fast hot path)

✔ JSONL truth layer

✔ Scheduled indexing system

✔ GUI viewer + sessions

✔ Deterministic hashing (dHash v1 in C)



NEXT EVOLUTION
──────────────────────────────────────────────────────────────────────

DROP ANYWHERE SEARCH
```
drag image → drop into window
    → new session created
    → new window opens
    → current session preserved
```
No buttons. No friction. Pure intent.



TAGLINE
──────────────────────────────────────────────────────────────────────

'Bout TIME!
