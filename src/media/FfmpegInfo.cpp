#include "media/FfmpegInfo.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/version.h>
}

namespace cutline {

FfmpegInfo queryFfmpegInfo() {
    FfmpegInfo info;
    info.license = QString::fromUtf8(avcodec_license());
    info.configuration = QString::fromUtf8(avcodec_configuration());

    const unsigned version = avcodec_version();
    info.avcodecVersion = QStringLiteral("%1.%2.%3")
                              .arg(AV_VERSION_MAJOR(version))
                              .arg(AV_VERSION_MINOR(version))
                              .arg(AV_VERSION_MICRO(version));

    const bool licenseIsGpl = info.license.contains(QLatin1String("GPL")) && !info.license.contains(QLatin1String("LGPL"));
    info.gpl = licenseIsGpl || info.configuration.contains(QLatin1String("--enable-gpl"));
    info.nonfree = info.license.contains(QLatin1String("nonfree"), Qt::CaseInsensitive) ||
                   info.configuration.contains(QLatin1String("--enable-nonfree"));
    return info;
}

} // namespace cutline
