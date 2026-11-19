#include "app/icons/IconProvider.h"

#include <QFile>
#include <QPainter>
#include <QSet>
#include <QSvgRenderer>
#include <QUrlQuery>

namespace cutline {

IconProvider::IconProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

QImage IconProvider::render(const QString& name, const QColor& color, const QSize& pixelSize) {
    QImage image(pixelSize.isValid() ? pixelSize : QSize(24, 24), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QFile file(QStringLiteral(":/cutline/icons/%1.svg").arg(name));
    if (!file.open(QIODevice::ReadOnly)) {
        static QSet<QString> warned;
        if (!warned.contains(name)) {
            warned.insert(name);
            qWarning("Unknown icon \"%s\"", qPrintable(name));
        }
        return image;
    }

    QByteArray svg = file.readAll();
    svg.replace("currentColor", color.name(QColor::HexRgb).toLatin1());
    QSvgRenderer renderer(svg);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer.render(&painter, QRectF(QPointF(0, 0), QSizeF(image.size())));
    painter.end();

    if (color.alpha() < 255) {
        QPainter fade(&image);
        fade.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        fade.fillRect(image.rect(), QColor(0, 0, 0, color.alpha()));
    }
    return image;
}

QImage IconProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize) {
    const qsizetype q = id.indexOf(QLatin1Char('?'));
    const QString name = q < 0 ? id : id.left(q);
    QColor color(0xE6, 0xE6, 0xE6);
    if (q >= 0) {
        const QString hex = QUrlQuery(id.mid(q + 1)).queryItemValue(QStringLiteral("color"));
        const QColor parsed = QColor::fromString(QLatin1Char('#') + hex);
        if (parsed.isValid())
            color = parsed;
    }
    const QSize pixelSize = requestedSize.isValid() && !requestedSize.isEmpty() ? requestedSize : QSize(24, 24);
    const QImage image = render(name, color, pixelSize);
    if (size)
        *size = image.size();
    return image;
}

} // namespace cutline
