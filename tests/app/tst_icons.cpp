#include "app/icons/IcoWriter.h"
#include "app/icons/IconProvider.h"

#include <QImage>
#include <QtTest>

#include <functional>

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

int countPixels(const QImage& img, const std::function<bool(QRgb)>& pred) {
    int n = 0;
    const QImage argb = img.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < argb.height(); ++y)
        for (int x = 0; x < argb.width(); ++x)
            if (pred(argb.pixel(x, y)))
                ++n;
    return n;
}

// Every pixel with alpha >= 64 has the given hue (±10°); anti-aliased edges are skipped.
bool allOpaquePixelsAreHue(const QImage& img, int hue) {
    const QImage argb = img.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < argb.height(); ++y)
        for (int x = 0; x < argb.width(); ++x) {
            const QRgb p = argb.pixel(x, y);
            if (qAlpha(p) < 64)
                continue;
            const int h = QColor::fromRgb(p).hue();
            const int diff = qAbs(h - hue) % 360;
            if (std::min(diff, 360 - diff) > 10)
                return false;
        }
    return true;
}

quint16 readU16(const QByteArray& b, int at) { return quint16(quint8(b[at]) | (quint8(b[at + 1]) << 8)); }
quint32 readU32(const QByteArray& b, int at) { return quint32(readU16(b, at)) | (quint32(readU16(b, at + 2)) << 16); }

} // namespace

class TestIcons : public QObject {
    Q_OBJECT

private slots:
    void everyIconRenders_data() {
        QTest::addColumn<QString>("name");
        for (const auto* n : {"lock", "unlock", "eye", "eye-off", "speaker", "speaker-off", "plus", "trash", "undo",
                              "redo", "settings", "keyboard", "search", "close", "chevron-down", "chevron-right",
                              "panel-viewer", "panel-inspector", "panel-history", "panel-timeline", "record", "reset",
                              "warning", "info"})
            QTest::newRow(n) << QString::fromLatin1(n);
    }
    void everyIconRenders() {
        QFETCH(QString, name);
        const QImage img = IconProvider::render(name, QColor(u"#ff0000"_s), {48, 48});
        QCOMPARE(img.size(), QSize(48, 48));
        QVERIFY(countPixels(img, [](QRgb p) { return qAlpha(p) > 128; }) > 20);
        QVERIFY(allOpaquePixelsAreHue(img, 0));
    }

    void unknownIconIsTransparent() {
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"Unknown icon \"nope\""_s));
        const QImage img = IconProvider::render(u"nope"_s, Qt::white, {16, 16});
        QCOMPARE(img.size(), QSize(16, 16));
        QCOMPARE(countPixels(img, [](QRgb p) { return qAlpha(p) > 0; }), 0);
    }

    void requestImageParsesColor() {
        IconProvider provider;
        QSize size;
        const QImage img = provider.requestImage(u"plus?color=00ff00"_s, &size, {24, 24});
        QCOMPARE(size, QSize(24, 24));
        QVERIFY(countPixels(img, [](QRgb p) { return qAlpha(p) > 128; }) > 10);
        QVERIFY(allOpaquePixelsAreHue(img, 120));
    }

    void icoStructure() {
        QList<QImage> images;
        for (int px : {16, 32, 256}) {
            QImage img(px, px, QImage::Format_ARGB32);
            img.fill(Qt::red);
            images << img;
        }
        const QByteArray ico = writeIco(images);
        QCOMPARE(readU16(ico, 0), quint16(0));
        QCOMPARE(readU16(ico, 2), quint16(1));
        QCOMPARE(readU16(ico, 4), quint16(3));
        QCOMPARE(quint8(ico[6]), quint8(16));
        QCOMPARE(quint8(ico[6 + 2 * 16]), quint8(0)); // 256 px is stored as 0
        const quint32 firstOffset = readU32(ico, 6 + 12);
        QCOMPARE(ico.mid(int(firstOffset), 8), QByteArray("\x89PNG\r\n\x1a\n", 8));
        QCOMPARE(readU32(ico, 6 + 8), quint32(readU32(ico, 6 + 16 + 12) - firstOffset)); // size == gap to next
    }
};

QTEST_MAIN(TestIcons)
#include "tst_icons.moc"
