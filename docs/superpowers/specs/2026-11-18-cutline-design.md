# CUTLINE — Design

Status: draft for approval · 2026-11-18
Sources: the CUTLINE product specification and the Keyboard Shortcut & Interaction specification (both supplied in
chat on 2026-11-18), the three reference screenshots in `references/`, and `docs/reference-analysis.md`.

Where this document says **(call)** it marks a decision Claude made on the user's behalf. The user asked Claude to pick
the recommended option wherever the specs conflict or leave a choice open. All calls are listed together in §20.

---

## 1. Intent

CUTLINE is a free, open-source (MIT) desktop video editor for Windows, macOS and Linux (Fedora, Arch, Ubuntu,
Debian). Tagline: *Video editing without the paywall.* It exists to give people a real alternative to subscription
editors: a native app with a real engine, no watermark, no export limits, no account and no telemetry.

Its distinguishing feature is a **permission-controlled Claude integration over MCP**. Claude can inspect a project,
propose an edit plan, and apply it only after the user approves. Every applied change can be undone.

Success means someone with an ordinary laptop can import phone or camera footage, cut a multi-track edit with titles,
music and colour fixes, and export a correct MP4. It should feel like a commercial tool while doing it, and nothing on
screen should be fake.

**Audience (user, 2026-11-19):** everyone equally: beginners, creators and experienced editors.
- Defaults are balanced: plain wording, a clean default layout, and pro features one click away.
- A Simple/Advanced interface level in Settings comes later (M10) for people who want fewer or more panels.

**What people make most (user):** YouTube videos (16:9, 1080p and 4K) and Shorts/TikTok/Reels (vertical 9:16,
1080×1920). These presets come first everywhere: new sequence, sequence settings and export.

**More product decisions (user, 2026-11-19, second batch):**
- **First launch: welcome screen plus a guided tour.**
  - The welcome screen arrives in M2. It has quick-start cards (*YouTube video, 16:9*; *Short / TikTok / Reel, 9:16*;
    *Open project…*), recent projects, and a "Show this at startup" checkbox.
  - The guided tour arrives in M10, once every panel it points at works. It is a skippable step-by-step overlay shown
    once on first launch and again from Help ▸ Take the Tour.
- **Auto captions in 1.0:** local, offline speech-to-text using whisper.cpp (MIT).
  - It writes captions onto a subtitle track, with caption styles made for Shorts.
  - The speech model is a one-time, opt-in download, and audio never leaves the computer.
  - Built in M9, next to the Claude integration. Claude can also read the transcript through a `get_transcript`
    tool, when the user allows it.
- **Claude edits:** ask every time. Every change is a plan the user approves (§15), and it can be loosened per project.
- **Computers are a real mix:** first launch runs a hardware check (§17) and picks defaults to suit the machine.

## 2. Goals, non-goals, anti-goals

**Goals (1.0):** everything in the product spec §5–§12 and the full keyboard spec, delivered across the milestones in
§19.

**Non-goals for 1.0** (can come later; the architecture must not block them):
- node-based compositing, motion tracking, stabilisation, multicam, nested sequences
- HDR grading (HDR sources are shown tone-mapped, with a warning)
- a third-party plugin SDK (OpenFX later)
- cloud speech-to-text (local auto captions *are* in 1.0, per the user, 2026-11-19)
- collaboration, cloud sync, mobile versions
- image sequences and animated GIF import
- translations (1.0 is English only, but every UI string is wrapped for translation from the start)
- sync-lock tracks, bezier keyframe handles, 5.1 export

**Anti-goals** (a failure even if everything else works):
- buttons, panels or shortcuts that do nothing, and fake progress bars or fake success messages
- any telemetry, hidden network traffic, or account requirement
- modifying, moving or overwriting source media
- Claude changing the timeline without the user seeing and approving it, or Claude running arbitrary commands
- the "generic AI app" look: random gradients, glass, floating cards, dashboard tiles
- GPL code in the official binaries (see §4)

## 3. Constraints

- **License:** CUTLINE's code is MIT. Official downloads may contain only MIT-compatible components: permissive
  licences (MIT, BSD, Apache-2.0, zlib, BSL, ISC, OFL for fonts) or LGPL libraries that are linked dynamically.
- **Platforms:**
  - Windows 10/11 x64 is the primary target and the only one that can be run and checked by hand on the dev machine
    (i5-12400F, RTX 3050, 16 GB).
  - macOS 13+ on Apple Silicon. Intel Macs are best-effort **(call)**.
  - Linux x86_64 on Ubuntu, Debian, Fedora and Arch.
- **Toolchain:** C++20, CMake ≥ 3.28, Ninja, MSVC 19.4x / Clang 17+ / GCC 13+.
- **Qt:** Qt 6, built against the latest stable release for the official downloads. Qt 6.8 is the minimum for
  distro-native packages, because the viewer needs `QQuickRhiItem`, which arrived in 6.7.
- **Hardware floor:** 4-core CPU, 8 GB RAM, a GPU with Direct3D 11, Metal, Vulkan 1.1 or OpenGL 3.3, and a 1366×768
  screen.
- **Verification reality:** macOS and Linux are built and tested headless in GitHub Actions. There is no Mac and no WSL
  on the dev machine, so UI on those platforms is checked from CI screenshots only.

## 4. Licensing and codecs

| Component | Licence | How it is used |
|---|---|---|
| Qt 6 (Core, Gui, Quick, QuickControls2, Svg, Network, Test) | LGPLv3 | dynamic linking; no GPL-only Qt add-ons |
| FFmpeg (avformat, avcodec, avutil, swresample, swscale, avfilter) | LGPL 2.1+ | built **without** `--enable-gpl` / `--enable-nonfree` |
| dav1d (AV1 decode), libopus, SVT-AV1 (AV1 encode) | BSD-2 / BSD-3 / BSD-3-Clear + AOM patent licence | via FFmpeg |
| OpenH264 (software H.264 encode) | BSD-2 | Cisco's binary, loaded at runtime (below) |
| miniaudio (audio output) | MIT-0 / public domain | vendored single header |
| QtKeychain (API key storage) | BSD-3 | dynamic or static |
| xkbcommon (Linux keyboard layouts) | MIT | system library |
| Inter (UI font), JetBrains Mono (timecode) | OFL-1.1 | bundled |
| Phosphor icons (generic glyphs only) | MIT | adapted to CUTLINE's stroke and grid |

**H.264 / H.265 without x264 or x265 (call):**
1. **Hardware encoders first:** NVENC, Intel QSV, AMD AMF, Apple VideoToolbox, VAAPI. Almost every machine from the
   last ten years has one.
