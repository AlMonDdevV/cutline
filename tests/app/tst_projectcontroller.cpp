#include "app/controllers/AppSettings.h"
#include "app/controllers/HistoryModel.h"
#include "app/controllers/ProjectController.h"
#include "app/controllers/RecentProjects.h"
#include "app/controllers/SequenceInspector.h"
#include "app/controllers/TimecodeHelper.h"
#include "app/controllers/TracksModel.h"

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

QStringList column(const QAbstractItemModel& model, const QByteArray& role) {
    const int r = model.roleNames().key(role, -1);
    QStringList out;
    for (int i = 0; i < model.rowCount(); ++i)
        out << model.data(model.index(i, 0), r).toString();
    return out;
}

QVariant cell(const QAbstractItemModel& model, int row, const QByteArray& role) {
    return model.data(model.index(row, 0), model.roleNames().key(role, -1));
}

} // namespace

class TestProjectController : public QObject {
    Q_OBJECT

    QTemporaryDir dir;
    std::unique_ptr<AppSettings> settings;
    std::unique_ptr<RecentProjects> recent;
    std::unique_ptr<ProjectController> project;

    QUrl url(const QString& name) { return QUrl::fromLocalFile(dir.filePath(name)); }

private slots:
    void init() {
        const QString config = dir.filePath(u"config-%1"_s.arg(QTest::currentTestFunction()));
        QDir().mkpath(config);
        settings = std::make_unique<AppSettings>(config);
        recent = std::make_unique<RecentProjects>(*settings);
        project = std::make_unique<ProjectController>(*settings, *recent);
    }
    void cleanup() {
        project.reset();
        recent.reset();
        settings.reset();
    }

    void windowTitle() {
        QCOMPARE(project->windowTitle(), u"Untitled — CUTLINE"_s);
        QVERIFY(project->setProjectName(u"Interview"_s));
        QCOMPARE(project->windowTitle(), u"Interview* — CUTLINE"_s);
    }

    void saveAsThenSave() {
        QVERIFY(project->setProjectName(u"First"_s));
        QVERIFY(project->saveAs(url(u"a.cutline"_s)));
        QVERIFY(QFile::exists(dir.filePath(u"a.cutline"_s)));
        QVERIFY(!project->dirty());
        QCOMPARE(project->filePath(), dir.filePath(u"a.cutline"_s));
        QVERIFY(project->setProjectName(u"Second"_s));
        QVERIFY(project->save());
        QFile f(dir.filePath(u"a.cutline"_s));
        QVERIFY(f.open(QIODevice::ReadOnly));
        QVERIFY(f.readAll().contains("Second"));
    }

    void saveWithoutPathRequestsSaveAs() {
        QSignalSpy spy(project.get(), &ProjectController::saveAsRequested);
        QVERIFY(!project->save());
        QCOMPARE(spy.count(), 1);
    }

    void openReplacesAndClearsHistory() {
        QVERIFY(project->setProjectName(u"Stored"_s));
        QVERIFY(project->saveAs(url(u"b.cutline"_s)));
        project->newProject();
        QVERIFY(project->setProjectName(u"Other"_s));
        QVERIFY(project->openProject(url(u"b.cutline"_s)));
        QCOMPARE(project->projectName(), u"Stored"_s);
        QVERIFY(!project->canUndo());
        QVERIFY(!project->dirty());
    }

