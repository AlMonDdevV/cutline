#pragma once

#include <QColor>
#include <QImage>
#include <QQuickImageProvider>
#include <QSize>
#include <QString>

namespace cutline {

// Serves CUTLINE's stroke icons to QML as "image://icons/<name>?color=<rrggbb>", recoloured on the fly.
class IconProvider : public QQuickImageProvider {
public:
    IconProvider();

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

    // Renders :/cutline/icons/<name>.svg with `currentColor` replaced by `color`.
    // Unknown names give a transparent image and one warning.
    static QImage render(const QString& name, const QColor& color, const QSize& pixelSize);
};

} // namespace cutline
