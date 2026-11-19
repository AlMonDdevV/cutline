#include "app/icons/IcoWriter.h"

#include <QBuffer>

namespace cutline {
namespace {

void putU8(QByteArray& out, int v) { out.append(char(quint8(v))); }
void putU16(QByteArray& out, quint16 v) {
    out.append(char(v & 0xFF));
    out.append(char(v >> 8));
}
void putU32(QByteArray& out, quint32 v) {
    putU16(out, quint16(v & 0xFFFF));
    putU16(out, quint16(v >> 16));
}

} // namespace

QByteArray writeIco(const QList<QImage>& images) {
    QList<QByteArray> pngs;
    for (const QImage& image : images) {
        QByteArray png;
        QBuffer buffer(&png);
        buffer.open(QIODevice::WriteOnly);
        image.convertToFormat(QImage::Format_ARGB32).save(&buffer, "PNG");
        pngs << png;
    }

    QByteArray out;
    putU16(out, 0);                      // reserved
    putU16(out, 1);                      // type: icon
    putU16(out, quint16(images.size())); // count

    quint32 offset = quint32(6 + 16 * images.size());
    for (qsizetype i = 0; i < images.size(); ++i) {
        const QSize s = images[i].size();
        putU8(out, s.width() >= 256 ? 0 : s.width());
        putU8(out, s.height() >= 256 ? 0 : s.height());
        putU8(out, 0);  // palette colours
        putU8(out, 0);  // reserved
        putU16(out, 1); // colour planes
        putU16(out, 32);
        putU32(out, quint32(pngs[i].size()));
        putU32(out, offset);
        offset += quint32(pngs[i].size());
    }
    for (const QByteArray& png : pngs)
        out.append(png);
    return out;
}

} // namespace cutline
