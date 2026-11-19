**CUTLINE 0.1.0: developer preview.** Video editing without the paywall: free, open source (MIT), no account, no
telemetry.

> ⚠️ **This preview is the interface and project tools.** It can't import, play, cut or export video yet. The editing
> engine arrives in the next previews (see the roadmap below).

### What you can do
- Start from the **welcome screen**: YouTube video (16:9) or Short / TikTok / Reel (9:16)
- Create, open and save **`.cutline` projects**, with recent projects and unsaved-changes prompts
- Set **sequence settings**: YouTube HD/4K and vertical presets, 23.976–60 fps, 44.1/48/96 kHz, timecode start
- Manage **tracks**: add, remove, rename (double-click), lock, hide/mute, solo
- **Undo and redo** everything, with a clickable History panel
- Arrange **resizable panels** and save **workspaces**
- Use **Dark and Light themes**, interface scale 90–150%, and **custom themes**: import, live-edit, export
  ([how to make a theme](https://github.com/AlMonDdevV/cutline/blob/main/docs/themes.md))
- Change **keyboard shortcuts**: search, record, resolve conflicts, reset, import/export
  ([shortcut list](https://github.com/AlMonDdevV/cutline/blob/main/docs/keyboard.md))

### Download
| System | File | How |
|---|---|---|
| Windows 10/11 (64-bit) | `CUTLINE-0.1.0-preview-windows-x64.zip` | Unzip anywhere and run `cutline.exe`. Not code-signed yet: if Windows warns about an unknown publisher, choose **More info → Run anyway**. |
| macOS, Linux | not packaged yet | Builds and tests pass on every change (macOS Apple Silicon, Debian 13, Ubuntu 26.04, Fedora, Arch). [Build from source](https://github.com/AlMonDdevV/cutline/blob/main/docs/building.md) until packages arrive. |

Check your download with `CUTLINE-0.1.0-preview-windows-x64.zip.sha256`.

**System requirements (Windows):** Windows 10 or 11, 64-bit; a GPU with Direct3D 11 (or Windows' built-in software
renderer); 4 GB RAM is enough for this preview; about 200 MB of disk.

### Roadmap (next previews, no dates)
1. Import and playback
2. Timeline editing and export (cuts, trims, ripple/roll/slip/slide, J/L cuts, snapping, markers, MP4 with YouTube
   and Shorts presets)
3. Effects: keyframes with easing, transitions, titles and lower thirds, colour and LUTs, chroma key, speed
4. Audio and captions: ducking, normalising, noise reduction, offline auto-captions
5. Claude integration

### Licences
CUTLINE is MIT. This download includes Qt 6.10.3 (LGPLv3) and FFmpeg 9.0.2 (LGPL 2.1+, built without GPL or nonfree
parts) as replaceable DLLs, plus the Inter and JetBrains Mono fonts (SIL OFL). See `THIRD-PARTY-NOTICES.txt` in
the zip.
