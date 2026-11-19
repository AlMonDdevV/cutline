#pragma once

#include <QByteArray>
#include <QImage>
#include <QList>

namespace cutline {

// A Windows .ico holding each image as a PNG-compressed entry (Vista+ format). Sizes up to 256 px.
QByteArray writeIco(const QList<QImage>& images);

} // namespace cutline
