#include "core/Commands.h"
#include "core/Project.h"
#include "core/ProjectIO.h"

#include <QtTest>

#include <functional>

using namespace cutline;
using namespace Qt::StringLiterals;

using CommandFactory = std::function<std::unique_ptr<Command>(const Project&)>;
Q_DECLARE_METATYPE(CommandFactory)

namespace {

QStringList codes(const Project& p) {
    QStringList out;
    const Sequence& s = p.sequences.front();
    for (const Track& t : s.tracks)
        out << trackCode(s, t.id);
    return out;
}

QString seqId(const Project& p) { return p.sequences.front().id; }
QString trackId(const Project& p, int index) { return p.sequences.front().tracks.at(size_t(index)).id; }

SequenceSettings settingsWith(const Project& p, const std::function<void(SequenceSettings&)>& change) {
    SequenceSettings s = settingsOf(p.sequences.front());
    change(s);
    return s;
}

} // namespace

class TestCommands : public QObject {
    Q_OBJECT

private slots:
    void defaultProjectShape() {
        const Project p = makeDefaultProject();
        QCOMPARE(p.name, u"Untitled"_s);
        QCOMPARE(p.sequences.size(), size_t(1));
        const Sequence& s = p.sequences.front();
        QCOMPARE(s.name, u"Sequence 1"_s);
        QVERIFY(s.format == SequenceFormat{});
        QCOMPARE(s.format.width, 1920);
        QCOMPARE(s.format.frameRate, (FrameRate{30, 1}));
        QCOMPARE(codes(p), (QStringList{u"V1"_s, u"V2"_s, u"A1"_s, u"A2"_s}));
        QCOMPARE(s.tracks.front().name, u"Video 1"_s);
        QCOMPARE(s.tracks.back().name, u"Audio 2"_s);
        QCOMPARE(p.defaultSequenceId, s.id);
    }

    void addTrackCodes() {
        Project p = makeDefaultProject();
        QVERIFY(makeAddTrack(seqId(p), TrackKind::Video, u"v3"_s)->apply(p).ok);
        QCOMPARE(codes(p), (QStringList{u"V1"_s, u"V2"_s, u"V3"_s, u"A1"_s, u"A2"_s}));
        QCOMPARE(p.sequences.front().tracks.at(2).id, u"v3"_s);
        QCOMPARE(p.sequences.front().tracks.at(2).name, u"Video 3"_s);
        QVERIFY(makeAddTrack(seqId(p), TrackKind::Audio, u"a3"_s)->apply(p).ok);
        QCOMPARE(codes(p).last(), u"A3"_s);
        QCOMPARE(p.sequences.front().tracks.back().id, u"a3"_s);
    }

    void applyRevertRestoresState_data() {
        QTest::addColumn<CommandFactory>("factory");
        QTest::newRow("rename project") << CommandFactory([](const Project&) { return makeRenameProject(u"Interview"_s); });
        QTest::newRow("sequence settings") << CommandFactory([](const Project& p) {
            return makeSetSequenceSettings(seqId(p), settingsWith(p, [](SequenceSettings& s) {
                                               s.format.width = 1080;
                                               s.format.height = 1920;
                                               s.timecodeStartHour = 1;
                                           }));
        });
        QTest::newRow("add video") << CommandFactory([](const Project& p) { return makeAddTrack(seqId(p), TrackKind::Video); });
        QTest::newRow("add audio") << CommandFactory([](const Project& p) { return makeAddTrack(seqId(p), TrackKind::Audio); });
        QTest::newRow("remove V2") << CommandFactory([](const Project& p) { return makeRemoveTrack(seqId(p), trackId(p, 1)); });
        QTest::newRow("remove A1") << CommandFactory([](const Project& p) { return makeRemoveTrack(seqId(p), trackId(p, 2)); });
        QTest::newRow("track name") << CommandFactory([](const Project& p) { return makeSetTrackName(seqId(p), trackId(p, 0), u"B-roll"_s); });
        QTest::newRow("track locked") << CommandFactory([](const Project& p) { return makeSetTrackLocked(seqId(p), trackId(p, 0), true); });
        QTest::newRow("track enabled") << CommandFactory([](const Project& p) { return makeSetTrackEnabled(seqId(p), trackId(p, 3), false); });
        QTest::newRow("track solo") << CommandFactory([](const Project& p) { return makeSetTrackSolo(seqId(p), trackId(p, 3), true); });
    }
    void applyRevertRestoresState() {
        QFETCH(CommandFactory, factory);
        Project p = makeDefaultProject();
        const QByteArray before = io::serializeProject(p);
        auto command = factory(p);
        const Status status = command->apply(p);
        QVERIFY2(status.ok, qPrintable(status.message));
        QVERIFY(io::serializeProject(p) != before);
        command->revert(p);
        QCOMPARE(io::serializeProject(p), before);
    }