2. **OpenH264 as the software H.264 fallback.** Cisco only covers the H.264 patent royalties for its own prebuilt
   binary when the user downloads it from Cisco. So CUTLINE has its own small encoder wrapper that loads
   `openh264` dynamically if it is present. Settings has a one-click, opt-in "Download OpenH264 from Cisco" button,
   which is one of the documented network connections. Fedora's Cisco repo package is detected too.
3. **H.265 in software** only when the FFmpeg that CUTLINE is running on includes libx265. That happens with native
   Linux packages that use the distro's FFmpeg; the official downloads never bundle it.
4. **AV1** via SVT-AV1 is always available, because it is royalty-free.

**Enforcement:**
- At startup, the app reads `avcodec_license()` and `avcodec_configuration()`.
- A unit test fails the official build if FFmpeg reports GPL or nonfree.
- The About box shows the licence of the FFmpeg that is actually loaded, which is GPL on some native Linux packages.

**Linux distros:**
- Fedora's own FFmpeg (`ffmpeg-free`) leaves out patented codecs. The Fedora `.rpm` therefore depends on hardware
  encoders plus OpenH264 from the Cisco repo. The Flatpak, which uses the full-codec FFmpeg extension, is the
  recommended install on Fedora.
- Ubuntu, Debian and Arch native packages use the distro's FFmpeg.

This section is not legal advice. `docs/licensing.md` records the reasoning, and the installers ship a
`THIRD-PARTY-NOTICES` file.

## 5. Approaches considered

| | Approach | Verdict |
|---|---|---|
| **A** | **Own engine:** FFmpeg for decode and encode, CUTLINE's own GPU compositor on Qt RHI (D3D11/12, Metal, Vulkan, OpenGL), own audio mixer | **Chosen by the user.** Everything stays MIT/LGPL. Preview and export share one render path. CUTLINE owns the data model that the project file and MCP use. Native MSVC/Clang builds. Cost: frame-accurate seeking, A/V sync and caching have to be built in-house, so first playback arrives later. |
| B | MLT Framework (Shotcut, Kdenlive) | Rejected. Its GPU (movit), Qt text, frei0r, rubberband and vid.stab modules are GPL, so under the MIT constraint much of its value is gone. Its XML would become the project format and the thing Claude reasons about, and on Windows it is mostly built via MSYS2 or KDE Craft. |
| — | FFmpeg filter graphs as the engine | Dropped. Fine for export, but static graphs cannot keep up with interactive scrubbing and keyframe editing, so preview and export would split into two code paths. |
| — | GStreamer Editing Services (Pitivi) | Dropped. Weaker on Windows and macOS, and a maze of plugin licences. |

## 6. System overview

```
┌────────────────────────── cutline (GUI process) ──────────────────────────┐
│  QML UI ── ViewModels ── ActionRegistry/ShortcutRouter                     │
│     │            │                │                                        │
│     ▼            ▼                ▼                                        │
│  core: Project, Sequences, Commands, UndoStack  ◀── ControlServer ◀─┐      │
│     │                                                (QLocalSocket)  │      │
│     ▼                                                                │      │
│  playback: SourceReaders (decode) → FrameCache → Compositor (RHI)    │      │
│            AudioReaders → Mixer → miniaudio device                   │      │
│  export: RenderJob (own QRhi + readers) → Encoder → Muxer            │      │
└────────────┬────────────────────────────────────────────────────────┼──────┘
             │ QProcess, JSON lines                                    │
   cutline-worker (×N): probe, thumbnails,                     cutline-mcp (stdio MCP server,
   frame index, waveform peaks, proxies                        launched by Claude Desktop/Code)
             
   cutline-cli: headless render/probe/validate (tests, batch export)
```

**Source layout:**

```
src/core      Qt Core only: time, project model, commands, undo, serialization, migrations
src/media     FFmpeg wrappers: demux/decode/hwaccel, probe, fingerprints, cache layout
src/worker    cutline-worker executable (background jobs, crash-isolated)
src/render    RHI compositor, effect registry + shaders (precompiled .qsb), text rendering
src/audio     readers, mixer, DSP (gain, fades, biquad EQ, tempo), meters, device output
src/export    render jobs, encoder registry/detection, OpenH264 wrapper, muxing, queue
src/app       QML UI, view models, timeline QQuickItem, docking, themes, actions/shortcuts
src/control   ControlServer (in-app), permission model, edit plans, tool implementations
src/mcp       cutline-mcp executable (MCP protocol ↔ ControlServer bridge)
src/cli       cutline-cli executable
tests/        Qt Test suites per module + fixtures generator
packaging/    windows/ (Inno Setup), macos/ (dmg), linux/ (AppImage, flatpak, deb, rpm, PKGBUILD)
docs/         architecture, project format, MCP, licensing, network, user guide
```

The rule that holds it together: **every change to a project goes through a `Command` in `src/core`**. The UI,
keyboard shortcuts, the CLI and Claude all call the same commands. That is why every Claude edit can be undone,
why every displayed shortcut is backed by a real action, and why the core can be tested without a GPU.

## 7. Core model (`src/core`)

**Time.**
- Every position is an `int64` count of **ticks**, where 1 tick = 1/705,600,000 s (a "flick").
- Every common frame duration is an exact whole number of ticks: 24, 25, 30, 50 and 60 fps, plus 23.976, 29.97 and
  59.94 (1001-based rates), and audio samples at 44.1 kHz and 48 kHz.
- So 23.976 timelines never drift. JSON stays exact up to about 147 days.
- Frame rates are stored as rationals (`24000/1001`).
- Timecode formatting and parsing supports drop-frame and non-drop-frame.

**Entities:**
- `Project` holds the media pool, bins and sequences.
- `MediaItem`: id, display name, path (absolute and project-relative), fingerprint, probe metadata, proxy state.
- `Bin`: a folder tree of media and sequences.
- `Sequence`: format (width, height, frame rate, sample rate, channel layout), video tracks (V1 is the bottom layer),
  audio tracks, markers.
- `Track`: id, kind, name, locked, enabled/muted, solo (audio), gain and pan (audio), display height.
- `Clip`: id, source (a media reference or a generator such as text or a solid colour), timeline start, duration,
  source in-point, speed (rational), reverse flag, enabled, colour label, link id (to pair A/V), group id, transform
  parameters, opacity, blend mode, effect stack, audio gain, fade in/out.
- `Transition`: belongs to a track and sits between two adjacent clips. It has a type, duration, alignment (centre,
  start or end) and parameters. It can only use available handles; if there aren't enough, the command fails with a
  clear message.
- `Marker`: time, duration, name, colour, note.
- `Param<T>`: either a static value or keyframes `{time (clip-relative ticks), value, interpolation: hold | linear | ease}`.

