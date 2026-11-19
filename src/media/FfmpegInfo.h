#pragma once

#include <QString>

namespace cutline {

// Build facts about the FFmpeg libraries CUTLINE is running on.
struct FfmpegInfo {
    QString license;        // avcodec_license(), e.g. "LGPL version 2.1 or later"
    QString configuration;  // avcodec_configuration()
    QString avcodecVersion; // "major.minor.micro"
    bool gpl = false;       // GPL build: never allowed in official downloads
    bool nonfree = false;   // nonfree build: never redistributable
};

FfmpegInfo queryFfmpegInfo();

} // namespace cutline
