#include "app/AppServices.h"
#include "app/actions/ActionRegistry.h"
#include "app/controllers/AppSettings.h"
#include "app/controllers/ProjectController.h"
#include "app/controllers/TracksModel.h"
#include "app/theme/ThemeController.h"

#include <QTemporaryDir>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

class TestDefaultActions : public QObject {
    Q_OBJECT

    QTemporaryDir dir;
    std::unique_ptr<AppServices> services;

private slots:
    void init() { services = std::make_unique<AppServices>(dir.filePath(QString::fromLatin1(QTest::currentTestFunction()))); }
    void cleanup() { services.reset(); }

    void allIdsRegistered() {
        QStringList ids;
        for (Action* a : services->actions().all())
            ids << a->id();
        const QStringList expected = {
            u"file.new"_s, u"file.open"_s, u"file.save"_s, u"file.saveAs"_s, u"app.quit"_s,
            u"edit.undo"_s, u"edit.redo"_s, u"edit.deselect"_s, u"app.settings"_s,
            u"sequence.addVideoTrack"_s, u"sequence.addAudioTrack"_s, u"sequence.removeTrack"_s,
            u"panel.inspector"_s, u"panel.history"_s, u"panel.next"_s, u"panel.previous"_s, u"panel.nextFromCanvas"_s,
            u"view.fullscreen"_s, u"view.themeDark"_s, u"view.themeLight"_s, u"view.themeSystem"_s,
            u"workspace.saveAs"_s, u"workspace.reset"_s, u"help.shortcuts"_s, u"help.about"_s,
        };
        QCOMPARE(ids, expected);
    }

    void defaultsHaveNoConflicts() {
        const auto all = services->actions().all();
        for (Action* a : all)
            for (const QKeySequence& seq : a->shortcuts()) {
                const auto clashes = services->actions().conflicts(seq, a->spec().contexts, a->id());
                QVERIFY2(clashes.isEmpty(), qPrintable(a->id() + u" clashes on "_s + seq.toString()));
            }
    }

    void undoEnabledFollowsHistory() {
        Action* undo = services->actions().action(u"edit.undo"_s);
        QVERIFY(!undo->isEnabled());
        services->project().addTrack(u"video"_s);
        QVERIFY(undo->isEnabled());
        QVERIFY(undo->trigger());
        QCOMPARE(services->project().tracks()->rowCount(), 4);
    }

    void textEntryFlags() {
        const QStringList yes = {u"file.new"_s, u"file.open"_s, u"file.save"_s, u"file.saveAs"_s, u"app.quit"_s,
                                 u"app.settings"_s, u"panel.next"_s, u"panel.previous"_s, u"view.fullscreen"_s};
        for (Action* a : services->actions().all())
            QCOMPARE(a->spec().worksInTextEntry, yes.contains(a->id()));
    }

    void themeActionsChecked() {
        Action* light = services->actions().action(u"view.themeLight"_s);
        QVERIFY(light->trigger());
        QVERIFY(!services->theme().dark());
        QVERIFY(light->isChecked());
        QVERIFY(!services->actions().action(u"view.themeDark"_s)->isChecked());
        QCOMPARE(services->settings().theme(), u"light"_s);
    }

    void startupWarningsCollectCorruptFiles() {
        services.reset();
        const QString config = dir.filePath(u"corrupt"_s);
        QDir().mkpath(config);
        QFile f(config + u"/shortcuts.json"_s);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("{oops");
        f.close();
        services = std::make_unique<AppServices>(config);
        QVERIFY(services->startupWarnings().contains(
            u"Your keyboard shortcuts couldn't be read, so the defaults are in use."_s));
    }
};

QTEST_MAIN(TestDefaultActions)
#include "tst_defaultactions.moc"
