#include "core/Time.h"

#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

class TestTime : public QObject {
    Q_OBJECT

private slots:
    void exactFrameDurations_data() {
        QTest::addColumn<int>("num");
        QTest::addColumn<int>("den");
        QTest::addColumn<qint64>("expected");
        QTest::newRow("24") << 24 << 1 << qint64(29'400'000);
        QTest::newRow("25") << 25 << 1 << qint64(28'224'000);
        QTest::newRow("30") << 30 << 1 << qint64(23'520'000);
        QTest::newRow("60") << 60 << 1 << qint64(11'760'000);
        QTest::newRow("23.976") << 24000 << 1001 << qint64(29'429'400);
        QTest::newRow("29.97") << 30000 << 1001 << qint64(23'543'520);
        QTest::newRow("59.94") << 60000 << 1001 << qint64(11'771'760);
    }
    void exactFrameDurations() {
        QFETCH(int, num);
        QFETCH(int, den);
        QFETCH(qint64, expected);
        const FrameRate rate{num, den};
        QCOMPARE(framesToTicks(1, rate), expected);
        QCOMPARE(framesToTicks(1000, rate), expected * 1000);
    }

    void ticksToFramesFloors() {
        const FrameRate r{30000, 1001};
        QCOMPARE(ticksToFrames(framesToTicks(17982, r), r), qint64(17982));
        QCOMPARE(ticksToFrames(framesToTicks(17982, r) - 1, r), qint64(17981));
    }

    void noDriftOverTenHours() {
        const FrameRate r{24000, 1001};
        const qint64 f = qint64(24) * 3600 * 10;
        QCOMPARE(ticksToFrames(framesToTicks(f, r), r), f);
    }

    void nonExactRateRounds() {
        const FrameRate r{2398, 100};
        QVERIFY(qAbs(framesToTicks(2398, r) - 100 * kTicksPerSecond) <= 1);
    }

    void samples() {
        QCOMPARE(samplesToTicks(48000, 48000), kTicksPerSecond);
        QCOMPARE(samplesToTicks(1, 44100), Ticks(16000));
    }

    void dropFrameFormat_data() {
        QTest::addColumn<qint64>("frame");
        QTest::addColumn<QString>("text");
        QTest::newRow("0") << qint64(0) << u"00:00:00;00"_s;
        QTest::newRow("1799") << qint64(1799) << u"00:00:59;29"_s;
        QTest::newRow("1800") << qint64(1800) << u"00:01:00;02"_s;
        QTest::newRow("17982") << qint64(17982) << u"00:10:00;00"_s;
        QTest::newRow("107892") << qint64(107892) << u"01:00:00;00"_s;
    }
    void dropFrameFormat() {
        QFETCH(qint64, frame);
        QFETCH(QString, text);
        QCOMPARE(formatTimecode(frame, {30000, 1001}, TimecodeMode::Drop), text);
    }

    void nonDropFormat() {
        QCOMPARE(formatTimecode(1800, {30000, 1001}, TimecodeMode::NonDrop), u"00:01:00:00"_s);
        QCOMPARE(formatTimecode(25, {25, 1}, TimecodeMode::NonDrop, 1), u"01:00:01:00"_s);
    }

    void dropRequestedOnNonDropRateFallsBack() {
        QCOMPARE(formatTimecode(24, {24, 1}, TimecodeMode::Drop), u"00:00:01:00"_s);
    }

    void parseRoundTrip() {
        const FrameRate r{30000, 1001};
        for (qint64 f : {qint64(0), qint64(1), qint64(1799), qint64(1800), qint64(17982), qint64(107891)}) {
            const auto parsed = parseTimecode(formatTimecode(f, r, TimecodeMode::Drop), r, TimecodeMode::Drop);
            QVERIFY(parsed.has_value());
            QCOMPARE(*parsed, f);
        }
        const auto withHour = parseTimecode(u"01:00:01:00", {25, 1}, TimecodeMode::NonDrop, 1);
        QVERIFY(withHour.has_value());
        QCOMPARE(*withHour, qint64(25));
    }

    void parseRejectsDroppedLabels() {
        QVERIFY(!parseTimecode(u"00:01:00;00", {30000, 1001}, TimecodeMode::Drop));
        QVERIFY(!parseTimecode(u"00:00:00:30", {30, 1}, TimecodeMode::NonDrop));
        QVERIFY(!parseTimecode(u"garbage", {30, 1}, TimecodeMode::NonDrop));
    }

    void significantIndex() {
        QCOMPARE(timecodeSignificantIndex(u"00:01:14:21"), qsizetype(4));
        QCOMPARE(timecodeSignificantIndex(u"00:00:00:00"), qsizetype(10));
        QCOMPARE(timecodeSignificantIndex(u"12:00:00:00"), qsizetype(0));
    }
};

QTEST_GUILESS_MAIN(TestTime)
#include "tst_time.moc"
