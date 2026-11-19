#include "core/Commands.h"
#include "core/Project.h"
#include "core/ProjectIO.h"
#include "core/Version.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

Project modifiedProject() {
    Project p = makeDefaultProject();
    const QString seq = p.sequences.front().id;
    makeRenameProject(u"Summer Edit"_s)->apply(p);
    makeAddTrack(seq, TrackKind::Audio)->apply(p);
    makeSetTrackLocked(seq, p.sequences.front().tracks.at(0).id, true)->apply(p);
    makeSetTrackSolo(seq, p.sequences.front().tracks.at(2).id, true)->apply(p);
    SequenceSettings s = settingsOf(p.sequences.front());
    s.format = {1080, 1920, {30000, 1001}, 48000, 2};
    s.timecodeStartHour = 1;
    makeSetSequenceSettings(seq, s)->apply(p);
    return p;
}

QJsonObject json(const Project& p) { return QJsonDocument::fromJson(io::serializeProject(p)).object(); }

QByteArray withFormat(const std::function<void(QJsonObject& format)>& change) {
    QJsonObject root = json(makeDefaultProject());
    QJsonArray sequences = root.value(u"sequences").toArray();
    QJsonObject seq = sequences.at(0).toObject();
    QJsonObject format = seq.value(u"format").toObject();
    change(format);
    seq.insert(u"format"_s, format);
    sequences.replace(0, seq);
    root.insert(u"sequences"_s, sequences);
    return QJsonDocument(root).toJson();
}

} // namespace

class TestProjectIO : public QObject {
    Q_OBJECT

private slots:
    void roundTripIsByteIdentical() {
        const QByteArray first = io::serializeProject(modifiedProject());
        const auto loaded = io::deserializeProject(first);
        QVERIFY2(loaded.ok(), loaded.ok() ? "" : qPrintable(loaded.error().message));
        QCOMPARE(io::serializeProject(loaded.value()), first);
    }

    void writesHeader() {
        const QJsonObject root = json(makeDefaultProject());
        QCOMPARE(root.value(u"format").toString(), u"cutline.project"_s);
        QCOMPARE(root.value(u"version").toInt(), 1);
        QCOMPARE(root.value(u"createdWith").toString(), u"CUTLINE "_s + QString::fromLatin1(kVersion));
        QCOMPARE(root.value(u"createdWith").toString(), u"CUTLINE 0.1.0"_s);
        QVERIFY(root.value(u"media").isArray());
        QVERIFY(root.value(u"bins").isArray());
    }

    void newerVersionRefused() {
        QJsonObject root = json(makeDefaultProject());
        root.insert(u"version"_s, 2);
        const auto r = io::deserializeProject(QJsonDocument(root).toJson());
        QVERIFY(!r.ok());
        QCOMPARE(r.error().code, Error::Code::NewerVersion);
        QCOMPARE(r.error().message,
                 u"This project was made with a newer version of CUTLINE (project format 2). Update CUTLINE to open it."_s);
    }

    void notACutlineProject() {
        for (const QByteArray& text : {QByteArray(R"({"format":"other"})"), QByteArray("[]")}) {
            const auto r = io::deserializeProject(text);
            QVERIFY(!r.ok());
            QCOMPARE(r.error().code, Error::Code::NotACutlineProject);
            QCOMPARE(r.error().message, u"This file isn't a CUTLINE project."_s);
        }
    }

    void corruptJsonReportsLine() {
        const auto r = io::deserializeProject("{\n  \"format\": \"cutline.project\",\n  \"version\": 1 oops\n}\n");
        QVERIFY(!r.ok());
        QCOMPARE(r.error().code, Error::Code::Corrupt);
        QVERIFY2(r.error().message.startsWith(u"The project file is damaged: "_s), qPrintable(r.error().message));
        QVERIFY2(r.error().message.contains(u"(line 3, column"_s), qPrintable(r.error().message));
    }

    void missingFieldNamed() {
        const auto r = io::deserializeProject(withFormat([](QJsonObject& f) { f.remove(u"width"_s); }));
        QVERIFY(!r.ok());
        QCOMPARE(r.error().code, Error::Code::MissingField);
        QCOMPARE(r.error().message, u"The project file is missing sequences[0].format.width."_s);
    }

    void wrongTypeNamed() {
        const auto r = io::deserializeProject(withFormat([](QJsonObject& f) { f.insert(u"width"_s, u"abc"_s); }));
        QVERIFY(!r.ok());
        QCOMPARE(r.error().code, Error::Code::WrongType);
        QCOMPARE(r.error().message, u"sequences[0].format.width should be a number."_s);
    }

    void invalidValueRejected() {
        const auto r = io::deserializeProject(withFormat([](QJsonObject& f) { f.insert(u"width"_s, 15); }));
        QVERIFY(!r.ok());
        QCOMPARE(r.error().code, Error::Code::InvalidValue);
        QCOMPARE(r.error().message,
                 u"sequences[0].format: Width and height must be even numbers between 16 and 8192."_s);
    }

    void unknownFieldsIgnored() {
        QJsonObject root = json(makeDefaultProject());
        root.insert(u"futureFeature"_s, QJsonObject{{u"x"_s, 1}});
        const auto r = io::deserializeProject(QJsonDocument(root).toJson());
        QVERIFY(r.ok());
    }

    void unicodePathRoundTrip() {
        QTemporaryDir dir;
        const QString folder = dir.filePath(u"Zoë Vidéos"_s);
        QVERIFY(QDir().mkpath(folder));
        const QString path = folder + u"/Mon projet 東京.cutline"_s;
        const Project p = modifiedProject();
        QVERIFY(io::saveProject(p, path).ok);
        const auto loaded = io::loadProject(path);
        QVERIFY2(loaded.ok(), loaded.ok() ? "" : qPrintable(loaded.error().message));
        QCOMPARE(io::serializeProject(loaded.value()), io::serializeProject(p));
    }

    void saveToMissingFolderFails() {
        QTemporaryDir dir;
        const Status status = io::saveProject(makeDefaultProject(), dir.filePath(u"no/such/folder/p.cutline"_s));
        QVERIFY(!status.ok);
        QVERIFY2(status.message.startsWith(u"Couldn't save the project:"_s), qPrintable(status.message));
    }

    void saveFailureLeavesExistingFileIntact() {
        QTemporaryDir dir;
        const QString path = dir.filePath(u"p.cutline"_s);
        Project a = makeDefaultProject();
        makeRenameProject(u"A"_s)->apply(a);
        QVERIFY(io::saveProject(a, path).ok);
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        const QByteArray original = file.readAll();
        file.close();

        QVERIFY(QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::ReadUser));
        Project b = a;
        makeRenameProject(u"B"_s)->apply(b);
        const Status status = io::saveProject(b, path);
        QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadUser |
                                        QFileDevice::WriteUser);
        QVERIFY(!status.ok);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), original);
    }

    void openMissingFile() {
        const auto r = io::loadProject(u"definitely/not/here.cutline"_s);
        QVERIFY(!r.ok());
        QCOMPARE(r.error().code, Error::Code::Io);
    }
};

QTEST_GUILESS_MAIN(TestProjectIO)
#include "tst_projectio.moc"
