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
#include <QJSValue>
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
        // Start above contentItem so popups in the overlay are searched too.
        QQuickItem* root = window->contentItem()->parentItem() ? window->contentItem()->parentItem() : window->contentItem();
        if (QQuickItem* found = findItem(root, name))
            return found;
        return window->findChild<QQuickItem*>(name);
    }

    QObject* openKeyboardPage() {
        services->actions().action(u"help.shortcuts"_s)->trigger();
        QObject* settings = object(u"dialog.settings"_s);
        if (!settings || !QTest::qWaitFor([settings] { return settings->property("opened").toBool(); }, 3000))
            return nullptr;
        return settings;
    }

    void record(const QString& actionId, Qt::Key key, Qt::KeyboardModifiers mods) {
        // Like a user: search for the action so its row is on screen.
        QQuickItem* search = item(u"keyboard.search"_s);
        QVERIFY(search);
        search->setProperty("text", services->actions().action(actionId)->title());
        QQuickItem* button = nullptr;
        QTRY_VERIFY((button = item(u"keyboard.record."_s + actionId)) != nullptr);
        QVERIFY2(button, qPrintable(actionId));
        QMetaObject::invokeMethod(button, "clicked");
        QQuickItem* recorder = item(u"keyboard.recorder."_s + actionId);
        QVERIFY(recorder);
        QTRY_VERIFY(recorder->hasActiveFocus());
        QTest::keyClick(window, key, mods);
    }
    QObject* object(const QString& name) { return window->findChild<QObject*>(name); }

    void focusPanel(const QString& id) {
        QQuickItem* p = item(u"panel."_s + id);
        QVERIFY2(p, qPrintable(id));
        p->forceActiveFocus();
        QTRY_VERIFY(p->hasActiveFocus());
    }

    QString focusedPanel() {
        for (const QString& id : {u"viewer"_s, u"inspector"_s, u"history"_s, u"timeline"_s})
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
        for (const QString& id : {u"viewer"_s, u"timeline"_s, u"inspector"_s})
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

    void ctrlCommaOpensSettings() {
        launch();
        focusPanel(u"timeline"_s);
        QTest::keyClick(window, Qt::Key_Comma, Qt::ControlModifier);
        QObject* settings = object(u"dialog.settings"_s);
        QVERIFY(settings);
        QTRY_VERIFY(settings->property("opened").toBool());
        QCOMPARE(settings->property("page").toString(), u"general"_s);
        QTest::keyClick(window, Qt::Key_Escape);
        QTRY_VERIFY(!settings->property("visible").toBool());
    }

    void recordShortcutFlow() {
        launch();
        QObject* settings = openKeyboardPage();
        QVERIFY(settings);
        QCOMPARE(settings->property("page").toString(), u"keyboard"_s);
        record(u"panel.history"_s, Qt::Key_H, Qt::ControlModifier);
        QTRY_COMPARE(services->actions().action(u"panel.history"_s)->shortcuts(),
                     QList<QKeySequence>{QKeySequence(u"Ctrl+H"_s)});
        QMetaObject::invokeMethod(settings, "close");
        QTRY_VERIFY(!settings->property("visible").toBool());
        focusPanel(u"timeline"_s);
        QTest::keyClick(window, Qt::Key_H, Qt::ControlModifier);
        QTRY_VERIFY(item(u"panel.history"_s) && item(u"panel.history"_s)->hasActiveFocus());
    }

    void conflictFlow() {
        launch();
        QVERIFY(openKeyboardPage());
        record(u"panel.history"_s, Qt::Key_S, Qt::ControlModifier);
        QObject* conflict = object(u"dialog.shortcutConflict"_s);
        QVERIFY(conflict);
        QTRY_VERIFY(conflict->property("opened").toBool());
        QCOMPARE(conflict->property("message").toString(),
                 u"%1 is already used by “Save Project” (Everywhere)."_s.arg(QKeySequence(u"Ctrl+S"_s).toString(QKeySequence::NativeText)));
        QMetaObject::invokeMethod(item(u"dialog.shortcutConflict.reassign"_s), "clicked");
        QTRY_VERIFY(services->actions().action(u"file.save"_s)->shortcuts().isEmpty());
        QMetaObject::invokeMethod(item(u"keyboard.restoreAll"_s), "clicked");
        QTRY_COMPARE(services->actions().action(u"file.save"_s)->shortcuts(), QList<QKeySequence>{QKeySequence(u"Ctrl+S"_s)});
    }

    void searchFieldDoesNotTriggerActions() {
        launch();
        int hits = 0;
        ActionSpec s;
        s.id = u"test.snap"_s;
        s.title = u"Snap"_s;
        s.defaultShortcuts = {QKeySequence(u"S"_s)};
        services->actions().add(s, [&hits] { ++hits; });
        QVERIFY(openKeyboardPage());
        QQuickItem* search = item(u"keyboard.search"_s);
        QVERIFY(search);
        search->forceActiveFocus();
        QTRY_VERIFY(search->hasActiveFocus());
        QTest::keyClick(window, 's');
        QTest::keyClick(window, Qt::Key_Space);
        QCOMPARE(hits, 0);
        QCOMPARE(search->property("text").toString(), u"s "_s);
    }

    // ---- Final-review fixes ----

    void settingsFitsAtLargeScale() {
        launch();
        window->resize(1366, 768);
        services->theme().setScale(1.5);
        QObject* settings = openKeyboardPage();
        QVERIFY(settings);
        auto* popup = qobject_cast<QObject*>(settings);
        QTRY_VERIFY(popup->property("height").toReal() <= window->height());
        QVERIFY(popup->property("width").toReal() <= window->width());
        QQuickItem* title = nullptr;
        QQuickItem* keys = nullptr;
        QTRY_VERIFY((title = item(u"keyboard.row.file.save.title"_s)) && (keys = item(u"keyboard.row.file.save.keys"_s)));
        const qreal titleRight = title->mapToItem(nullptr, QPointF(title->width(), 0)).x();
        const qreal keysLeft = keys->mapToItem(nullptr, QPointF(0, 0)).x();
        QVERIFY2(titleRight <= keysLeft + 0.5, qPrintable(u"title ends at %1, keys start at %2"_s.arg(titleRight).arg(keysLeft)));
        QVERIFY(title->width() >= 80);
    }

    void longProjectNameDoesNotOverlapPicker() {
        launch();
        window->resize(1366, 768);
        services->theme().setScale(1.5);
        QVERIFY(services->project().setProjectName(u"Paris trip vlog — episode 3 final cut, colour pass two"_s));
        QQuickItem* name = item(u"header.projectName"_s);
        QQuickItem* picker = item(u"header.workspacePicker"_s);
        QVERIFY(name && picker);
        QTest::qWait(50);
        const qreal nameRight = name->mapToItem(nullptr, QPointF(name->width(), 0)).x();
        const qreal pickerLeft = picker->mapToItem(nullptr, QPointF(0, 0)).x();
        QVERIFY2(nameRight <= pickerLeft, qPrintable(u"name ends at %1, picker starts at %2"_s.arg(nameRight).arg(pickerLeft)));
    }

    void themeWarningShownInStatusBar() {
        launch();
        services->theme().setMode(u"custom:Missing Theme"_s);
        QQuickItem* message = item(u"status.message"_s);
        QVERIFY(message);
        QTRY_VERIFY(message->property("text").toString().contains(u"couldn't be loaded"_s));
    }

    void customResolutionShowsSizeFields() {
        launch();
        QQuickItem* combo = item(u"inspector.resolution"_s);
        QVERIFY(combo);
        QQuickItem* custom = item(u"inspector.customSize"_s);
        QVERIFY(custom);
        QVERIFY(!custom->isVisible());
        QMetaObject::invokeMethod(combo, "activated", Q_ARG(int, 6));
        QTRY_VERIFY(custom->isVisible());
        QCOMPARE(services->project().document().project().sequences.front().format.width, 1920);
    }

    void failedSaveAsClearsPendingAction() {
        launch();
        services->project().addTrack(u"video"_s); // 5 tracks, unsaved
        // New Project → "Save" on an untitled project leaves "then start a new project" pending a Save As.
        services->actions().action(u"file.new"_s)->trigger();
        QObject* dialog = object(u"dialog.saveChanges"_s);
        QTRY_VERIFY(dialog->property("visible").toBool());
        QMetaObject::invokeMethod(dialog, "choose", Q_ARG(QVariant, u"save"_s));
        QTRY_VERIFY(window->property("afterSave").value<QJSValue>().isCallable());
        // The Save As fails (missing folder): the pending action must be dropped...
        const QUrl bad = QUrl::fromLocalFile(dir.filePath(u"no/such/folder/x.cutline"_s));
        QMetaObject::invokeMethod(window, "finishSaveAs", Q_ARG(QVariant, QVariant(bad)));
        QVERIFY(!window->property("afterSave").value<QJSValue>().isCallable());
        // ...so a later, unrelated Save As doesn't suddenly replace the project.
        QMetaObject::invokeMethod(window, "finishSaveAs",
                                  Q_ARG(QVariant, QVariant(QUrl::fromLocalFile(dir.filePath(u"ok.cutline"_s)))));
        QCOMPARE(services->project().tracks()->rowCount(), 5);
    }

    void trackRenameCommitsOnBlur() {
        launch();
        QQuickItem* name = item(u"track.V1.name"_s);
        QVERIFY(name);
        QMetaObject::invokeMethod(name, "startEditing");
        QQuickItem* field = nullptr;
        QTRY_VERIFY((field = item(u"track.V1.editor"_s)) && field->hasActiveFocus());
        field->setProperty("text", QString());
        for (char c : QByteArray("Blur"))
            QTest::keyClick(window, c);
        focusPanel(u"viewer"_s);
        const auto* tracks = services->project().tracks();
        QTRY_COMPARE(tracks->data(tracks->index(1, 0), tracks->roleNames().key("name")).toString(), u"Blur"_s);
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
