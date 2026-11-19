#include "media/FfmpegInfo.h"

#include <QtTest>

using namespace cutline;

class TestFfmpegInfo : public QObject {
    Q_OBJECT

private slots:
    void reportsVersionAndLicense() {
        const FfmpegInfo info = queryFfmpegInfo();
        QVERIFY(!info.license.isEmpty());
        QCOMPARE(info.avcodecVersion.count(QLatin1Char('.')), qsizetype(2));
    }

    void officialBuildIsLgpl() {
#ifdef CUTLINE_OFFICIAL_BUILD
        const FfmpegInfo info = queryFfmpegInfo();
        QVERIFY2(!info.gpl, qPrintable(info.license));
        QVERIFY(!info.nonfree);
        QVERIFY(info.license.startsWith(u"LGPL"));
#else
        QSKIP("Not an official build: the distro FFmpeg licence is reported, not enforced");
#endif
    }
};

QTEST_GUILESS_MAIN(TestFfmpegInfo)
#include "tst_ffmpeginfo.moc"
