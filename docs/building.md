# Building CUTLINE

CUTLINE builds with CMake (≥ 3.28) and Ninja, against Qt 6 (≥ 6.8) and FFmpeg. The version pins live in
`tooling/qt-version.txt` (Qt) and `vcpkg-configuration.json` (vcpkg baseline, which fixes the FFmpeg version).

## Windows 10/11 (primary)

### One-time setup

1. **Visual Studio 2022** (Community is fine) with:
   - *Desktop development with C++*
   - **Windows 11 SDK (10.0.26100)**
   - **C++ CMake tools for Windows**, which includes Ninja

   To add the last two to an existing install, run this from an elevated prompt:
   ```powershell
   & "C:\Program Files (x86)\Microsoft Visual Studio\Installer\setup.exe" modify --installPath "C:\Program Files\Microsoft Visual Studio\2022\Community" --add Microsoft.VisualStudio.Component.Windows11SDK.26100 --add Microsoft.VisualStudio.Component.VC.CMake.Project --passive --norestart
   ```
2. **Qt**, installed through aqtinstall:
   ```powershell
   python -m pip install --user aqtinstall
   python -m aqt install-qt windows desktop 6.10.3 win64_msvc2022_64 -O C:\Qt -m qtshadertools qtimageformats
   ```
3. **vcpkg** (with usage reporting off):
   ```powershell
   git clone https://github.com/microsoft/vcpkg C:\dev\vcpkg
   C:\dev\vcpkg\bootstrap-vcpkg.bat -disableMetrics
   [Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'C:\dev\vcpkg', 'User')
   ```

### Build, test, run

`scripts\build.ps1` enters the Visual Studio developer shell for you:

```powershell
scripts\build.ps1 -Target configure   # first configure builds FFmpeg from source: 20–40 minutes
scripts\build.ps1 -Target build
scripts\build.ps1 -Target test
scripts\build.ps1 -Target run
```

If Qt isn't at `C:\Qt\<version>\msvc2022_64`, set `QT_ROOT_DIR` first.

## Linux

Native builds use the distro's Qt and FFmpeg (preset `linux`, `CUTLINE_OFFICIAL_BUILD=OFF`). The official Linux
downloads (AppImage and Flatpak) come in a later milestone.

| Distro | Packages |
|---|---|
| Debian 13 / Ubuntu 26.04 | `build-essential cmake ninja-build pkg-config qt6-base-dev qt6-declarative-dev qt6-svg-dev qt6-shadertools-dev qml6-module-qtqml qml6-module-qtqml-workerscript qml6-module-qtquick qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-dialogs qml6-module-qtquick-templates qml6-module-qtquick-window libavcodec-dev libavformat-dev libavfilter-dev libavutil-dev libswresample-dev libswscale-dev libgl-dev libvulkan-dev libxkbcommon-dev` |
| Fedora | `gcc-c++ cmake ninja-build pkgconf-pkg-config qt6-qtbase-devel qt6-qtdeclarative-devel qt6-qtsvg-devel qt6-qtshadertools-devel ffmpeg-free-devel mesa-libGL-devel vulkan-headers libxkbcommon-devel` |
| Arch | `base-devel cmake ninja pkgconf qt6-base qt6-declarative qt6-svg qt6-shadertools ffmpeg vulkan-headers` |

Releases older than these (for example Ubuntu 24.04, which ships Qt 6.4) are too old for a native build. Use the
AppImage or Flatpak once they exist.

```sh
cmake --preset linux
cmake --build --preset linux
ctest --preset linux
```

## macOS (Apple Silicon)

1. Xcode Command Line Tools: `xcode-select --install`
2. `brew install cmake ninja pkg-config nasm`
3. Qt: `python3 -m pip install aqtinstall && python3 -m aqt install-qt mac desktop 6.10.3 clang_64 -O ~/Qt -m qtshadertools qtimageformats`
4. vcpkg: clone it, run `./bootstrap-vcpkg.sh -disableMetrics`, and export `VCPKG_ROOT`
5. Then:
   ```sh
   export QT_ROOT_DIR=~/Qt/6.10.3/macos
   cmake --preset macos && cmake --build --preset macos && ctest --preset macos
   ```

## Continuous integration

`.github/workflows/ci.yml` builds and tests on Windows, macOS and four Linux containers (Debian 13, Ubuntu 26.04,
Fedora, Arch) on every push.
