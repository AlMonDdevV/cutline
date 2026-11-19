#include "app/AppServices.h"
#include "app/actions/ShortcutRouter.h"
#include "app/controllers/ProjectController.h"
#include "app/icons/IconProvider.h"
#include "app/theme/Fonts.h"
#include "app/theme/ThemeController.h"
#include "core/Version.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QImage>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <QtQml/qqmlextensionplugin.h>

#include <cstdio>
#include <memory>

Q_IMPORT_QML_PLUGIN(CutlinePlugin)

namespace {

// Exit codes for --screenshot: 0 ok, 2 couldn't write the file, 3 couldn't open --project, 4 timed out.
void armScreenshot(QQuickWindow* window, const QString& path) {
    auto frames = std::make_shared<int>(0);
    auto done = std::make_shared<bool>(false);
    auto grab = [window, path, done] {
        if (*done)
            return;
        *done = true;
        const QImage image = window->grabWindow();
        const bool ok = !image.isNull() && image.save(path);
        if (!ok)
            std::fprintf(stderr, "cutline: couldn't write %s\n", qPrintable(path));
        QCoreApplication::exit(ok ? 0 : 2);
    };
    QObject::connect(window, &QQuickWindow::frameSwapped, window, [frames, grab] {
        if (++*frames == 3)
            QTimer::singleShot(50, grab);
    });
    QTimer::singleShot(2500, window, grab); // offscreen platforms may not report frame swaps
    QTimer::singleShot(20000, [] { QCoreApplication::exit(4); });
}

} // namespace

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("CUTLINE"));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(cutline::kVersion));
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    cutline::loadBundledFonts();

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("CUTLINE: video editing without the paywall."));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption screenshot(QStringLiteral("screenshot"), QStringLiteral("Render the window to a PNG and exit."),
                                        QStringLiteral("file.png"));
    const QCommandLineOption size(QStringLiteral("size"), QStringLiteral("Window size for --screenshot, e.g. 1600x900."),
                                  QStringLiteral("WxH"), QStringLiteral("1600x900"));
    const QCommandLineOption theme(QStringLiteral("theme"), QStringLiteral("dark, light or system."), QStringLiteral("mode"));
    const QCommandLineOption scale(QStringLiteral("scale"), QStringLiteral("Interface scale, 0.9 to 1.5."), QStringLiteral("x"));
    const QCommandLineOption project(QStringLiteral("project"), QStringLiteral("Project to open."), QStringLiteral("file.cutline"));
    parser.addOptions({screenshot, size, theme, scale, project});
    parser.addPositionalArgument(QStringLiteral("project"), QStringLiteral("A .cutline project to open."), QStringLiteral("[project]"));
    parser.process(app);

    const bool screenshotMode = parser.isSet(screenshot);
    // Screenshots never touch the user's real settings, recent list or shortcuts.
    std::unique_ptr<QTemporaryDir> scratchConfig;
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    if (screenshotMode) {
        scratchConfig = std::make_unique<QTemporaryDir>();
        configDir = scratchConfig->path();
    }

    cutline::AppServices services(configDir);
    if (parser.isSet(theme))
        services.theme().setMode(parser.value(theme));
    if (parser.isSet(scale))
        services.theme().setScale(parser.value(scale).toDouble());

    QString projectPath = parser.value(project);
    if (projectPath.isEmpty() && !parser.positionalArguments().isEmpty())
        projectPath = parser.positionalArguments().first();

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("icons"), new cutline::IconProvider);
    engine.setInitialProperties({{QStringLiteral("screenshotMode"), screenshotMode},
                                 {QStringLiteral("startupProject"), projectPath}});
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Cutline", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;

    auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
    services.router().attach(window);

    if (screenshotMode && window) {
        if (!projectPath.isEmpty() && services.project().filePath().isEmpty())
            return 3;
        const QStringList wh = parser.value(size).split(QLatin1Char('x'));
        if (wh.size() == 2)
            window->resize(wh[0].toInt(), wh[1].toInt());
        armScreenshot(window, parser.value(screenshot));
    }

    return QGuiApplication::exec();
}
