# Licensing and codecs

CUTLINE's own code is under the [MIT License](../LICENSE). This page explains which third-party components
the official downloads contain, why each one is allowed, and how the build enforces it.

## The rule

The official downloads from GitHub Releases may contain only components whose licences are compatible with
distributing CUTLINE under MIT:

- permissive licences: MIT, BSD, Apache-2.0, zlib, BSL-1.0, ISC, and OFL-1.1 for fonts
- LGPL libraries, linked **dynamically**, so you can swap in your own build of the library

GPL components are never bundled. "Nonfree" FFmpeg components, which can't be redistributed at all, are never used.

## Components

| Component | Licence | How CUTLINE uses it |
|---|---|---|
| Qt 6 (Core, Gui, Quick, Quick Controls 2, Svg, Test) | LGPLv3 | dynamic linking; GPL-only Qt add-ons are not used |
| FFmpeg (avformat, avcodec, avutil, swresample, swscale, avfilter) | LGPL 2.1 or later | built without `--enable-gpl` and `--enable-nonfree`, linked dynamically |
| dav1d, libopus, SVT-AV1 (later milestones) | BSD-2 / BSD-3 / BSD-3-Clause-Clear + AOM patent licence | through FFmpeg |
| OpenH264 (later milestone) | BSD-2 | Cisco's own binary, downloaded only if you opt in, loaded at runtime |
| miniaudio (later milestone) | MIT-0 / public domain | vendored |
| QtKeychain (later milestone) | BSD-3 | stores an API key in the OS keychain |
| Inter, JetBrains Mono (fonts) | OFL-1.1 | bundled |

`docs/dependencies.md` lists the exact versions.

## H.264 and H.265 without GPL encoders

The best-known H.264 and H.265 encoders, x264 and x265, are GPL. Bundling them would force the downloads to be
released under the GPL. CUTLINE gets these formats in other ways instead:

1. **Hardware encoders:** NVIDIA NVENC, Intel Quick Sync, AMD AMF, Apple VideoToolbox and Linux VAAPI. Nearly
   every computer from the last ten years has one.
2. **OpenH264** for software H.264. Cisco pays the H.264 patent royalties only for its own prebuilt binary, and
   only when you download it from Cisco. So CUTLINE downloads it when you click a button in Settings, and never
   ships it.
3. **x265** for software H.265, only when the FFmpeg CUTLINE runs on already includes it. That happens with some
   native Linux packages built against the distro's FFmpeg. The official downloads never include it.
4. **AV1** through SVT-AV1 is always available, because AV1 is royalty-free.

## Linux distributions

- **Ubuntu, Debian and Arch** native packages use the distro's FFmpeg, which may be a GPL build. The About box
  shows the licence of the FFmpeg actually loaded.
- **Fedora's** own FFmpeg (`ffmpeg-free`) leaves out patented codecs. Install the Flatpak, which uses the
  full-codec FFmpeg extension, or use FFmpeg from RPM Fusion.

## How the build enforces it

- The official presets (`windows-msvc`, `macos`) set `CUTLINE_OFFICIAL_BUILD=ON`.
- In those builds, the test `tst_ffmpeginfo` reads `avcodec_license()` and `avcodec_configuration()` from the
  FFmpeg that was built. It **fails the build** if FFmpeg reports GPL, `--enable-gpl` or nonfree.
- `vcpkg.json` asks for the FFmpeg port with `default-features: false` and only the library features CUTLINE
  needs. It never asks for `gpl`, `nonfree`, `x264` or `x265`.

---

This page records the project's reasoning. It is not legal advice.