    void validationMessages_data() {
        QTest::addColumn<CommandFactory>("factory");
        QTest::addColumn<QString>("message");
        const QString longName(121, u'x');
        QTest::newRow("empty project name") << CommandFactory([](const Project&) { return makeRenameProject(u"  "_s); })
                                            << u"Project names can't be empty."_s;
        QTest::newRow("long project name") << CommandFactory([longName](const Project&) { return makeRenameProject(longName); })
                                           << u"Project names can be at most 120 characters."_s;
        QTest::newRow("empty track name") << CommandFactory([](const Project& p) { return makeSetTrackName(seqId(p), trackId(p, 0), u""_s); })
                                          << u"Track names can't be empty."_s;
        QTest::newRow("long track name") << CommandFactory([](const Project& p) { return makeSetTrackName(seqId(p), trackId(p, 0), QString(65, u'y')); })
                                         << u"Track names can be at most 64 characters."_s;
        QTest::newRow("odd width") << CommandFactory([](const Project& p) {
            return makeSetSequenceSettings(seqId(p), settingsWith(p, [](SequenceSettings& s) { s.format.width = 1921; }));
        }) << u"Width and height must be even numbers between 16 and 8192."_s;
        QTest::newRow("huge height") << CommandFactory([](const Project& p) {
            return makeSetSequenceSettings(seqId(p), settingsWith(p, [](SequenceSettings& s) { s.format.height = 8194; }));
        }) << u"Width and height must be even numbers between 16 and 8192."_s;
        QTest::newRow("zero fps") << CommandFactory([](const Project& p) {
            return makeSetSequenceSettings(seqId(p), settingsWith(p, [](SequenceSettings& s) { s.format.frameRate = {0, 1}; }));
        }) << u"Frame rate must be between 1 and 240 fps."_s;
        QTest::newRow("fps 300") << CommandFactory([](const Project& p) {
            return makeSetSequenceSettings(seqId(p), settingsWith(p, [](SequenceSettings& s) { s.format.frameRate = {300, 1}; }));
        }) << u"Frame rate must be between 1 and 240 fps."_s;
        QTest::newRow("sample rate") << CommandFactory([](const Project& p) {
            return makeSetSequenceSettings(seqId(p), settingsWith(p, [](SequenceSettings& s) { s.format.sampleRate = 22050; }));
        }) << u"Sample rate must be 44.1, 48 or 96 kHz."_s;
        QTest::newRow("channels") << CommandFactory([](const Project& p) {
            return makeSetSequenceSettings(seqId(p), settingsWith(p, [](SequenceSettings& s) { s.format.channels = 6; }));
        }) << u"Sequences are stereo in this version."_s;
        QTest::newRow("timecode start") << CommandFactory([](const Project& p) {
            return makeSetSequenceSettings(seqId(p), settingsWith(p, [](SequenceSettings& s) { s.timecodeStartHour = 10; }));
        }) << u"Timecode can start at 00:00:00:00 or 01:00:00:00."_s;
        QTest::newRow("last video") << CommandFactory([](const Project& p) {
            // V2 is removed first by the test setup below; here V1 is the last video track.
            return makeRemoveTrack(seqId(p), trackId(p, 0));
        }) << u"A sequence needs at least one video track."_s;
        QTest::newRow("solo on video") << CommandFactory([](const Project& p) { return makeSetTrackSolo(seqId(p), trackId(p, 0), true); })
                                       << u"Solo is only available on audio tracks."_s;
        QTest::newRow("unknown sequence") << CommandFactory([](const Project&) { return makeAddTrack(u"nope"_s, TrackKind::Video); })
                                          << u"That sequence no longer exists."_s;
        QTest::newRow("unknown track") << CommandFactory([](const Project& p) { return makeSetTrackLocked(seqId(p), u"nope"_s, true); })
                                       << u"That track no longer exists."_s;
    }
    void validationMessages() {
        QFETCH(CommandFactory, factory);
        QFETCH(QString, message);
        Project p = makeDefaultProject();
        if (QByteArray(QTest::currentDataTag()) == "last video")
            QVERIFY(makeRemoveTrack(seqId(p), trackId(p, 1))->apply(p).ok);
        const QByteArray before = io::serializeProject(p);
        const Status status = factory(p)->apply(p);
        QVERIFY(!status.ok);
        QCOMPARE(status.message, message);
        QCOMPARE(io::serializeProject(p), before);
    }

