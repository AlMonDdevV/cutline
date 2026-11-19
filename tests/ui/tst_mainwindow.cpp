#include "app/AppServices.h"
#include "app/actions/ActionRegistry.h"
#include "app/actions/ShortcutRouter.h"
#include "app/controllers/AppSettings.h"
#include "app/controllers/ProjectController.h"
#include "app/controllers/TracksModel.h"
#include "app/icons/IconProvider.h"
#include "app/layout/WorkspaceManager.h"
#include "app/theme/Fonts.h"
#include "app/theme/ThemeController.h"

#include <QImage>
#include <QProcess>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QtQml/qqmlextensionplugin.h>
#include <QtTest>

Q_IMPORT_QML_PLUGIN(CutlinePlugin)

using namespace cutline;
using namespace Qt::StringLiterals;

class TestMainWindow : public QObject {
    Q_OBJECT

    QTemporaryDir dir;
    std::unique_ptr<AppServices> services;
    std::unique_ptr<QQmlApplicationEngine> engine;
    QQuickWindow* window = nullptr;

    void launch(bool showWelcome = false) {
        services = std::make_unique<AppServices>(dir.filePath(u"cfg-%1"_s.arg(QTest::currentTestFunction())));
        services->settings().setShowWelcome(showWelcome);
        engine = std::make_unique<QQmlApplicationEngine>();
        engine->addImageProvider(u"icons"_s, new IconProvider);
        engine->loadFromModule("Cutline", "Main");
        QVERIFY(!engine->rootObjects().isEmpty());
        window = qobject_cast<QQuickWindow*>(engine->rootObjects().first());
        QVERIFY(window);
        services->router().attach(window);
        window->requestActivate();
        QVERIFY(QTest::qWaitForWindowActive(window));
    }

    // Panels are created by Loaders/Repeaters, so search the visual item tree, not QObject children.
    static QQuickItem* findItem(QQuickItem* root, const QString& name) {
        if (!root)
            return nullptr;
        if (root->objectName() == name)
            return root;
        for (QQuickItem* child : root->childItems())
            if (QQuickItem* found = findItem(child, name))
                return found;
        return nullptr;
    }
    QQuickItem* item(const QString& name) {
        if (QQuickItem* found = findItem(window->contentItem(), name))
            return found;
        return window->findChild<QQuickItem*>(name);
    }
    QObject* object(const QString& name) { return window->findChild<QObject*>(name); }

    void focusPanel(const QString& id) {
        QQuickItem* p = item(u"panel."_s + id);
        QVERIFY2(p, qPrintable(id));
        p->forceActiveFocus();
        QTRY_VERIFY(p->hasActiveFocus());
    }

    QString focusedPanel() {
        for (const QString id : {u"viewer"_s, u"inspector"_s, u"history"_s, u"timeline"_s})
            if (QQuickItem* p = item(u"panel."_s + id); p && p->hasActiveFocus())
                return id;
        return {};
    }

private slots:
    void initTestCase() {
        QQuickStyle::setStyle(u"Basic"_s);
        loadBundledFonts();
    }

    void cleanup() {
        if (window)
            window->setProperty("forceQuit", true);
        if (window)
            window->close();
        QCoreApplication::processEvents();
        engine.reset();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QCoreApplication::processEvents();
        services.reset();
        window = nullptr;
    }

    void loads() {
        launch();
        QCOMPARE(window->title(), u"Untitled — CUTLINE"_s);
        for (const QString id : {u"viewer"_s, u"timeline"_s, u"inspector"_s})
            QVERIFY2(item(u"panel."_s + id) && item(u"panel."_s + id)->isVisible(), qPrintable(id));
        QVERIFY(item(u"panel.history"_s));
    }

    void menusOnlyUseRegisteredActions() {
        launch();
        int menuItems = 0;
        for (QObject* o : window->findChildren<QObject*>()) {
            const QVariant id = o->property("actionId");
            if (!id.isValid() || id.toString().isEmpty())
                continue;
            ++menuItems;
            QVERIFY2(services->actions().action(id.toString()), qPrintable(id.toString()));
        }
        QVERIFY(menuItems >= 20);
    }

