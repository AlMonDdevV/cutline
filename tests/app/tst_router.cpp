#include "app/actions/ActionRegistry.h"
#include "app/actions/ShortcutRecorder.h"
#include "app/actions/ShortcutRouter.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QSignalSpy>
#include <QtQml/qqmlextensionplugin.h>
#include <QtTest>

Q_IMPORT_QML_PLUGIN(CutlinePlugin)

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

// A key event as the platform reports a digit-row key, with the native codes a real keyboard sends.
QKeyEvent digitRowEvent(int digit, Qt::KeyboardModifiers mods, Qt::Key reportedKey, const QString& text) {
#if defined(Q_OS_WIN)
    const quint32 scan = digit == 0 ? 0x0B : quint32(0x01 + digit);
    const quint32 vk = quint32(0x30 + digit);
#elif defined(Q_OS_MACOS)
    static const quint32 macCodes[] = {0x1D, 0x12, 0x13, 0x14, 0x15, 0x17, 0x16, 0x1A, 0x1C, 0x19};
    const quint32 scan = 0;
    const quint32 vk = macCodes[digit];
#else
    const quint32 scan = digit == 0 ? 19 : quint32(9 + digit);
    const quint32 vk = 0;
#endif
    return QKeyEvent(QEvent::KeyPress, reportedKey, mods, scan, vk, 0, text);
}

} // namespace

class TestRouter : public QObject {
    Q_OBJECT

    std::unique_ptr<QQmlApplicationEngine> engine;
    std::unique_ptr<ActionRegistry> registry;
    std::unique_ptr<ShortcutRouter> router;
    QQuickWindow* window = nullptr;

    QQuickItem* item(const char* name) {
        auto* found = window->findChild<QQuickItem*>(QLatin1String(name));
        if (!found)
            qFatal("RouterScene has no item named %s", name);
        return found;
    }

    int* counter(const QString& id, Contexts contexts, QKeySequence key, bool textEntry = false) {
        auto* count = new int(0);
        ActionSpec s;
        s.id = id;
        s.category = u"Test"_s;
        s.title = id;
        s.contexts = contexts;
        s.defaultShortcuts = {key};
        s.worksInTextEntry = textEntry;
        registry->add(s, [count] { ++*count; });
        m_counters.push_back(std::unique_ptr<int>(count));
        return count;
    }
    std::vector<std::unique_ptr<int>> m_counters;

    void focus(const char* name) {
        QQuickItem* i = item(name);
        QVERIFY2(i, name);
        i->forceActiveFocus();
        QVERIFY(i->hasActiveFocus());
    }

private slots:
    void initTestCase() { QQuickStyle::setStyle(u"Basic"_s); }

