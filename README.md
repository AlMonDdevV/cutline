# CUTLINE

**Video editing without the paywall.**

CUTLINE is a free, open-source desktop video editor for Windows, macOS and Linux. No subscription, no watermark,
no account and no telemetry. It's licensed under the [MIT License](LICENSE).

> **Status: 0.1.0 developer preview.** This preview has the interface and project tools: projects, sequence and
> track settings, undo/redo, themes, workspaces and keyboard shortcuts. **It can't edit video yet.** Import and
> playback, timeline editing and export, then effects and keyframes arrive in the next previews.
> See [CHANGELOG.md](CHANGELOG.md).

## Download

Windows 10/11: get the portable `.zip` from [Releases](https://github.com/AlMonDdevV/cutline/releases), unzip it,
and run `cutline.exe`. macOS and Linux users can [build from source](docs/building.md) for now.

## Docs

- [Building from source](docs/building.md)
- [Keyboard shortcuts](docs/keyboard.md)
- [Themes: format, schema and how to make one](docs/themes.md)
- [Project file format](docs/project-format.md)
- [Licensing and codecs](docs/licensing.md) · [Dependencies](docs/dependencies.md) · [Network use](docs/network.md)
- [Design](docs/superpowers/specs/2026-11-18-cutline-design.md)

## Roadmap

1. **Import and playback:** media library, viewer with sound, JKL, frame stepping
2. **Timeline editing and export:** cuts, trims, ripple/roll/slip/slide, J/L cuts, snapping, markers, MP4 export
   with YouTube and Shorts presets
3. **Effects:** keyframes with easing, transitions, titles and lower thirds, colour correction and LUTs, chroma key,
   speed changes
4. **Audio and captions:** ducking, normalising, noise reduction, offline auto-captions
5. **Claude integration:** edit plans you approve, over a local MCP connection