    void everyUiActionHasHandler() {
        launch();
        const QVariantMap handlers = window->property("uiHandlerIds").toStringList().isEmpty()
                                         ? QVariantMap()
                                         : QVariantMap();
        const QStringList ids = window->property("uiHandlerIds").toStringList();
        for (const QString& id : services->actions().uiActionIds())
            QVERIFY2(ids.contains(id), qPrintable(id));
        Q_UNUSED(handlers);
    }

    void newWhenDirtyAsks() {
        launch();
        services->project().addTrack(u"video"_s);
        focusPanel(u"timeline"_s);
        QTest::keyClick(window, Qt::Key_N, Qt::ControlModifier);
        QObject* dialog = object(u"dialog.saveChanges"_s);
        QVERIFY(dialog);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QMetaObject::invokeMethod(dialog, "choose", Q_ARG(QVariant, u"discard"_s));
        QTRY_VERIFY(!services->project().dirty());
        QCOMPARE(services->project().tracks()->rowCount(), 4);
    }

    void closeWhenDirtyAsks() {
        launch();
        services->project().addTrack(u"audio"_s);
        window->close();
        QObject* dialog = object(u"dialog.saveChanges"_s);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QMetaObject::invokeMethod(dialog, "choose", Q_ARG(QVariant, u"cancel"_s));
        QTRY_VERIFY(!dialog->property("visible").toBool());
        QVERIFY(window->isVisible());
    }

    void renameTrackInline() {
        launch();
        int hits = 0;
        ActionSpec s;
        s.id = u"test.blade"_s;
        s.title = u"Blade"_s;
        s.defaultShortcuts = {QKeySequence(u"B"_s)};
        services->actions().add(s, [&hits] { ++hits; });

        QQuickItem* name = item(u"track.V1.name"_s);
        QVERIFY(name);
        QMetaObject::invokeMethod(name, "startEditing");
        QQuickItem* field = item(u"track.V1.editor"_s);
        QVERIFY(field);
        QTRY_VERIFY(field->hasActiveFocus());
        field->setProperty("text", QString());
        for (char c : QByteArray("Interview b"))
            QTest::keyClick(window, c);
        QTest::keyClick(window, Qt::Key_Return);
        QCOMPARE(hits, 0);
        const auto* tracks = services->project().tracks();
        const int nameRole = tracks->roleNames().key("name");
        QTRY_COMPARE(tracks->data(tracks->index(1, 0), nameRole).toString(), u"Interview b"_s);
    }

