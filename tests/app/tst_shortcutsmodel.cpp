#include "app/AppServices.h"
#include "app/actions/ActionRegistry.h"
#include "app/actions/ShortcutProfile.h"
#include "app/actions/ShortcutsModel.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

class TestShortcutsModel : public QObject {
    Q_OBJECT

    QTemporaryDir dir;
    std::unique_ptr<AppServices> services;
    std::unique_ptr<ShortcutsModel> model;

    QStringList ids() const {
        QStringList out;
        const int role = model->roleNames().key("actionId");
        for (int i = 0; i < model->rowCount(); ++i)
            out << model->data(model->index(i), role).toString();
        return out;
    }

private slots:
    void init() {
        services = std::make_unique<AppServices>(dir.filePath(QString::fromLatin1(QTest::currentTestFunction())));
        model = std::make_unique<ShortcutsModel>(services->actions(), services->shortcutStore());
    }
    void cleanup() {
        model.reset();
        services.reset();
    }

    void searchByTitle() {
        model->setFilter(u"save project"_s);
        QCOMPARE(ids(), (QStringList{u"file.save"_s, u"file.saveAs"_s}));
    }

    void searchByKeys() {
        model->setFilter(u"ctrl+s"_s);
        QCOMPARE(ids(), QStringList{u"file.save"_s});
    }

    void categoryFilter() {
        model->setCategory(u"View"_s);
        const int role = model->roleNames().key("category");
        QVERIFY(model->rowCount() > 0);
        for (int i = 0; i < model->rowCount(); ++i)
            QCOMPARE(model->data(model->index(i), role).toString(), u"View"_s);
        QVERIFY(model->categories().startsWith(u"File"_s));
    }

    void conflictsListed() {
        const QVariantList c = model->conflictsFor(u"panel.history"_s, u"Ctrl+S"_s);
        QCOMPARE(c.size(), 1);
        const QVariantMap first = c.first().toMap();
        QCOMPARE(first.value(u"actionId"_s).toString(), u"file.save"_s);
        QCOMPARE(first.value(u"title"_s).toString(), u"Save Project"_s);
        QCOMPARE(first.value(u"contextsText"_s).toString(), u"Everywhere"_s);
    }

    void assignReplaces() {
        QVERIFY(model->assign(u"panel.history"_s, u"Ctrl+S"_s, true));
        QVERIFY(services->actions().action(u"file.save"_s)->shortcuts().isEmpty());
        QCOMPARE(services->actions().action(u"panel.history"_s)->shortcuts(), QList<QKeySequence>{QKeySequence(u"Ctrl+S"_s)});
        QFile f(services->shortcutStore().activePath());
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QByteArray saved = f.readAll();
        QVERIFY(saved.contains("panel.history"));
        QVERIFY(saved.contains("file.save"));
    }

    void assignWithoutReplaceRefuses() {
        QVERIFY(!model->assign(u"panel.history"_s, u"Ctrl+S"_s, false));
        // Native key text: "Ctrl+S" on Windows and Linux, "⌘S" on macOS.
        QCOMPARE(model->lastError(),
                 QKeySequence(u"Ctrl+S"_s).toString(QKeySequence::NativeText) + u" is already used by “Save Project”."_s);
        QVERIFY(services->actions().action(u"panel.history"_s)->shortcuts().isEmpty());
    }

    void fixedCannotChange() {
        QVERIFY(!model->assign(u"edit.deselect"_s, u"Ctrl+D"_s, false));
        QCOMPARE(model->lastError(), u"This shortcut can't be changed."_s);
    }

    void resetAll() {
        QVERIFY(model->assign(u"panel.history"_s, u"Ctrl+H"_s, false));
        model->resetAll();
        QVERIFY(services->actions().action(u"panel.history"_s)->shortcuts().isEmpty());
        QVERIFY(services->actions().profile().overrides.isEmpty());
    }

    void importBadFile() {
        const QString path = dir.filePath(u"bad.json"_s);
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("{}");
        f.close();
        QVERIFY(!model->importProfile(QUrl::fromLocalFile(path)));
        QCOMPARE(model->lastError(), u"This file isn't a CUTLINE shortcuts profile."_s);
    }

    void exportImportRoundTrip() {
        QVERIFY(model->assign(u"panel.history"_s, u"Ctrl+H"_s, false));
        const QUrl file = QUrl::fromLocalFile(dir.filePath(u"keys.cutline-keys.json"_s));
        QVERIFY(model->exportProfile(file));
        model->resetAll();
        QVERIFY(model->importProfile(file));
        QCOMPARE(services->actions().action(u"panel.history"_s)->shortcuts(), QList<QKeySequence>{QKeySequence(u"Ctrl+H"_s)});
    }

    void overriddenRole() {
        QVERIFY(model->assign(u"panel.history"_s, u"Ctrl+H"_s, false));
        model->setFilter(u"show history"_s);
        QCOMPARE(model->rowCount(), 1);
        QVERIFY(model->data(model->index(0), model->roleNames().key("overridden")).toBool());
        QCOMPARE(model->data(model->index(0), model->roleNames().key("shortcutsText")).toString(),
                 QKeySequence(u"Ctrl+H"_s).toString(QKeySequence::NativeText));
    }
};

QTEST_MAIN(TestShortcutsModel)
#include "tst_shortcutsmodel.moc"