    void init() {
        engine = std::make_unique<QQmlApplicationEngine>();
        engine->load(QUrl::fromLocalFile(QStringLiteral(CUTLINE_TEST_DATA "/RouterScene.qml")));
        QCOMPARE(engine->rootObjects().size(), 1);
        window = qobject_cast<QQuickWindow*>(engine->rootObjects().first());
        QVERIFY(window);
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));
        registry = std::make_unique<ActionRegistry>();
        router = std::make_unique<ShortcutRouter>(*registry);
        router->attach(window);
    }

    void cleanup() {
        // Let a closing popup finish its queued work before the window and engine go away.
        if (m_popup) {
            QObject::disconnect(m_popup, nullptr, this, nullptr);
            m_popup = nullptr;
        }
        if (window)
            window->close();
        QCoreApplication::processEvents();
        engine.reset();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
        router.reset();
        registry.reset();
        m_counters.clear();
        window = nullptr;
    }

    void spaceInTextFieldTypes() {
        int* hits = counter(u"play"_s, {}, QKeySequence(Qt::Key_Space));
        focus("fieldA");
        QTest::keyClick(window, Qt::Key_Space, Qt::NoModifier);
        QCOMPARE(*hits, 0);
        QCOMPARE(item("fieldA")->property("text").toString(), u" "_s);
    }

    void letterInTextFieldTypes() {
        int* hits = counter(u"blade"_s, {}, QKeySequence(u"B"_s));
        focus("fieldA");
        QTest::keyClick(window, Qt::Key_B, Qt::NoModifier);
        QCOMPARE(*hits, 0);
        QCOMPARE(item("fieldA")->property("text").toString(), u"b"_s);
    }

    void deleteInTextFieldEdits() {
        int* del = counter(u"delete"_s, {}, QKeySequence(Qt::Key_Delete));
        int* back = counter(u"back"_s, {}, QKeySequence(Qt::Key_Backspace));
        focus("fieldA");
        item("fieldA")->setProperty("text", u"ab"_s);
        item("fieldA")->setProperty("cursorPosition", 2);
        QTest::keyClick(window, Qt::Key_Backspace);
        QCOMPARE(item("fieldA")->property("text").toString(), u"a"_s);
        item("fieldA")->setProperty("cursorPosition", 0);
        QTest::keyClick(window, Qt::Key_Delete);
        QCOMPARE(item("fieldA")->property("text").toString(), QString());
        QCOMPARE(*del + *back, 0);
    }

    void ctrlAInTextFieldSelectsText() {
        int* hits = counter(u"selectAll"_s, {}, QKeySequence(u"Ctrl+A"_s));
        focus("fieldA");
        item("fieldA")->setProperty("text", u"abc"_s);
        QTest::keyClick(window, Qt::Key_A, Qt::ControlModifier);
        QCOMPARE(*hits, 0);
        QCOMPARE(item("fieldA")->property("selectedText").toString(), u"abc"_s);
    }

    void textEntryAllowedActionFires() {
        int* hits = counter(u"save"_s, {}, QKeySequence(u"Ctrl+S"_s), true);
        focus("fieldA");
        QTest::keyClick(window, Qt::Key_S, Qt::ControlModifier);
        QCOMPARE(*hits, 1);
    }

    void panelContextRouting() {
        int* timeline = counter(u"t.x"_s, Context::Timeline, QKeySequence(u"X"_s));
        int* inspector = counter(u"i.x"_s, Context::Inspector, QKeySequence(u"X"_s));
        focus("timelineItem");
        QTest::keyClick(window, Qt::Key_X);
        QCOMPARE(*timeline, 1);
        QCOMPARE(*inspector, 0);
        focus("inspectorItem");
        QTest::keyClick(window, Qt::Key_X);
        QCOMPARE(*timeline, 1);
        QCOMPARE(*inspector, 1);
    }

    void panelScopedBeatsGlobal() {
        int* global = counter(u"g.y"_s, {}, QKeySequence(u"Y"_s));
        int* timeline = counter(u"t.y"_s, Context::Timeline, QKeySequence(u"Y"_s));
        focus("timelineItem");
        QTest::keyClick(window, Qt::Key_Y);
        QCOMPARE(*timeline, 1);
        QCOMPARE(*global, 0);
    }

    void modalCapturesKeys() {
        int* hits = counter(u"play"_s, {}, QKeySequence(Qt::Key_Space));
        auto* popup = window->findChild<QObject*>(u"testPopup"_s);
        QVERIFY(popup);
        connect(popup, SIGNAL(opened()), this, SLOT(onPopupOpened()));
        connect(popup, SIGNAL(closed()), this, SLOT(onPopupClosed()));
        m_popup = popup;
        QMetaObject::invokeMethod(popup, "open");
        QTRY_VERIFY(popup->property("opened").toBool());
        QTest::keyClick(window, Qt::Key_Space);
        QCOMPARE(*hits, 0);
        QTest::keyClick(window, Qt::Key_Escape);
        QTRY_VERIFY(!popup->property("visible").toBool());
    }
    void onPopupOpened() {
        if (router)
            router->pushModal(m_popup);
    }
    void onPopupClosed() {
        if (router)
            router->popModal(m_popup);
    }

    void escapeStack() {
        int* deselect = counter(u"edit.deselect"_s, {}, QKeySequence(Qt::Key_Escape));
        int cancelled = 0;
        focus("timelineItem");
        router->beginOperation(u"drag"_s, [&] { ++cancelled; });
        QTest::keyClick(window, Qt::Key_Escape);
        QCOMPARE(cancelled, 1);
        QCOMPARE(*deselect, 0);
        router->endOperation();

        focus("fieldA");
        QTest::keyClick(window, Qt::Key_Escape);
        QCOMPARE(window->activeFocusItem(), item("inspectorPanel"));
        QCOMPARE(*deselect, 0);
        QTest::keyClick(window, Qt::Key_Escape);
        QCOMPARE(*deselect, 1);
    }

    void recorderGetsEverything() {
        int* save = counter(u"save"_s, {}, QKeySequence(u"Ctrl+S"_s), true);
        auto* recorder = qobject_cast<ShortcutRecorder*>(item("recorder"));
        QVERIFY(recorder);
        QSignalSpy captured(recorder, &ShortcutRecorder::captured);
        recorder->setRecording(true);
        focus("recorder");
        QTest::keyClick(window, Qt::Key_S, Qt::ControlModifier);
        QCOMPARE(*save, 0);
        QCOMPARE(captured.count(), 1);
        QCOMPARE(captured.first().first().toString(), u"Ctrl+S"_s);
        QVERIFY(!recorder->recording());
    }

    void shiftDigitRowUs() {
        int* hits = counter(u"panel.inspector"_s, {}, QKeySequence(u"Shift+5"_s));
        int* one = counter(u"panel.media"_s, {}, QKeySequence(u"Shift+1"_s));
        focus("timelineItem");
        QKeyEvent ev = digitRowEvent(1, Qt::ShiftModifier, Qt::Key_Exclam, u"!"_s);
        QCoreApplication::sendEvent(window, &ev);
        QCOMPARE(*one, 1);
        QKeyEvent five = digitRowEvent(5, Qt::ShiftModifier, Qt::Key_Percent, u"%"_s);
        QCoreApplication::sendEvent(window, &five);
        QCOMPARE(*hits, 1);
    }

    void shiftDigitAzerty() {
        int* hits = counter(u"panel.media"_s, {}, QKeySequence(u"Shift+1"_s));
        focus("timelineItem");
        QKeyEvent ev = digitRowEvent(1, Qt::ShiftModifier, Qt::Key_1, u"1"_s);
        QCoreApplication::sendEvent(window, &ev);
        QCOMPARE(*hits, 1);
    }

    void backtabNormalized() {
        int* hits = counter(u"panel.previous"_s, {}, QKeySequence(u"Ctrl+Shift+Tab"_s), true);
        focus("timelineItem");
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Backtab, Qt::ControlModifier | Qt::ShiftModifier);
        QCoreApplication::sendEvent(window, &ev);
        QCOMPARE(*hits, 1);
    }

    void shiftedSymbolMatches() {
        int* hits = counter(u"zoomIn"_s, {}, QKeySequence(u"+"_s));
        focus("timelineItem");
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Plus, Qt::ShiftModifier, u"+"_s);
        QCoreApplication::sendEvent(window, &ev);
        QCOMPARE(*hits, 1);
    }

    void keypadStripped() {
        int* hits = counter(u"zoomOut"_s, {}, QKeySequence(u"-"_s));
        focus("timelineItem");
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Minus, Qt::KeypadModifier, u"-"_s);
        QCoreApplication::sendEvent(window, &ev);
        QCOMPARE(*hits, 1);
    }

    void disabledActionConsumesKey() {
        int* hits = counter(u"q"_s, {}, QKeySequence(u"Q"_s));
        registry->action(u"q"_s)->setEnabledWhen([] { return false; });
        focus("timelineItem");
        const int before = window->property("itemKeyCount").toInt();
        QTest::keyClick(window, Qt::Key_Q);
        QCOMPARE(*hits, 0);
        QCOMPARE(window->property("itemKeyCount").toInt(), before);
    }

    void tabFromCanvasOnlyWhenScoped() {
        int* hits = counter(u"panel.nextFromCanvas"_s, Contexts(Context::Timeline) | Context::Viewer,
                            QKeySequence(Qt::Key_Tab));
        focus("timelineItem");
        QTest::keyClick(window, Qt::Key_Tab);
        QCOMPARE(*hits, 1);
        focus("fieldA");
        QTest::keyClick(window, Qt::Key_Tab);
        QCOMPARE(*hits, 1);
        QVERIFY(item("fieldB")->hasActiveFocus());
    }

private:
    QObject* m_popup = nullptr;
};

QTEST_MAIN(TestRouter)
#include "tst_router.moc"