    void undoRedoKeys() {
        launch();
        focusPanel(u"timeline"_s);
        services->actions().action(u"sequence.addVideoTrack"_s)->trigger();
        QCOMPARE(services->project().tracks()->rowCount(), 5);
        QTest::keyClick(window, Qt::Key_Z, Qt::ControlModifier);
        QCOMPARE(services->project().tracks()->rowCount(), 4);
        QTest::keyClick(window, Qt::Key_Y, Qt::ControlModifier);
        QCOMPARE(services->project().tracks()->rowCount(), 5);
        QTest::keyClick(window, Qt::Key_Z, Qt::ControlModifier);
        QTest::keyClick(window, Qt::Key_Z, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(services->project().tracks()->rowCount(), 5);
    }

    void shiftFiveShowsInspector() {
        launch();
        services->workspaces().hidePanel(u"inspector"_s);
        QTRY_VERIFY(!item(u"panel.inspector"_s) || !item(u"panel.inspector"_s)->isVisible());
        focusPanel(u"timeline"_s);
        QKeyEvent five(QEvent::KeyPress, Qt::Key_Percent, Qt::ShiftModifier,
#if defined(Q_OS_WIN)
                       0x06, 0x35, 0,
#elif defined(Q_OS_MACOS)
                       0, 0x17, 0,
#else
                       14, 0, 0,
#endif
                       u"%"_s);
        QCoreApplication::sendEvent(window, &five);
        QTRY_VERIFY(item(u"panel.inspector"_s) && item(u"panel.inspector"_s)->isVisible());
        QTRY_VERIFY(item(u"panel.inspector"_s)->hasActiveFocus());
    }

    void f6CyclesPanels() {
        launch();
        focusPanel(u"viewer"_s);
        QTest::keyClick(window, Qt::Key_F6);
        QTRY_COMPARE(focusedPanel(), u"inspector"_s);
        QTest::keyClick(window, Qt::Key_F6);
        QTRY_COMPARE(focusedPanel(), u"timeline"_s);
        QTest::keyClick(window, Qt::Key_F6);
        QTRY_COMPARE(focusedPanel(), u"viewer"_s);
        QTest::keyClick(window, Qt::Key_F6, Qt::ShiftModifier);
        QTRY_COMPARE(focusedPanel(), u"timeline"_s);
    }

    void tabFromTimeline() {
        launch();
        focusPanel(u"timeline"_s);
        QTest::keyClick(window, Qt::Key_Tab);
        QTRY_COMPARE(focusedPanel(), u"viewer"_s);
        QQuickItem* nameField = item(u"inspector.sequenceName"_s);
        QVERIFY(nameField);
        nameField->forceActiveFocus();
        QTest::keyClick(window, Qt::Key_Tab);
        QTRY_VERIFY(!nameField->hasActiveFocus());
        QCOMPARE(focusedPanel(), u"inspector"_s);
    }

    void themeSwitch() {
        launch();
        services->actions().action(u"view.themeLight"_s)->trigger();
        QCOMPARE(services->theme().panel(), QColor(u"#EBEBEB"_s));
        QCOMPARE(window->color(), QColor(u"#D9D9D9"_s));
    }

    void smallScreenLayoutFits() {
        launch();
        services->theme().setScale(1.5);
        window->resize(1366, 768);
        QTRY_COMPARE(window->width(), 1366);
        QTest::qWait(100);
        const QMap<QString, QSize> minimum = {{u"viewer"_s, {320, 180}}, {u"inspector"_s, {240, 200}}, {u"timeline"_s, {360, 160}}};
        for (auto it = minimum.cbegin(); it != minimum.cend(); ++it) {
            QQuickItem* p = item(u"panel."_s + it.key());
            QVERIFY(p && p->isVisible());
            QVERIFY2(p->width() >= it.value().width() && p->height() >= it.value().height(),
                     qPrintable(u"%1 is %2x%3"_s.arg(it.key()).arg(p->width()).arg(p->height())));
        }
        QQuickItem* header = item(u"header.bar"_s);
        QVERIFY(header);
        QVERIFY(header->implicitWidth() <= window->width());
    }

    void welcomeOnFirstLaunch() {
        launch(true);
        QObject* welcome = object(u"dialog.welcome"_s);
        QVERIFY(welcome);
        QTRY_VERIFY(welcome->property("visible").toBool());
        QMetaObject::invokeMethod(welcome, "start", Q_ARG(QVariant, u"vertical"_s));
        QTRY_VERIFY(!welcome->property("visible").toBool());
        QCOMPARE(services->project().document().project().sequences.front().format.width, 1080);
        QCOMPARE(services->project().document().project().sequences.front().format.height, 1920);
    }

    void welcomeRespectsSetting() {
        launch(false);
        QObject* welcome = object(u"dialog.welcome"_s);
        QTest::qWait(200);
        QVERIFY(!welcome || !welcome->property("visible").toBool());
    }

    void screenshotMode() {
        const QString out = dir.filePath(u"shot.png"_s);
        QProcess p;
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        p.setProcessEnvironment(env);
        p.start(QCoreApplication::applicationDirPath() + u"/cutline"_s,
                {u"--screenshot"_s, out, u"--size"_s, u"1600x900"_s, u"--theme"_s, u"light"_s});
        QVERIFY(p.waitForFinished(60000));
        QCOMPARE(p.exitCode(), 0);
        const QImage img(out);
        QVERIFY(!img.isNull());
        QCOMPARE(img.size() / img.devicePixelRatio(), QSize(1600, 900));
    }
};

QTEST_MAIN(TestMainWindow)
#include "tst_mainwindow.moc"
