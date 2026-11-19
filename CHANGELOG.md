# Changelog

## 0.1.0-preview: developer preview (2026-11-19)

The first public build of CUTLINE: the interface and project tools. **It can't import, play, cut or export video
yet.** The editing engine arrives in the next previews.

### What's in it
- **Welcome screen** with quick starts for a YouTube video (16:9, 1920 × 1080) and a Short, TikTok or Reel
  (9:16, 1080 × 1920), plus recent projects.
- **Projects:**
  - Create, open and save `.cutline` files.
  - Saves are atomic, so a crash never leaves a half-written file.
  - Recent projects and unsaved-changes prompts.
  - Clear error messages for damaged or newer files.
- **Sequence settings:** YouTube HD/4K and vertical presets, custom sizes, 23.976–60 fps, 44.1/48/96 kHz, and
  timecode starting at 00:00:00:00 or 01:00:00:00.
- **Tracks:** add, remove, rename (double-click), lock, hide or mute, and solo.
- **Undo and redo** for every change, with a History panel you can click to jump back.
- **Interface:**
  - Viewer, Timeline, Inspector and History panels in resizable areas.
  - Saved, named workspaces.
  - Dark and Light themes and interface scale from 90% to 150%.
- **Custom themes:**
  - Import, export and live-edit theme files.
  - Themes can set colours, fonts, corner radius and spacing.
  - See `docs/themes.md` and the JSON Schema in `schema/`.
- **Keyboard:**
  - A complete shortcut system that never fires while you type.
  - A Keyboard Shortcuts page to search, record, resolve conflicts, reset, and import/export profiles.
  - Works with non-US keyboard layouts.

### Downloads
- **Windows 10/11 (64-bit):** portable `.zip`, nothing to install. Not code-signed yet, so Windows may show
  "unknown publisher"; choose *More info → Run anyway*.
- **macOS and Linux:** these build and pass their tests on every change (macOS Apple Silicon, Debian 13,
  Ubuntu 26.04, Fedora, Arch). Packaged downloads come in a later preview; you can build from source now
  (`docs/building.md`).

### Privacy
CUTLINE makes no network connections. No telemetry, no account.
