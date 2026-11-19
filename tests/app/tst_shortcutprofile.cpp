#include "app/actions/ActionRegistry.h"
#include "app/actions/ShortcutProfile.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

void addDefaults(ActionRegistry& registry) {
    ActionSpec history;
    history.id = u"panel.history"_s;
    history.category = u"View"_s;
    history.title = u"Show History"_s;
    registry.add(history, [] {});
    ActionSpec save;
    save.id = u"file.save"_s;
    save.category = u"File"_s;
    save.title = u"Save Project"_s;
    save.defaultShortcuts = {QKeySequence(u"Ctrl+S"_s)};
    registry.add(save, [] {});
}

} // namespace

class TestShortcutProfile : public QObject {
    Q_OBJECT

private slots:
    void roundTrip() {
        ShortcutProfile p;
        p.name = u"Mine"_s;
        p.overrides.insert(u"panel.history"_s, {QKeySequence(u"Ctrl+H"_s), QKeySequence(u"F9"_s)});
        p.overrides.insert(u"file.save"_s, {});
        const auto back = parseShortcutProfile(serializeShortcutProfile(p));
        QVERIFY(back.ok());
        QCOMPARE(back.value().name, u"Mine"_s);
        QCOMPARE(back.value().overrides, p.overrides);
    }

    void rejectsForeignFile() {
        for (const QByteArray& text : {QByteArray(R"({"format":"something"})"), QByteArray("not json")}) {
            const auto r = parseShortcutProfile(text);
            QVERIFY(!r.ok());
            QCOMPARE(r.error().message, u"This file isn't a CUTLINE shortcuts profile."_s);
        }
    }

    void persistsAcrossRegistryRestart() {
        QTemporaryDir dir;
        ShortcutStore store(dir.path());
        {
            ActionRegistry first;
            addDefaults(first);
            QVERIFY(first.assign(u"panel.history"_s, {QKeySequence(u"Ctrl+H"_s)}).ok);
            QVERIFY(store.saveActive(first.profile()).ok);
        }
        ActionRegistry second;
        addDefaults(second);
        QString warning;
        second.applyProfile(store.loadActive(&warning));
        QVERIFY(warning.isEmpty());
        QCOMPARE(second.action(u"panel.history"_s)->shortcuts(), QList<QKeySequence>{QKeySequence(u"Ctrl+H"_s)});
        QCOMPARE(second.action(u"file.save"_s)->shortcuts(), QList<QKeySequence>{QKeySequence(u"Ctrl+S"_s)});
    }

    void importExport() {
        QTemporaryDir dir;
        ShortcutStore store(dir.path());
        ShortcutProfile p;
        p.name = u"Shared"_s;
        p.overrides.insert(u"file.save"_s, {QKeySequence(u"Ctrl+Alt+S"_s)});
        const QString path = dir.filePath(u"keys.cutline-keys.json"_s);
        QVERIFY(store.exportTo(p, path).ok);
        const auto imported = store.importFrom(path);
        QVERIFY(imported.ok());
        QCOMPARE(imported.value().overrides, p.overrides);
    }

    void unknownIdsIgnoredAndCounted() {
        ActionRegistry registry;
        addDefaults(registry);
        ShortcutProfile p;
        p.overrides.insert(u"future.action"_s, {QKeySequence(u"Ctrl+U"_s)});
        p.overrides.insert(u"file.save"_s, {QKeySequence(u"Ctrl+W"_s)});
        int ignored = 0;
        registry.applyProfile(p, &ignored);
        QCOMPARE(ignored, 1);
        QCOMPARE(registry.action(u"file.save"_s)->shortcuts(), QList<QKeySequence>{QKeySequence(u"Ctrl+W"_s)});
    }

    void corruptStoredProfileFallsBack() {
        QTemporaryDir dir;
        QFile f(dir.filePath(u"shortcuts.json"_s));
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("{not json");
        f.close();
        ShortcutStore store(dir.path());
        QString warning;
        const ShortcutProfile p = store.loadActive(&warning);
        QCOMPARE(p.name, u"Default"_s);
        QVERIFY(p.overrides.isEmpty());
        QCOMPARE(warning, u"Your keyboard shortcuts couldn't be read, so the defaults are in use."_s);
    }

    void missingStoredProfileIsQuietDefault() {
        QTemporaryDir dir;
        ShortcutStore store(dir.path());
        QString warning;
        QCOMPARE(store.loadActive(&warning).name, u"Default"_s);
        QVERIFY(warning.isEmpty());
    }
};

QTEST_MAIN(TestShortcutProfile)
#include "tst_shortcutprofile.moc"
