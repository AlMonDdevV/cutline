#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtQml/qqmlextensionplugin.h>
#include <QtTest>

Q_IMPORT_QML_PLUGIN(CutlinePlugin)

using namespace Qt::StringLiterals;

class TestAppSmoke : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() { QQuickStyle::setStyle(u"Basic"_s); }

    void mainLoads() {
        QQmlApplicationEngine engine;
        engine.loadFromModule("Cutline", "Main");
        QCOMPARE(engine.rootObjects().size(), 1);
        auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        QVERIFY(window);
        QCOMPARE(window->title(), u"CUTLINE"_s);
    }
};

QTEST_MAIN(TestAppSmoke)
#include "tst_app_smoke.moc"