    void lastAudioTrackProtected() {
        Project p = makeDefaultProject();
        QVERIFY(makeRemoveTrack(seqId(p), trackId(p, 3))->apply(p).ok);
        const Status status = makeRemoveTrack(seqId(p), trackId(p, 2))->apply(p);
        QCOMPARE(status.message, u"A sequence needs at least one audio track."_s);
    }

    void titles() {
        Project p = makeDefaultProject();
        auto apply = [&p](std::unique_ptr<Command> c) {
            const Status s = c->apply(p);
            return s.ok ? c->title() : u"FAILED: "_s + s.message;
        };
        QCOMPARE(apply(makeRenameProject(u"X"_s)), u"Rename project"_s);
        QCOMPARE(apply(makeSetSequenceSettings(seqId(p), settingsOf(p.sequences.front()))), u"Change sequence settings"_s);
        QCOMPARE(apply(makeAddTrack(seqId(p), TrackKind::Video)), u"Add video track"_s);
        QCOMPARE(apply(makeAddTrack(seqId(p), TrackKind::Audio)), u"Add audio track"_s);
        QCOMPARE(apply(makeSetTrackName(seqId(p), trackId(p, 0), u"Main"_s)), u"Rename track V1"_s);
        QCOMPARE(apply(makeSetTrackLocked(seqId(p), trackId(p, 1), true)), u"Lock track V2"_s);
        QCOMPARE(apply(makeSetTrackLocked(seqId(p), trackId(p, 1), false)), u"Unlock track V2"_s);
        QCOMPARE(apply(makeSetTrackEnabled(seqId(p), trackId(p, 0), false)), u"Hide track V1"_s);
        QCOMPARE(apply(makeSetTrackEnabled(seqId(p), trackId(p, 0), true)), u"Show track V1"_s);
        QCOMPARE(apply(makeSetTrackEnabled(seqId(p), trackId(p, 3), false)), u"Mute track A1"_s);
        QCOMPARE(apply(makeSetTrackEnabled(seqId(p), trackId(p, 3), true)), u"Unmute track A1"_s);
        QCOMPARE(apply(makeSetTrackSolo(seqId(p), trackId(p, 4), true)), u"Solo track A2"_s);
        QCOMPARE(apply(makeSetTrackSolo(seqId(p), trackId(p, 4), false)), u"Unsolo track A2"_s);
        QCOMPARE(apply(makeRemoveTrack(seqId(p), trackId(p, 2))), u"Remove track V3"_s);
    }
};

QTEST_GUILESS_MAIN(TestCommands)
#include "tst_commands.moc"