    void openErrorEmits() {
        QFile f(dir.filePath(u"garbage.cutline"_s));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("[]");
        f.close();
        QSignalSpy spy(project.get(), &ProjectController::errorOccurred);
        QVERIFY(!project->openProject(url(u"garbage.cutline"_s)));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().at(0).toString(), u"Couldn't open project"_s);
        QCOMPARE(spy.first().at(1).toString(), u"This file isn't a CUTLINE project."_s);
    }

    void saveErrorKeepsDirty() {
        QVERIFY(project->setProjectName(u"Dirty"_s));
        QSignalSpy spy(project.get(), &ProjectController::errorOccurred);
        QVERIFY(!project->saveAs(url(u"missing/folder/x.cutline"_s)));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().at(0).toString(), u"Couldn't save project"_s);
        QVERIFY(spy.first().at(1).toString().startsWith(u"Couldn't save the project:"_s));
        QVERIFY(project->dirty());
    }

    void recentProjects() {
        QStringList saved;
        for (int i = 0; i < 12; ++i) {
            const QString name = u"p%1.cutline"_s.arg(i);
            QVERIFY(project->saveAs(url(name)));
            saved.prepend(dir.filePath(name));
        }
        QCOMPARE(recent->paths().size(), 10);
        QCOMPARE(recent->paths().first(), saved.first());
        QVERIFY(project->saveAs(url(u"p5.cutline"_s)));
        QCOMPARE(recent->paths().first(), dir.filePath(u"p5.cutline"_s));
        QCOMPARE(recent->paths().count(dir.filePath(u"p5.cutline"_s)), 1);
        QVERIFY(QFile::remove(dir.filePath(u"p11.cutline"_s)));
        QVERIFY(!recent->paths().contains(dir.filePath(u"p11.cutline"_s)));
    }

    void recentProjectsUnicode() {
        const QString folder = dir.filePath(u"Zoë Vidéos"_s);
        QVERIFY(QDir().mkpath(folder));
        const QString path = folder + u"/Mon projet 東京.cutline"_s;
        QVERIFY(project->saveAs(QUrl::fromLocalFile(path)));
        QCOMPARE(recent->paths().first(), path);
        project->newProject();
        QVERIFY(project->openProject(QUrl::fromLocalFile(recent->paths().first())));
        QCOMPARE(project->filePath(), path);
    }

    void tracksModelOrder() {
        QCOMPARE(column(*project->tracks(), "code"), (QStringList{u"V2"_s, u"V1"_s, u"A1"_s, u"A2"_s}));
        QCOMPARE(cell(*project->tracks(), 0, "kind").toString(), u"video"_s);
        QCOMPARE(cell(*project->tracks(), 2, "kind").toString(), u"audio"_s);
    }

    void tracksModelUpdates() {
        QSignalSpy inserted(project->tracks(), &QAbstractItemModel::rowsInserted);
        QVERIFY(!project->addTrack(u"video"_s).isEmpty());
        QCOMPARE(inserted.count(), 1);
        QCOMPARE(cell(*project->tracks(), 0, "code").toString(), u"V3"_s);
        QCOMPARE(project->tracks()->rowCount(), 5);
    }

    void historyModel() {
        project->addTrack(u"video"_s);
        QCOMPARE(column(*project->history(), "title"), (QStringList{u"Initial state"_s, u"Add video track"_s}));
        QVERIFY(cell(*project->history(), 1, "isCurrent").toBool());
        project->undo();
        QVERIFY(cell(*project->history(), 0, "isCurrent").toBool());
        QVERIFY(cell(*project->history(), 1, "isUndone").toBool());
    }

    void sequenceInspector() {
        SequenceInspector* s = project->sequence();
        QCOMPARE(s->resolutionIndex(), 0);
        s->setResolutionIndex(2);
        QCOMPARE(s->width(), 1080);
        QCOMPARE(s->height(), 1920);
        project->undo();
        QCOMPARE(s->width(), 1920);
        QCOMPARE(s->height(), 1080);
        QCOMPARE(s->resolutionIndex(), 0);
        s->setFrameRateIndex(3);
        QCOMPARE(project->document().project().sequences.front().format.frameRate, (FrameRate{30000, 1001}));
        QCOMPARE(s->frameRateIndex(), 3);
    }

    void failureSetsStatus() {
        const auto& tracks = project->document().project().sequences.front().tracks;
        const QString v1 = tracks.at(0).id;
        const QString v2 = tracks.at(1).id;
        QVERIFY(project->removeTrack(v2));
        QVERIFY(!project->removeTrack(v1));
        QCOMPARE(project->statusMessage(), u"A sequence needs at least one video track."_s);
    }

    void timecodeHelper() {
        TimecodeHelper tc;
        QCOMPARE(tc.format(1800, 30000, 1001, true, 0), u"00:01:00;02"_s);
        QCOMPARE(tc.significantIndex(u"00:01:14:21"_s), 4);
    }

    void newProjectPresets() {
        project->newProject(u"vertical"_s);
        const auto& f = project->document().project().sequences.front().format;
        QCOMPARE(f.width, 1080);
        QCOMPARE(f.height, 1920);
        QCOMPARE(f.frameRate, (FrameRate{30, 1}));
        QVERIFY(!project->dirty());
        project->newProject(u"youtube"_s);
        QCOMPARE(project->document().project().sequences.front().format.width, 1920);
    }
};

QTEST_MAIN(TestProjectController)
#include "tst_projectcontroller.moc"
