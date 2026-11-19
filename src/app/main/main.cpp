#include "app/icons/IconProvider.h"
#include "app/theme/Fonts.h"
#include "core/Version.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQml/qqmlextensionplugin.h>

Q_IMPORT_QML_PLUGIN(CutlinePlugin)

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("CUTLINE"));
    QCoreApplication::setApplicationVersion(QString::fromLatin1(cutline::kVersion));
    QQuickStyle::setStyle(QStringLiteral("Basic"));
    cutline::loadBundledFonts();

    QQmlApplicationEngine engine;
    engine.addImageProvider(QStringLiteral("icons"), new cutline::IconProvider);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Cutline", "Main");

    return QGuiApplication::exec();
}