**Invariants**, checked after every command (also in release builds, as an error that is reported, not a crash):
- clips on one track never overlap
- every transition references two adjacent clips with enough handles
- ids are unique
- linked clips move together unless linking is overridden

**Commands:**
- Small classes with `apply`/`revert`. They validate first and fail with a message, never a partial edit.
- The `UndoStack` supports merging (a whole drag is one step) and macros (a Claude plan is one step).
- The full set: insert, overwrite, append, move, trim (head/tail), ripple trim, roll, slip, slide, split, delete,
  ripple delete, lift, paste, duplicate, group/ungroup, link/unlink, set-param, keyframe add/remove/move, add/remove
  track, track props, marker ops, transition ops, media import/remove/relink, bin ops.
- Each milestone adds the commands it needs.

**Model notifications:** core emits fine-grained change signals so view models and the compositor's caches update
only what changed. Every applied command also bumps a project **revision number**, which Claude's plans check (§15).

**Editing behaviour (call):**
- **New project:** the first sequence's format matches the first video clip added. With no video, it is 1080p at 30 fps,
  48 kHz stereo. It can be changed in sequence settings.
- **Timecode** starts at `00:00:00:00`. `01:00:00:00` is an option per sequence.
- **Dropping media** onto the timeline *overwrites* by default. Holding **Ctrl** while dropping *inserts* and shifts
  later clips.
- A video file with audio becomes a **linked** video clip and audio clip(s).
- Stills default to 5 s. Image and text clips can be stretched to any length.
- **Ripple edits** (ripple delete, ripple trim, insert) shift later clips on the edited tracks and their linked tracks
  only, as Resolve does. Other tracks are never moved, and locked tracks are never touched. There is no sync-lock
  in 1.0.
- **Snapping targets:** clip edges, the playhead, markers, and in/out marks. It is toggled with **S**, and holding
  Shift while dragging suspends it temporarily.
- **Audio:** mono and stereo clips mix into a stereo master, and 5.1 sources are downmixed. Export is stereo.
- **Subtitles** live on a video track flagged as a subtitle track and holding text clips. They are burned into the
  export, and an optional `.srt` sidecar is written.
- **Removing media that is in use** asks first, then removes its clips in the same undo step.
- **Clipboard:** copied clips go to the system clipboard as `application/x-cutline-clips` JSON, so they paste between
  projects and CUTLINE windows.
- **Undo history** lasts for the session (unlimited, memory-capped) and is not saved in the project file.
- **Locking a project:** opening a project writes `<name>.cutline.lock` holding the PID. A second CUTLINE opening it
  offers read-only, or taking over if that process is gone.

## 8. Project file (`.cutline`)

- UTF-8 JSON, versioned, human-readable, written atomically with `QSaveFile` (write to a temp file, flush, then rename).
- The full spec goes in `docs/project-format.md`. Before 1.0 the format only grows by adding fields that have
  defaults, and stays at version 1. A JSON Schema (`schema/cutline-project-v1.schema.json`) is written when the
  format freezes in M10. The migration framework arrives with the first change that needs one.

```json
{
  "format": "cutline.project",
  "version": 1,
  "createdWith": "CUTLINE 0.1.0",
  "id": "6f1c…",
  "name": "Summer Edit",
  "settings": { "defaultSequence": "s1", "proxyMode": "auto" },
  "media": [{ "id": "m1", "name": "beach.mp4",
              "path": { "absolute": "D:/Footage/beach.mp4", "relative": "../Footage/beach.mp4" },
              "fingerprint": "sz:52428800|mt:…|h:9a3f…", "kind": "video",
              "probe": { "width": 3840, "height": 2160, "frameRate": [30000, 1001], "durationTicks": 0, "rotation": 90 } }],
  "bins": [{ "id": "b1", "name": "Drone", "items": ["m1"] }],
  "sequences": [{ "id": "s1", "name": "Main",
                  "format": { "width": 1920, "height": 1080, "frameRate": [30, 1], "sampleRate": 48000, "channels": 2 },
                  "tracks": [{ "id": "v1", "kind": "video", "name": "V1", "locked": false, "enabled": true, "clips": [] }],
                  "markers": [] }],
  "ui": { "playheadTicks": 0, "zoom": 1.0 }
}
```

- **Fingerprint:** file size, modification time, and a hash of the first and last 1 MiB. It is used for cache keys
  and relinking.
- **Migrations:** run step by step (v1→v2→…) on load. A file from a *newer* CUTLINE is refused with a clear message
  and never rewritten.
- **Corrupt files:** a precise error naming the line and field, plus an offer to open the latest backup.
- **App settings** use `QSettings`. **Caches** are content-addressed folders keyed by fingerprint:
  `cache/<fp>/{thumbs.bin, peaks.bin, index.bin, proxy.mov, manifest.json}`.
- **No SQLite in 1.0 (call):** the cache layout and `QSettings` cover the need with less to go wrong.

## 9. Media pipeline (`src/media`, `src/worker`)

- **Import:** files and folders, by dialog or drag-and-drop.
  - Video: mp4, mov, mkv, webm, avi, mxf, m4v, mts.
  - Audio: wav, mp3, aac, m4a, flac, ogg, opus.
  - Stills: png, jpg, bmp, tiff, webp.
  - Validation: extension allow-list, a successful probe, and sane streams (dimensions ≤ 16384, duration > 0, codec
    decodable). Rejected files are listed with a reason.
