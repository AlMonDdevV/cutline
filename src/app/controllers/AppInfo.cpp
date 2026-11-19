#include "app/controllers/AppInfo.h"

#include "core/Version.h"
#include "media/FfmpegInfo.h"

namespace cutline {

AppInfo::AppInfo(QStringList startupWarnings, QObject* parent) : QObject(parent), m_warnings(std::move(startupWarnings)) {
    const FfmpegInfo ffmpeg = queryFfmpegInfo();
    m_ffmpegVersion = ffmpeg.avcodecVersion;
    m_ffmpegLicense = ffmpeg.license;
}

QString AppInfo::version() const { return QString::fromLatin1(kVersion); }
QString AppInfo::qtVersion() const { return QString::fromLatin1(qVersion()); }

} // namespace cutline
