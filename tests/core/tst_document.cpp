#include "core/Commands.h"
#include "core/Document.h"
#include "core/ProjectIO.h"

#include <QRandomGenerator>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

QString seqId(const Document& d) { return d.project().sequences.front().id; }

} // namespace

class TestDocument : public QObject {
    Q_OBJECT

private slots:
    void executeUndoRedo() {
        auto doc = Document::createNew();
        QVERIFY(!doc->canUndo());
        QVERIFY(doc->execute(makeAddTrack(seqId(*doc), TrackKind::Video)).ok);
        QVERIFY(doc->canUndo());
        QCOMPARE(doc->undoTitle(), u"Add video track"_s);
        QCOMPARE(doc->project().sequences.front().tracks.size(), size_t(5));
        doc->undo();
        QVERIFY(doc->canRedo());
        QCOMPARE(doc->redoTitle(), u"Add video track"_s);
        QCOMPARE(doc->project().sequences.front().tracks.size(), size_t(4));
        doc->redo();
        QCOMPARE(doc->project().sequences.front().tracks.size(), size_t(5));
    }

    void failedExecuteChangesNothing() {
        auto doc = Document::createNew();
        const quint64 revision = doc->revision();
        const Status status = doc->execute(makeRenameProject(u""_s));
        QVERIFY(!status.ok);
        QCOMPARE(status.message, u"Project names can't be empty."_s);
        QCOMPARE(doc->revision(), revision);
        QCOMPARE(doc->historyCount(), 0);
        QVERIFY(!doc->isDirty());
    }

    void revisionCounts() {
        auto doc = Document::createNew();
        const quint64 r0 = doc->revision();
        doc->execute(makeRenameProject(u"A"_s));
        QCOMPARE(doc->revision(), r0 + 1);
        doc->undo();
        QCOMPARE(doc->revision(), r0 + 2);
        doc->redo();
        QCOMPARE(doc->revision(), r0 + 3);
    }

    void dirtyTracksCleanIndex() {
        QTemporaryDir dir;
        auto doc = Document::createNew();
        QVERIFY(!doc->isDirty());
        doc->execute(makeRenameProject(u"A"_s));
        QVERIFY(doc->isDirty());
        QVERIFY(doc->saveAs(dir.filePath(u"a.cutline"_s)).ok);
        QVERIFY(!doc->isDirty());
        doc->execute(makeRenameProject(u"B"_s));
        QVERIFY(doc->isDirty());
        doc->undo();
        QVERIFY(!doc->isDirty());
        doc->undo();
        QVERIFY(doc->isDirty());
        doc->redo();
        QVERIFY(!doc->isDirty());
    }

    void executeAfterUndoDropsRedo() {
        auto doc = Document::createNew();
        doc->execute(makeRenameProject(u"A"_s));
        doc->execute(makeRenameProject(u"B"_s));
        doc->undo();
        QVERIFY(doc->canRedo());
        doc->execute(makeRenameProject(u"C"_s));
        QVERIFY(!doc->canRedo());
        QCOMPARE(doc->historyCount(), 2);
        QCOMPARE(doc->project().name, u"C"_s);
    }

    void jumpTo() {
        auto doc = Document::createNew();
        doc->execute(makeRenameProject(u"A"_s));
        const QByteArray afterFirst = io::serializeProject(doc->project());
        doc->execute(makeRenameProject(u"B"_s));
        doc->execute(makeAddTrack(seqId(*doc), TrackKind::Audio));
        doc->jumpTo(1);
        QCOMPARE(doc->historyIndex(), 1);
        QCOMPARE(io::serializeProject(doc->project()), afterFirst);
        doc->jumpTo(3);
        QCOMPARE(doc->historyIndex(), 3);
        QCOMPARE(doc->historyTitle(2), u"Add audio track"_s);
    }