- **`cutline-worker` processes (call):** the first probe, thumbnails, the frame index, waveform peaks and proxies all
  run in separate worker processes. A malformed or hostile file can crash a worker but not the editor. Workers can be
  killed to cancel, prioritised (what's visible first), and capped in number (cores − 2).
- **Metadata:** codec, profile, resolution, frame rate (detects VFR), duration, bitrate, pixel format, colour space and
  range, **rotation from the display matrix** (phone footage), audio channels and rate, and creation time.
- **Frame index:** a demux-only scan of packet timestamps and keyframes (fast, no decoding). It gives frame-accurate
  seeking and handles VFR.
- **Thumbnails:** a filmstrip at several densities, 160 px tall, plus a poster frame.
- **Waveforms:** multi-resolution peak pyramid (min/max/RMS per 256 samples and coarser levels) for any zoom.
- **Proxies (call):** MJPEG in `.mov` at 720p (540p and 1080p optional) with PCM audio. Every frame is a keyframe and
  decodes quickly, with no codec patent issues. There is a toggle (Off / Auto / Always), and export always uses the
  originals.
- **Organisation:** bins, search by name, type filter, sort (name, date, duration), and a "used in timeline" badge.
- **Missing media:** detected on open. The relink dialog searches a chosen folder by fingerprint, then by name and
  duration.

## 10. Playback and rendering (`src/render`, `src/audio`)

**Decode.**
- Each active clip has a `SourceReader`: demux, decode (hardware where available: D3D11VA, VideoToolbox, VAAPI, NVDEC;
  otherwise software), then hand frames to the `FrameCache`.
- The cache is an LRU with a RAM budget, defaulting to 25% of RAM and capped at 4 GB.
- To show a frame, the reader seeks to the keyframe before it, decodes forward to the exact timestamp, and prefetches
  ahead in the playback direction.
- Reverse playback decodes a whole GOP, then plays it backwards.

**Compositor.** For each output time:
1. Work out which clips are active (bottom track to top).
2. Map timeline time to source time: `(t − start) × speed + in`, reversed if needed. Pick the frame with timestamp ≤
   that time.
3. Upload the YUV planes (NV12, P010, YUV420P, YUV422P, etc.) as textures and convert to RGB in a shader, respecting the BT.601,
   709 or 2020 matrix and limited or full range.
4. Run the effect stack (ping-pong render targets), then transform, crop and blend onto the canvas.
5. For a transition, render both sides into temporary targets and mix them with the transition shader.

Further rendering details:
- **Working space:** RGBA16F, Rec.709 display-referred.
- **Text:** laid out and drawn with `QPainter`/`QTextLayout` into an image, uploaded as a texture, and cached while the
  text is static.
- **One renderer, everywhere (call):** the same compositor draws the viewer (in a `QQuickRhiItem` on the scene-graph
  render thread) and exports (on its own offscreen `QRhi` in the export thread). Tests and machines with broken GPUs
  use software rasterisers: WARP on Windows, llvmpipe/lavapipe on Linux. This replaces the "CPU reference renderer"
  floated earlier in chat, so no effect is written twice.
- **Shaders** are written once in Vulkan-style GLSL and precompiled to `.qsb` at build time. The GPL Qt Shader Tools
  library is never linked at runtime.

**Audio.**
- Readers decode and resample (libswresample) to the sequence rate as float32 planar.
- Speed changes use a pitch-preserving tempo filter (libavfilter `atempo`, LGPL) per reader.
- The mixer applies clip gain, fades and EQ, then track gain, pan, mute and solo, then the master bus with a safety
  limiter, and feeds peak meters.
- EQ is CUTLINE's own RBJ biquads: low shelf, three peaking bands, high shelf.
- Output goes through **miniaudio (call)**: WASAPI, CoreAudio, PulseAudio/ALSA (PipeWire through its Pulse layer).
  It is chosen for low latency and an exact sample clock.

**A/V sync.** During playback, the audio device's sample clock is the master and video follows it. With no audio,
a monotonic clock is the master. Late frames are dropped, and a dropped-frame counter shows in the viewer.

**Transport.**
- Play/pause, JKL shuttle: L or J repeated steps 1×, 2×, 4×, 8×; K stops; K+L / K+J play at slow speed.
- Frame step, edit-point jumps, home/end, scrubbing with audio scrub, and a loop toggle.
- Preview quality: Full, ½ or ¼ canvas resolution, plus the proxy toggle.

## 11. Effects (`src/render/effects`)

Effects are data-driven. Each effect has a descriptor: id, version, name, category, parameters (type, range, default,
unit, keyframable) and shader. Because of this:
- the inspector UI generates itself from the descriptor
- MCP can list and set effects by id
- project files reference effects by `id@version`

Shipped effects:
- **Built into every clip:** Transform (position, scale with X/Y link, rotation, anchor, flip), Crop (left, right, top,
  bottom, feather), Opacity, Blend mode (normal, add, multiply, screen, overlay, darken, lighten, difference).
  Transform and crop have handles in the viewer.
- **Colour:** Basic Correction (exposure, brightness, contrast, saturation, temperature, tint) and Colour Wheels (lift,
  gamma, gain, offset).
- **Blur and sharpen:** Gaussian blur (separable) and Sharpen (unsharp mask).
- **Keying:** Chroma Key (key colour picked from the viewer, tolerance, softness, spill suppression, matte view).
- **Transitions:** cross dissolve, dip to colour, wipe (angle, softness), slide. For audio, crossfade with curve choice.
- **Generators:** Text/Title (font, size, colour, stroke, shadow, background box, alignment, safe-area guides) with
  animated presets built on keyframes (fade, slide, scale, typewriter, lower third), and Solid Colour.
- **Time:** constant speed from 0.1× to 10× (nearest frame), reverse, pitch-preserving audio.
- **Audio:** gain (keyframable), fade handles, EQ.
- **Scopes** (colour workspace): luma waveform, RGB parade, vectorscope, histogram. Computed from a downscaled readback
  of the viewer frame.

Keyframes are edited in the inspector (diamond toggle, previous/next keyframe), with keyframe ticks drawn on the
selected clip. Every effect is checked in tests by rendering through the export path (§16). Nothing counts as done
until it matches in export.

**Effects browser:** categories on the left, a thumbnail grid on the right. Each thumbnail renders the effect on the
selected clip's current frame, or on a sample frame if nothing is selected. Apply by drag, double-click or Enter.

## 12. Export (`src/export`)

- **Render jobs** take an immutable snapshot of the sequence when queued, so editing during export is safe. Each job
  runs in the background with its own `QRhi`, readers and encoder.
- **Encoder detection:** at startup and on demand, each candidate encoder (NVENC, QSV, AMF, VideoToolbox, VAAPI,
  OpenH264, SVT-AV1, libx264/x265 if present, ProRes) gets a one-frame test open. The UI offers only what actually
  works on this machine and says why the others are missing.
- **Containers and codecs:** MP4 (H.264, H.265, AV1) and MOV (H.264, H.265, ProRes 422 via FFmpeg's `prores_ks`).
  Audio: AAC (FFmpeg's native encoder), Opus, and PCM in MOV.
- **Settings:**
  - Presets: 720p, 1080p, 1440p, 4K UHD, or match the sequence. 4K is offered when the sequence or sources allow it.
  - Frame rate: match, or the standard rates.
  - Rate control: target bitrate or constant quality, plus a keyframe interval.
  - Audio: codec, bitrate and sample rate.
  - Range: whole sequence, in/out, or between markers.
- **Colour conversion** from RGB to YUV happens on the GPU before readback.
- **Truthful progress:** frames encoded / total, render fps and ETA come straight from the encode loop.
- **Writing and verification:**
  - Output goes to `name.partial.<ext>`. After the encoder flushes, the file is re-probed: duration within one frame,
    expected streams present.
  - Only then is it renamed and success reported.
  - On a failure, the job shows FFmpeg's error text plus a plain explanation, and the partial file is deleted.
- **Cancel** asks for confirmation once a render is running, stops after the current frame and removes the partial
  file.
- **Queue:** several jobs run in order, with history kept for the session and the queue persisted in app data.

## 13. Interface (`src/app`)

**Look.** This follows the reference analysis.
- Neutral, untinted greys, so the footage is the only colour on screen.
- Colour shows media kind: video blue, audio green, titles sand, generators violet.
- **One accent: red, chosen by the user on 2026-11-19 after the mockup.** Dark theme `#E5484D`, light theme `#C62F35`.
  It marks the playhead, selection, focus rings and the logo's cut mark.
  - Because the accent is red, error colours never stand alone. Errors always carry the warning icon and words, and
    destructive buttons are outlined with an icon. Primary buttons are neutral, so a red control never just means
    "go".
- **Themes:** the built-in Dark (default) and Light, plus **custom themes**.
  - A custom theme is a palette JSON in the same format as the built-ins. Settings imports, exports and selects them,
    and they are stored in `<config>/themes/`.
  - Importing validates the file. If text contrast falls below WCAG AA or the surfaces aren't neutral grey (which can
    throw off colour judgement), the user sees a warning but can still use the theme.
  - Later, a community **theme library** (a public repo of user-submitted themes, browsed and downloaded in-app as an
    opt-in network connection) makes themes easy to share. There is no built-in Aero theme.
- 1px dividers, no cards, a 4px spacing grid, 24px inspector rows, and fixed keyframe and reset columns.
- **Typography:** Inter for the UI, and JetBrains Mono for timecode, with dimmed leading zeros.
- **Icons:** CUTLINE's own SVG set for editing tools (blade, ripple, roll, slip, slide, snapping, link, etc.), plus
  restyled Phosphor icons for generic glyphs.
- **Dark theme** is the default. **Light theme** uses neutral light greys (never pure white). The viewer surround stays
  mid-dark grey in both themes so colour judgement isn't thrown off.
- UI scale goes from 90% to 150%, with high-DPI support and a reduced-motion setting.

**Layout (default "Edit" workspace):**

```
┌ menu bar ─────────────────────────────────────────────────────────────────────┐
├ [Media][Effects]      project name · workspace: Edit ▾       [Mixer][Inspector]┤
├──────────────┬─────────────────────────────────────────┬───────────────────────┤
│ Media / FX   │                Viewer                   │      Inspector        │
│ (tabs)       │                                         │                       │
│              │   ◀◀ ◀ ▶ ▶▶   00:00:12:04  ½ ▾  proxy   │                       │
├──────────────┴──────────────┬──────────────────────────┴───────────┬───────────┤
│ tools · snapping · zoom     │  01:00:12:04                          │  Mixer    │
│ V2 ▢ 🔒 👁 │ clips…         │                                       │ (hidden   │
│ V1 ▢ 🔒 👁 │ clips…         │                                       │  by def.) │
│ A1 S M 🔒  │ waveforms…     │                                       │           │
├─────────────────────────────┴───────────────────────────────────────┴───────────┤
│ status: proxy jobs · render queue · Claude: off · dropped frames                │
└─────────────────────────────────────────────────────────────────────────────────┘
```

- **Workspaces (call):** Edit, Colour (scopes and inspector enlarged), Audio (mixer enlarged), Deliver (export
  settings and render queue). Users can save their own.
- **Viewer (call):** a single viewer with **Source** and **Timeline** tabs, which suits laptop screens.
  - Double-clicking media opens it in Source mode, where **I** and **O** set in and out marks for the next drop or
    insert.
  - Timeline mode uses I and O for export ranges.
  - Windows at least 1600 px wide can switch the Edit workspace to side-by-side source and timeline viewers.
- **Panels:** Media, Effects (Video / Transitions / Titles / Audio tabs), Viewer, Inspector, Timeline, Mixer, Scopes,
  Export & Queue, History, Claude.
- **Docking:** QML has none built in, so CUTLINE builds its own.
  - M2: a fixed frame of five areas (left, viewer, right, timeline, side) with resizable splits. Each area holds a
    tab group of panels, and an empty area hides. Panels can be shown, hidden and moved between areas ("Move panel
    to…"), and workspaces save and restore all of it as JSON.
  - M10: drag-and-drop docking, generalising the frame into a free layout tree if testing shows it's needed.
- **Timeline** is a C++ `QQuickItem` that draws tracks, clips, filmstrips, waveforms, fades, transitions and
  keyframe ticks with scene-graph nodes. Only the visible range is drawn, so 1,000+ clips stay smooth. Headers,
  toolbar and overlays are QML.
- **Menus:** a styled QML MenuBar on every platform through M9, drawing shortcut text from the ActionRegistry.
  The native macOS menu bar comes in M10, so the Mac-specific polish happens in one place.
- **Small screens:** the default layout must work at 1366×768. The mixer and scopes are hidden by default, and panels
  collapse to tabs.
- **Empty states** say what to do ("No media yet. Import files (Ctrl+I) or drop them here."). A disabled control has a
  tooltip saying why.
- **Accessibility:** every control has an accessible name and role, visible focus, WCAG AA text contrast, full keyboard
  operation (§14) and no information carried by colour alone (clip kinds also differ by icon and pattern).

## 14. Keyboard and interaction system

**ActionRegistry** (`src/app/actions`) is the single source of truth. Each action has:
- an id (e.g. `timeline.splitAtPlayhead`), category, title and description
- default shortcuts per platform
- a context
- an enabled predicate, a checked state for toggles, and a handler that calls a core command or a UI operation

Menus, toolbar buttons, tooltips ("Split at playhead · Ctrl+B"), the shortcuts page, the keyboard map and the
command list all read from it. **An action is registered only when it is implemented**, so no unimplemented shortcut
can appear anywhere. A test checks that every menu item and every listed shortcut resolves to a registered action with
a handler.

**ShortcutRouter**
- An event filter on each `QQuickWindow` handles `ShortcutOverride` and `KeyPress`.
- It resolves the active context in priority order:
  1. an operation in progress (dragging or trimming): only Escape and modifier changes
  2. a modal dialog or popup: the dialog's own keys, and Escape closes it
  3. **text entry** (focus on TextInput/TextEdit/TextField or anything marked `cutlineTextEntry`): editing keys go to
     the field, including Space, Delete, Backspace, letters, Ctrl+A/C/X/V/Z/Y and arrows. Only global commands with
     non-editing modifiers pass through (Ctrl+S, Ctrl+N, Ctrl+O, Ctrl+E, Ctrl+Q, F11).
  4. actions scoped to the focused panel (Timeline, Viewer, Media, Inspector, Effects, Mixer, History, Export,
     Claude), which win over global ones
  5. global actions
- An action's contexts are a set of panel flags, and an empty set means global. Two actions conflict when they share a
  key sequence and their sets overlap (global overlaps everything). Playback keys (Space, JKL, arrows) are global
  actions that don't work in text entry. Each action carries a `worksInTextEntry` flag, set only for commands such as
  Save.

**Defaults:** the keyboard spec's tables, with these resolutions **(call)**:
- **Tab** moves focus inside forms and dialogs as normal. It cycles panels only when the timeline or viewer has focus.
  **F6 / Shift+F6** and **Ctrl+Tab / Ctrl+Shift+Tab** cycle panels from anywhere (F6 is the Windows convention).
- **Escape** works as a stack: cancel the operation in progress, then close the popup or dialog, then clear the
  selection. During an export, the cancel confirmation is a dialog, so Escape dismisses the dialog and does *not*
  cancel the render.
- **C** toggles crop handles on the selected clip in the viewer. **T** is the trim tool (ripple/roll at edit points).
  **V** is select and **B** is blade.
- **+ / −** also accept `=` and the numpad keys.
- **Shift+1…6** match the physical digit row (native virtual key or scan code), because Shift+1 produces "!" on US
  layouts and the digits themselves need Shift on AZERTY.
- **macOS:** Ctrl becomes ⌘ automatically. Fullscreen is ⌃⌘F, Settings ⌘, and Quit ⌘Q. On Windows, Alt+F4 also quits.
- **Mouse gestures** (Ctrl+wheel zoom, Alt+drag duplicate, Shift+wheel horizontal scroll) appear in the reference as
  fixed, not rebindable, in 1.0.

**Profiles** are JSON files `{name, base: "default", overrides: {actionId: [sequences]}}` in the app config folder,
with import/export as `.cutline-keys.json`. The active profile name is shown on the settings page. Changes persist
immediately.

**Keyboard Shortcuts page:**
- search by action or key
- a category list and a click-to-record field
- live conflict detection: the same sequence in overlapping contexts, where Global overlaps everything but Timeline
  and Media don't overlap each other
- a conflict dialog: reassign (remove from the other action), or cancel
- restore defaults per action or for all

**Visual keyboard:**
- Physical geometries: ANSI-104, ISO-105, JIS-109, ABNT2 and the Mac variants.
- Key captions come from the *actual system layout*: Windows `MapVirtualKeyExW` + `ToUnicodeEx`, macOS
  `UCKeyTranslate` on the current input source, Linux xkbcommon. On Wayland, where the keymap isn't always
  reachable, there is a layout picker by name.
- The geometry is auto-picked from the layout and can be overridden.
- Modifier toggles show the Ctrl, Shift, Alt and Ctrl+Shift layers.
- Assigned keys are tinted by category and conflicts are outlined. Hovering a key shows its actions; clicking a key
  opens its assignments.

## 15. Claude integration (`src/control`, `src/mcp`)

**Shape:**
- `cutline-mcp` is a small stdio MCP server that Claude Desktop or Claude Code launches.
- It connects to the running CUTLINE through a per-user local socket (a named pipe on Windows, a Unix socket on
  macOS and Linux).
- If CUTLINE isn't running, tools return "CUTLINE is not running". The bridge never launches the app itself.
- The protocol (initialize, tools/list, tools/call, notifications) is implemented directly in C++ on Qt JSON. The
  protocol version is pinned in `docs/mcp.md` **(call)**.

**Off by default.**
- Settings → Claude → "Allow Claude to connect" starts the control server.
- Each new connection asks the user in the app: Allow for this session / Always for this project / Deny.
- The status bar shows Off / Waiting / Connected (client name), with a Disconnect button.
- Turning the setting off closes the socket.
- An "Add to Claude Desktop" helper shows the exact config entry it would write and writes it only on click.

**Permissions** are per project and can be changed at any time:

| Permission | Default | Covers |
|---|---|---|
| Read project structure | on when connected | sequences, tracks, clips, markers, effect params, media names/durations |
| Read file paths | off | full paths (they can reveal user names and folders). Off means file names only |
| Analyse media content | off | audio levels/silence detection, frame thumbnails sent to Claude |
| Propose edits | on | creates edit plans; nothing changes without approval |
| Auto-apply additive edits | off | markers, new clips/titles in empty space; destructive edits always need approval |
| Import files / start exports | off | each call still needs per-action confirmation showing the path |

**Tools (1.0):**
- **Read:** `get_project_info`, `list_media`, `search_media`, `inspect_clip` (media item or timeline clip),
  `inspect_timeline`, `get_project_duration`, `inspect_audio` (peak/RMS/silent ranges from the peak pyramid),
  `list_effects`, `get_export_status`.
- **See:** `get_frame` returns a frame at a given time as an image (max 768 px), under the "Analyse media content"
  permission.
- **Plan:** `suggest_edit` (a titled plan with rationale and operations) and `get_plan_status`.
- **Edit operations**, which are always queued into a plan: `create_clip`, `split_clip`, `move_clip`, `trim_clip`,
  `delete_clip`, `add_text_overlay`, `add_marker`, `set_clip_property`, `add_transition`, `create_subtitles` (from
  supplied cues or SRT text, onto a subtitle track, with optional `.srt` export).
- Times are accepted and returned as seconds, timecode and frames.

**How plans work (call):**
- `suggest_edit {title, rationale, operations[]}` is the main path.
- An operation can refer to the result of an earlier one in the same plan as `"$1"`, `"$2"`, … (for example, split a
  clip, then move the new right-hand part).
- Calling an edit tool on its own creates a one-operation plan, or applies it straight away if it is additive and
  auto-apply is on.
- Every plan records the project revision it was made against. If the user has edited since, applying re-validates
  every operation, and any failure blocks the whole plan with the reason, so Claude can re-plan.
- Only one Claude client is connected at a time.

**Rules:**
- Every argument is validated against its JSON Schema and then against the model; errors say what was wrong.
- Claude can only reference media already in the project, by id.
- There is no shell, no arbitrary file reading and no arbitrary output paths.
- Source media is never written.

**Edit plans UI** (Claude panel):
- Each plan shows its operations in plain language ("Split *beach.mp4* at 00:00:12:04 on V1"), with destructive ones
  flagged.
- Affected clips are outlined on the timeline as ghosts.
- Actions: Apply all, Apply selected, Reject.
- Applying runs all the operations as one undo step named "Claude: <plan title>". If any operation fails validation,
  nothing is applied and the plan shows why.

**Optional in-app Claude using the user's own API key:**
- The key is stored in the OS keychain through QtKeychain: Credential Manager, Keychain, or Secret Service/KWallet.
- A chat panel calls the Anthropic API directly and executes the same tools in-process, under the same permissions and
  plan approval.
- Wording never claims this uses a Claude subscription. The `claude-api` reference is consulted when M9 is built,
  for current model ids and SDK usage.

## 16. Testing

- **Qt Test + CTest** for everything **(call)**. One framework covers C++, signals and simulated key and mouse events.
- **Fixtures are generated, never downloaded:**
  - `tests/fixtures/make_fixtures` uses the FFmpeg libraries to write short clips in always-available encoders (FFV1,
    MJPEG, PCM, plus H.264/HEVC when an encoder exists).
  - Every frame carries its **frame number as a pixel barcode**, and the audio has known tones and click markers. That
    lets tests check exact frame accuracy, frame order, dropped or duplicated frames, and A/V offset.
  - Variants: VFR, rotated (display matrix), odd sizes, 23.976, 29.97 drop-frame, mono/5.1, still images, corrupt and
    truncated files.

**Suites:**

| Area | What is proven |
|---|---|
| Core | every command: apply → revert gives a byte-identical serialization; random command sequences (fuzz) then undo-all and redo-all; invariants; snapping math; timecode incl. drop-frame |
| Serialization | round-trip; migrations; newer-version refusal; corrupt file errors; atomic save (killed mid-save leaves the old file intact) |
| Recovery | simulated crash marker + autosave → recovery offered and restores state |
| Media | probe of every fixture; rejected files rejected with reasons; worker crash doesn't affect the app; frame index; thumbnails; peaks |
| Playback | seek to N decodes barcode N; reverse; speed mapping; A/V sync within one audio buffer |
| Effects | GPU render on software rasterisers compared with analytic expectations within tolerance (brightness, contrast, saturation, key alpha, blur energy, transition midpoint mix, transform positions) |
| Audio | gain/fade/mix maths; EQ frequency response at test tones; mute/solo; tempo keeps duration |
| Export | render barcode timelines → decode the output → check frame count, order, resolution, fps, duration, audio sync, and that effects appear; cancel leaves no file; failure (bad path, missing encoder) reports an error and never success; licence check (§4) |
| MCP | protocol handshake; every tool's schema; every tool against a test project; permission denials; plan apply/reject; undo of an applied plan; validation errors; no path leakage when the path permission is off |
| Shortcuts | every default shortcut fires its action in its context; text fields swallow Space/Delete/Ctrl+A/letters; modal capture; Escape stack; custom assignment persists across a registry restart; conflicts detected; profile import/export; digit-row and AZERTY/QWERTZ simulated events; every displayed shortcut maps to an implemented action |
| UI acceptance | scripted harness opens a project, performs actions and saves window grabs for review. On Windows the real window is also checked by hand with computer use |

CI (GitHub Actions) runs everything headless on Windows (WARP), Linux (llvmpipe; containers for Debian, Ubuntu,
Fedora and Arch) and macOS.

**Definition of done for any feature:** implemented, reachable from the UI (menu, button and/or shortcut), tested,
documented, and checked in a real build on Windows.

## 17. Performance, reliability, privacy

**Performance targets on the hardware floor:**
- 1080p30 H.264 single-track playback at full quality with no dropped frames
- 4K playback via proxies
- a 1,000-clip timeline scrolls and zooms at 60 fps
- warm start under 3 s
- the UI thread never blocks for more than 100 ms, because all I/O happens elsewhere

**Hardware-aware defaults (user: "a real mix" of computers):**
- On first launch (and from Settings ▸ Performance ▸ Re-check), CUTLINE builds a `HardwareProfile`: GPU adapter
  and whether it's integrated or discrete, VRAM where reported, RAM, CPU cores, and which hardware decoders and
  encoders actually open.
- That puts the machine in a tier:
  - **Light:** integrated GPU or ≤ 8 GB RAM
  - **Standard**
  - **Strong:** discrete GPU and ≥ 16 GB RAM
- The tier sets the defaults, each of which can be changed:

  | Default | Light | Standard | Strong |
  |---|---|---|---|
  | Automatic proxies | above 1080p30 | above 4K30 | off |
  | Preview quality | ½ | Full | Full |
  | Frame cache | 15% of RAM | 25% | 25% |

- Preview quality is built in M3, and the tier mapping and proxies in M8.

**Reliability:**
- **Autosave:** every 2 minutes, to app data, never over the user's file.
- **Crash recovery:** a session marker plus the autosave lead to a recovery prompt on the next launch.
- **Backups:** the last 10 saves are kept in `<project>.backups/`.
- **Crash logs:** local minidump on Windows, crash log elsewhere. Never uploaded.
- **Background work** can be cancelled throughout.

**Privacy and security:**
- `docs/network.md` lists every connection, and there are exactly four, all opt-in:
  1. the Anthropic API when in-app Claude is enabled
  2. the Cisco OpenH264 download when clicked
  3. an update check against GitHub Releases, off by default
  4. the speech-model download for auto captions, when clicked
  - The community theme library (after 1.0) will add a fifth, also opt-in.
- No telemetry.
- Source media is opened read-only.
- Temporary files go in a per-app temp folder with random names and are cleaned at start and exit.
- Dependencies are pinned and listed in `docs/dependencies.md`, and a CI job flags outdated versions.

## 18. Build, packaging, distribution

- **Dev and Windows/macOS builds:**
  - Qt from the official binaries (aqtinstall).
  - FFmpeg and small libraries through a **vcpkg manifest** with a pinned baseline and LGPL-only features **(call)**.
  - Binary caching in CI.
- **Windows:** Inno Setup installer (per-user by default, no admin; `.cutline` file association; uninstaller) and a
  portable `.zip`. Unsigned at first, so SmartScreen will warn. Signing can be added later.
- **macOS:** `.app` via macdeployqt in a `.dmg`, ad-hoc signed (required on Apple Silicon). Not notarised until there
  is an Apple Developer ID, so the first launch needs "Open Anyway" in System Settings, which the docs explain.
- **Linux:**
  - AppImage and Flatpak (bundling Qt; Flathub submission later) for every distro, including older ones such as
    Ubuntu 24.04, whose Qt 6.4 is too old.
  - Native packages built against the distro's Qt (≥ 6.8) and FFmpeg: `.deb` for Debian 13+ and Ubuntu releases
    with Qt ≥ 6.8, `.rpm` for current Fedora, and a `PKGBUILD` for the AUR.
- **Releases:** GitHub Releases with SHA-256 checksums and a CHANGELOG. The public GitHub repo is created only when the
  user says so.

**Website:**
- It already exists and isn't part of this repository. A separate session runs it: a coming-soon page in
  `Documents\BUILDZ\cutline-site` (Astro static site with a React island for a React Bits button), live on Vercel,
  linking the user's Ko-fi (ko-fi.com/almondsz).
- That site grows into the full download site from the spec. The editor project's job is to feed it:
  - **genuine screenshots** from `cutline --screenshot` (from M2), and later screen recordings, made with a demo
    project built from Blender Foundation open-movie footage under CC-BY, credited
  - release notes from `CHANGELOG.md`
  - download links and checksums from GitHub Releases
  - system requirements, install guides including the unsigned-app steps, privacy text from `docs/network.md`,
    and licence information from `docs/licensing.md`

## 19. Milestones

Each milestone ends with a working Windows build that is checked by hand, and green CI on all platforms. No milestone
adds controls that don't work; unfinished areas show honest empty states.

| # | Milestone | Ends with |
|---|---|---|
| 1 | **Foundations**: reference analysis ✓, this design, git repo, MIT licence, CMake/vcpkg skeleton, dev environment on this PC (with approval for each install), CI on Windows, macOS and Linux, licensing doc | a blank CUTLINE window built and launched on Windows; CI green |
| 2 | **App shell**: mockup review first, then design tokens and themes, fonts, icons, the five-area workspace frame and workspaces, menus, ActionRegistry + ShortcutRouter + Keyboard Shortcuts page, and New/Open/Save of v1 project files. Its panels are Viewer (empty canvas), Timeline (track headers), Inspector (sequence and track settings) and History, all doing real, undoable work | a real shell with working project files, undo and shortcuts |
| 3 | **Media and playback**: import, workers (probe, thumbnails, index, peaks), Media panel with bins and search, viewer with audio, frame step, JKL, scrubbing, quality settings | real camera/phone files play smoothly with sound |
| 4 | **Timeline + first export**: tracks, drag to timeline, move, trim, snapping, zoom, playhead, lock/mute/solo, multi-layer playback, basic H.264 MP4 export | an edit can be assembled and exported, verified by tests |
| 5 | **Editing and undo**: split, ripple delete, copy/cut/paste, duplicate (incl. Alt+drag), group, link, markers, tools V/B/T/C, edit-point navigation, history panel, visual keyboard | keyboard-only editing works end to end |
| 6 | **Effects**: transform/crop/opacity with keyframes and viewer handles, colour, blur/sharpen, chroma key, transitions, titles, speed/reverse, audio gain/fades/EQ, effects browser with live thumbnails, scopes | every effect matches in export, proven by tests |
| 7 | **Export engine**: queue, presets, encoder detection, H.264/H.265/AV1/ProRes, MP4/MOV, all settings, verification, cancel, errors, OpenH264 opt-in | reliable exports on every encoder this PC has |
| 8 | **Project management**: autosave, recovery, backups, missing media + relink, project settings, proxies | projects survive crashes and moved footage |
| 9 | **Claude and captions**: control server, cutline-mcp, permissions, edit plans, tools, setup helper, optional API-key mode, docs; local auto captions (whisper.cpp, opt-in model download), Shorts caption styles, `get_transcript` | Claude can propose an edit and the user can apply or undo it; one click captions a clip offline |
| 10 | **Polish and release**: performance pass against §17 targets, accessibility pass, drag-and-drop docking, Simple/Advanced interface level, installers for every platform, user guide, real screenshots and release info handed to the website | CUTLINE 1.0 release candidate |

After 1.0: a community theme library, and the other non-goals in §2.

A basic export comes in M4, earlier than the spec's order, so the shared render path is proven before effects are
built on it. M7 is then the full export engine. The shortcut system starts in M2 because menus and tooltips depend on
it.

## 20. Calls made for the user

These were decided by Claude under the user's instruction to choose the recommended option. Any of them can be changed.

1. Official binaries are LGPL-clean. H.264 and H.265 come from hardware encoders and OpenH264; x264 and x265 are used
   only if a system FFmpeg provides them (§4).
2. OpenH264 is loaded at runtime and downloaded from Cisco only on an opt-in click.
3. One renderer for preview and export; CI uses software rasterisers instead of a separate CPU renderer.
4. Background media jobs run in crash-isolated `cutline-worker` processes.
5. Proxies are 720p MJPEG.
6. Integer flick time base; frame rates stored as rationals.
7. No SQLite in 1.0.
8. Audio output through miniaudio.
9. Workspaces are named Edit, Colour, Audio, Deliver.
10. The accent is red (`#E5484D` dark / `#C62F35` light), chosen by the user. Custom themes are in; a community theme
    library comes later.
11. Fonts: Inter and JetBrains Mono. Icons: a custom editing set plus restyled Phosphor.
12. Tab/F6/Ctrl+Tab panel focus, the Escape stack, digit-row matching and macOS mappings (§14).
13. The Claude integration is off by default, with per-project permissions; file paths and media content are hidden
    unless allowed.
14. Hand-written MCP protocol layer rather than an SDK dependency.
15. Qt Test for all tests; fixtures are generated, with frame-number barcodes.
16. vcpkg for FFmpeg on Windows and macOS; distro libraries for native Linux packages; AppImage and Flatpak for
    everything else.
17. Apple Silicon first; Intel Mac best-effort.
18. Unsigned installers to start, with the workarounds documented.
19. A basic export moves to M4, and the shortcut system to M2.
20. The website stays the existing Astro/Vercel site in `BUILDZ\cutline-site`, fed with real screenshots
    (Blender open-movie demo footage) and release info from this repo.
21. Editing behaviour: overwrite on drop and Ctrl to insert; ripple on edited and linked tracks only; first-clip
    sequence format; 5 s stills; stereo master; session-only undo; a project lock file (§7).
22. A single viewer with Source/Timeline tabs, and I/O marks. Side-by-side viewers on wide windows.
23. Claude plans: an operation list with `$n` references, a revision check on apply, one client at a time, and a
    `get_frame` tool.

**Still needs the user later** (not design questions):
- approval of each install on this PC
- the GitHub account and repo name before anything is pushed. This also fixes the app id, e.g. `io.github.<user>.Cutline`
- any spending, such as code-signing certificates or an Apple Developer ID

## 21. Left to the builder

- Exact class boundaries inside each module, threading primitives, and cache eviction details.
- The shader code and parameter curves for each effect, within §11.
- Exact Qt, FFmpeg and vcpkg baseline versions, pinned when the environment is set up.
- Icon drawing and the final colour tokens, after the M2 mockup.
- Which Linux distro versions get native packages in CI, as long as §18's rules hold.
