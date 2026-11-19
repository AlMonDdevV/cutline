#pragma once

#include <QObject>
#include <QStringList>

namespace cutline {

// Facts for the About box and the status bar (the `AppInfo` singleton).
class AppInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    Q_PROPERTY(QString ffmpegVersion READ ffmpegVersion CONSTANT)
    Q_PROPERTY(QString ffmpegLicense READ ffmpegLicense CONSTANT)
    Q_PROPERTY(QString licenseName READ licenseName CONSTANT)
    Q_PROPERTY(QStringList startupWarnings READ startupWarnings CONSTANT)

public:
    explicit AppInfo(QStringList startupWarnings = {}, QObject* parent = nullptr);

    QString version() const;
    QString qtVersion() const;
    QString ffmpegVersion() const { return m_ffmpegVersion; }
    QString ffmpegLicense() const { return m_ffmpegLicense; }
    QString licenseName() const { return QStringLiteral("MIT License"); }
    QStringList startupWarnings() const { return m_warnings; }

private:
    QString m_ffmpegVersion, m_ffmpegLicense;
    QStringList m_warnings;
};

} // namespace cutline
