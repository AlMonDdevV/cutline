#include "app/actions/ActionRegistry.h"

#include <QSignalSpy>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

ActionSpec spec(const QString& id, Contexts contexts, QList<QKeySequence> keys, bool textEntry = false) {
    ActionSpec s;
    s.id = id;
    s.category = u"Test"_s;
    s.title = id;
    s.contexts = contexts;
    s.defaultShortcuts = std::move(keys);
    s.worksInTextEntry = textEntry;
    return s;
}

} // namespace

class TestActions : public QObject {
    Q_OBJECT

private slots:
    void overlapRules() {
        QVERIFY(contextsOverlap({}, Context::Timeline));
        QVERIFY(!contextsOverlap(Context::Timeline, Context::Media));
        QVERIFY(contextsOverlap(Contexts(Context::Timeline) | Context::Viewer, Context::Viewer));
        QCOMPARE(contextsText({}), u"Everywhere"_s);
        QCOMPARE(contextsText(Contexts(Context::Timeline) | Context::Viewer), u"Timeline, Viewer"_s);
    }

    void triggerRunsHandlerWhenEnabled() {
        ActionRegistry registry;
        int count = 0;
        bool enabled = true;
        Action* a = registry.add(spec(u"a"_s, {}, {}), [&] { ++count; });
        a->setEnabledWhen([&] { return enabled; });
        QVERIFY(a->trigger());
        QCOMPARE(count, 1);
        enabled = false;
        a->refresh();
        QVERIFY(!a->trigger());
        QCOMPARE(count, 1);
    }

    void uiActionEmits() {
        ActionRegistry registry;
        Action* a = registry.addUi(spec(u"file.open"_s, {}, {}));
        QSignalSpy spy(&registry, &ActionRegistry::uiRequested);
        a->trigger();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().toString(), u"file.open"_s);
        QCOMPARE(registry.uiActionIds(), QStringList{u"file.open"_s});
    }

    void conflictsDetected() {
        ActionRegistry registry;
        registry.add(spec(u"A"_s, {}, {QKeySequence(u"Ctrl+K"_s)}), [] {});
        registry.add(spec(u"B"_s, Context::Timeline, {}), [] {});
        const auto c = registry.conflicts(QKeySequence(u"Ctrl+K"_s), Context::Timeline, u"B"_s);
        QCOMPARE(c.size(), 1);
        QCOMPARE(c.first().actionId, u"A"_s);
    }

    void noConflictForDisjointPanels() {
        ActionRegistry registry;
        registry.add(spec(u"A"_s, Context::Timeline, {QKeySequence(u"X"_s)}), [] {});
        registry.add(spec(u"B"_s, Context::Media, {}), [] {});
        QVERIFY(registry.conflicts(QKeySequence(u"X"_s), Context::Media, u"B"_s).isEmpty());
    }

    void assignAndReset() {
        ActionRegistry registry;
        Action* a = registry.add(spec(u"A"_s, {}, {QKeySequence(u"Ctrl+K"_s)}), [] {});
        QSignalSpy spy(&registry, &ActionRegistry::shortcutsChanged);
        QVERIFY(registry.assign(u"A"_s, {QKeySequence(u"Ctrl+J"_s)}).ok);
        QCOMPARE(a->shortcuts(), QList<QKeySequence>{QKeySequence(u"Ctrl+J"_s)});
        QCOMPARE(spy.count(), 1);
        registry.resetToDefault(u"A"_s);
        QCOMPARE(a->shortcuts(), QList<QKeySequence>{QKeySequence(u"Ctrl+K"_s)});
        registry.assign(u"A"_s, {});
        QVERIFY(a->shortcuts().isEmpty());
        registry.resetAll();
        QCOMPARE(a->shortcuts().size(), 1);
    }

    void fixedShortcutCannotChange() {
        ActionRegistry registry;
        ActionSpec s = spec(u"edit.deselect"_s, {}, {QKeySequence(Qt::Key_Escape)});
        s.fixedShortcut = true;
        registry.add(s, [] {});
        const Status st = registry.assign(u"edit.deselect"_s, {QKeySequence(u"Ctrl+D"_s)});
        QVERIFY(!st.ok);
        QCOMPARE(st.message, u"This shortcut can't be changed."_s);
    }

    void matchingPrefersPanelScoped() {
        ActionRegistry registry;
        Action* global = registry.add(spec(u"g"_s, {}, {QKeySequence(u"Y"_s)}), [] {});
        Action* timeline = registry.add(spec(u"t"_s, Context::Timeline, {QKeySequence(u"Y"_s)}), [] {});
        const auto m = registry.matching(QKeyCombination(Qt::Key_Y), Context::Timeline, false);
        QCOMPARE(m.size(), 2);
        QCOMPARE(m.at(0), timeline);
        QCOMPARE(m.at(1), global);
        const auto other = registry.matching(QKeyCombination(Qt::Key_Y), Context::Media, false);
        QCOMPARE(other, QList<Action*>{global});
        const auto none = registry.matching(QKeyCombination(Qt::Key_Y), std::nullopt, false);
        QCOMPARE(none, QList<Action*>{global});
    }

    void matchingRespectsTextEntry() {
        ActionRegistry registry;
        registry.add(spec(u"space"_s, {}, {QKeySequence(Qt::Key_Space)}), [] {});
        Action* save = registry.add(spec(u"save"_s, {}, {QKeySequence(u"Ctrl+S"_s)}, true), [] {});
        QVERIFY(registry.matching(QKeyCombination(Qt::Key_Space), std::nullopt, true).isEmpty());
        QCOMPARE(registry.matching(QKeyCombination(Qt::ControlModifier, Qt::Key_S), std::nullopt, true),
                 QList<Action*>{save});
    }

    void shortcutTextIsNative() {
        ActionRegistry registry;
        Action* a = registry.add(spec(u"s"_s, {}, {QKeySequence(u"Ctrl+S"_s)}), [] {});
        QCOMPARE(a->shortcutText(), QKeySequence(u"Ctrl+S"_s).toString(QKeySequence::NativeText));
    }
};

QTEST_MAIN(TestActions)
#include "tst_actions.moc"
