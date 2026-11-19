// iconbake <in.svg> <out.ico> <out-png-dir>
// Renders the CUTLINE mark to PNGs at the standard icon sizes and packs them into a Windows .ico.
#include "app/icons/IcoWriter.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>

#include <cstdio>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    const QStringList args = QCoreApplication::arguments();
    if (args.size() != 4) {
        std::fprintf(stderr, "usage: iconbake <in.svg> <out.ico> <out-png-dir>\n");
        return 2;
    }

    QSvgRenderer renderer(args[1]);
    if (!renderer.isValid()) {
        std::fprintf(stderr, "iconbake: cannot read %s\n", qPrintable(args[1]));
        return 1;
    }

    QDir().mkpath(args[3]);
    QList<QImage> images;
    for (int px : {16, 20, 24, 32, 40, 48, 64, 128, 256}) {
        QImage image(px, px, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.setRenderHint(QPainter::Antialiasing);
        renderer.render(&painter);
        painter.end();
        if (!image.save(QDir(args[3]).filePath(QStringLiteral("cutline-%1.png").arg(px)))) {
            std::fprintf(stderr, "iconbake: cannot write PNG %d\n", px);
            return 1;
        }
        images << image;
    }

    QFile ico(args[2]);
    if (!ico.open(QIODevice::WriteOnly) || ico.write(cutline::writeIco(images)) < 0) {
        std::fprintf(stderr, "iconbake: cannot write %s\n", qPrintable(args[2]));
        return 1;
    }
    return 0;
}