    void randomSequenceUndoesToInitial() {
        auto doc = Document::createNew();
        QRandomGenerator rng(1234);
        const QByteArray initial = io::serializeProject(doc->project());
        const QStringList names = {u""_s, u"  "_s, u"Main"_s, QString(70, u'n'), u"B-roll"_s, u"Zoë 東京"_s};

        auto randomTrack = [&]() -> QString {
            const auto& tracks = doc->project().sequences.front().tracks;
            if (rng.bounded(10) == 0)
                return u"bogus"_s;
            return tracks.at(size_t(rng.bounded(int(tracks.size())))).id;
        };

        for (int i = 0; i < 500; ++i) {
            const QString seq = rng.bounded(20) == 0 ? u"bogus"_s : seqId(*doc);
            switch (rng.bounded(10)) {
            case 0: doc->execute(makeRenameProject(names.at(rng.bounded(int(names.size()))))); break;
            case 1: {
                SequenceSettings s = settingsOf(doc->project().sequences.front());
                s.format.width = 16 * rng.bounded(1, 300);
                s.format.height = 2 * rng.bounded(1, 4200);
                s.format.frameRate = {int(rng.bounded(0, 300)), 1};
                s.timecodeStartHour = rng.bounded(3);
                doc->execute(makeSetSequenceSettings(seq, s));
                break;
            }
            case 2: doc->execute(makeAddTrack(seq, rng.bounded(2) ? TrackKind::Video : TrackKind::Audio)); break;
            case 3: doc->execute(makeRemoveTrack(seq, randomTrack())); break;
            case 4: doc->execute(makeSetTrackName(seq, randomTrack(), names.at(rng.bounded(int(names.size()))))); break;
            case 5: doc->execute(makeSetTrackLocked(seq, randomTrack(), rng.bounded(2))); break;
            case 6: doc->execute(makeSetTrackEnabled(seq, randomTrack(), rng.bounded(2))); break;
            case 7: doc->execute(makeSetTrackSolo(seq, randomTrack(), rng.bounded(2))); break;
            case 8: doc->undo(); break;
            case 9: doc->redo(); break;
            }
        }

        const int endIndex = doc->historyIndex();
        const QByteArray final = io::serializeProject(doc->project());
        doc->jumpTo(0);
        QCOMPARE(io::serializeProject(doc->project()), initial);
        doc->jumpTo(endIndex);
        QCOMPARE(io::serializeProject(doc->project()), final);
    }

    void saveWithoutPathFails() {
        auto doc = Document::createNew();
        const Status status = doc->save();
        QVERIFY(!status.ok);
        QCOMPARE(status.message, u"This project hasn't been saved yet."_s);
    }

    void openRoundTrip() {
        QTemporaryDir dir;
        const QString path = dir.filePath(u"p.cutline"_s);
        auto doc = Document::createNew();
        doc->execute(makeRenameProject(u"Interview"_s));
        QVERIFY(doc->saveAs(path).ok);
        auto opened = Document::open(path);
        QVERIFY(opened.ok());
        QCOMPARE(opened.value()->project().name, u"Interview"_s);
        QCOMPARE(opened.value()->filePath(), path);
        QVERIFY(!opened.value()->isDirty());
        QVERIFY(!opened.value()->canUndo());
    }

    void signalsFire() {
        auto doc = Document::createNew();
        QSignalSpy changed(doc.get(), &Document::changed);
        QSignalSpy history(doc.get(), &Document::historyChanged);
        QSignalSpy dirty(doc.get(), &Document::dirtyChanged);
        doc->execute(makeAddTrack(seqId(*doc), TrackKind::Video));
        QCOMPARE(changed.count(), 1);
        QCOMPARE(changed.first().first().value<ChangeHint>().scope, ChangeHint::Scope::Tracks);
        QCOMPARE(history.count(), 1);
        QCOMPARE(dirty.count(), 1);
        QCOMPARE(dirty.first().first().toBool(), true);
        doc->undo();
        QCOMPARE(changed.count(), 2);
        QCOMPARE(dirty.count(), 2);
    }
};

QTEST_GUILESS_MAIN(TestDocument)
#include "tst_document.moc"
