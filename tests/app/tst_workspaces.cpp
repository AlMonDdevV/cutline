#include "app/layout/PanelRegistry.h"
#include "app/layout/WorkspaceLayout.h"
#include "app/layout/WorkspaceManager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

PanelRegistry panels() {
    PanelRegistry r;
    r.add({u"viewer"_s, u"Viewer"_s, Area::Viewer, Context::Viewer, u"ViewerPanel"_s, {320, 180}});
    r.add({u"inspector"_s, u"Inspector"_s, Area::Right, Context::Inspector, u"InspectorPanel"_s, {240, 200}});
    r.add({u"history"_s, u"History"_s, Area::Right, Context::History, u"HistoryPanel"_s, {200, 120}});
    r.add({u"timeline"_s, u"Timeline"_s, Area::Timeline, Context::Timeline, u"TimelinePanel"_s, {360, 160}});
    return r;
}

double sum(const QVariantList& v) {
    double s = 0;
    for (const QVariant& x : v)
        s += x.toDouble();
    return s;
}

} // namespace

class TestWorkspaces : public QObject {
    Q_OBJECT

    QTemporaryDir dir;
    PanelRegistry registry = panels();
    std::unique_ptr<QSettings> settings;

private slots:
    void init() {
        settings = std::make_unique<QSettings>(dir.filePath(u"test-%1.ini"_s.arg(QTest::currentTestFunction())),
                                               QSettings::IniFormat);
        settings->clear();
    }

    void builtinEdit() {
        const WorkspaceLayout l = builtinEditLayout();
        QCOMPARE(l.name, u"Edit"_s);
        QCOMPARE(l.areas[int(Area::Viewer)].panels, QStringList{u"viewer"_s});
        QCOMPARE(l.areas[int(Area::Right)].panels, (QStringList{u"inspector"_s, u"history"_s}));
        QCOMPARE(l.areas[int(Area::Right)].current, u"inspector"_s);
        QCOMPARE(l.areas[int(Area::Timeline)].panels, QStringList{u"timeline"_s});
        QVERIFY(l.areas[int(Area::Left)].panels.isEmpty());
        QVERIFY(l.areas[int(Area::Side)].panels.isEmpty());
        QCOMPARE(l.topSizes, (QList<double>{0.22, 0.50, 0.28}));
        QCOMPARE(l.verticalSizes, (QList<double>{0.55, 0.45}));
        QCOMPARE(l.bottomSizes, (QList<double>{0.78, 0.22}));
    }

    void jsonRoundTrip() {
        WorkspaceLayout l = builtinEditLayout();
        l.name = u"Review"_s;
        l.areas[int(Area::Side)] = {{u"history"_s}, u"history"_s};
        l.areas[int(Area::Right)] = {{u"inspector"_s}, u"inspector"_s};
        const auto back = workspaceFromJson(toJson(l), registry);
        QVERIFY(back.ok());
        QCOMPARE(toJson(back.value()), toJson(l));
    }

    void hideLastPanelHidesArea() {
        WorkspaceManager m(registry, *settings);
        QVERIFY(m.isAreaVisible(int(Area::Viewer)));
        m.hidePanel(u"viewer"_s);
        QVERIFY(!m.isAreaVisible(int(Area::Viewer)));
        QVERIFY(!m.isPanelVisible(u"viewer"_s));
    }

    void showReturnsToDefaultArea() {
        WorkspaceManager m(registry, *settings);
        m.movePanel(u"history"_s, int(Area::Side));
        m.hidePanel(u"history"_s);
        m.showPanel(u"history"_s);
        QVERIFY(m.panelsIn(int(Area::Right)).contains(u"history"_s));
        QCOMPARE(m.currentIn(int(Area::Right)), u"history"_s);
    }

    void movePanel() {
        WorkspaceManager m(registry, *settings);
        m.movePanel(u"history"_s, int(Area::Side));
        QCOMPARE(m.panelsIn(int(Area::Side)), QStringList{u"history"_s});
        QCOMPARE(m.panelsIn(int(Area::Right)), QStringList{u"inspector"_s});
        QCOMPARE(m.currentIn(int(Area::Right)), u"inspector"_s);
    }

    void sizesClampedAndNormalized() {
        WorkspaceManager m(registry, *settings);
        m.setSizes(u"top"_s, {0.01, 0.98, 0.01});
        const QVariantList s = m.sizes(u"top"_s);
        QCOMPARE(s.size(), 3);
        for (const QVariant& v : s)
            QVERIFY2(v.toDouble() >= 0.1 - 1e-9, qPrintable(QString::number(v.toDouble())));
        QVERIFY(qAbs(sum(s) - 1.0) < 1e-9);
        m.setSizes(u"top"_s, {0.5, 0.5}); // wrong count: ignored
        QCOMPARE(m.sizes(u"top"_s), s);
    }

    void saveSwitchPersist() {
        {
            WorkspaceManager m(registry, *settings);
            m.movePanel(u"history"_s, int(Area::Side));
            QVERIFY(m.saveAs(u"Review"_s));
            QCOMPARE(m.names(), (QStringList{u"Edit"_s, u"Review"_s}));
            QCOMPARE(m.current(), u"Review"_s);
        }
        WorkspaceManager again(registry, *settings);
        QCOMPARE(again.current(), u"Review"_s);
        QCOMPARE(again.panelsIn(int(Area::Side)), QStringList{u"history"_s});
        QVERIFY(again.switchTo(u"Edit"_s));
        QCOMPARE(again.current(), u"Edit"_s);
    }

    void nameValidation_data() {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("error");
        QTest::newRow("empty") << u"   "_s << u"Workspace names can't be empty."_s;
        QTest::newRow("long") << QString(41, u'w') << u"Workspace names can be at most 40 characters."_s;
        QTest::newRow("edit") << u"edit"_s << u"There's already a workspace called “edit”."_s;
    }
    void nameValidation() {
        QFETCH(QString, name);
        QFETCH(QString, error);
        WorkspaceManager m(registry, *settings);
        QVERIFY(!m.saveAs(name));
        QCOMPARE(m.lastError(), error);
    }

    void duplicateNameRejected() {
        WorkspaceManager m(registry, *settings);
        QVERIFY(m.saveAs(u"Review"_s));
        QVERIFY(!m.saveAs(u"REVIEW"_s));
        QCOMPARE(m.lastError(), u"There's already a workspace called “REVIEW”."_s);
    }

    void cannotRemoveEdit() {
        WorkspaceManager m(registry, *settings);
        QVERIFY(!m.remove(u"Edit"_s));
        QCOMPARE(m.lastError(), u"The Edit workspace can't be removed."_s);
        QVERIFY(m.saveAs(u"Temp"_s));
        QVERIFY(m.remove(u"Temp"_s));
        QCOMPARE(m.current(), u"Edit"_s);
    }

    void resetRestoresSaved() {
        WorkspaceManager m(registry, *settings);
        QVERIFY(m.saveAs(u"Review"_s));
        m.hidePanel(u"timeline"_s);
        m.resetCurrent();
        QVERIFY(m.isPanelVisible(u"timeline"_s));
        QVERIFY(m.switchTo(u"Edit"_s));
        m.movePanel(u"inspector"_s, int(Area::Left));
        m.resetCurrent();
        QCOMPARE(toJson(m.layout()), toJson(builtinEditLayout()));
    }

    void corruptStoredWorkspaceFallsBack() {
        settings->setValue(u"workspaces/data"_s, u"{bad"_s);
        settings->setValue(u"workspaces/current"_s, u"Review"_s);
        WorkspaceManager m(registry, *settings);
        QCOMPARE(m.current(), u"Edit"_s);
        QCOMPARE(m.loadWarning(), u"Your saved workspaces couldn't be read, so the default layout is in use."_s);
        QCOMPARE(toJson(m.layout()), toJson(builtinEditLayout()));
    }

    void unknownPanelDropped() {
        QJsonObject json = toJson(builtinEditLayout());
        QJsonArray areas = json.value(u"areas").toArray();
        QJsonObject side = areas.at(int(Area::Side)).toObject();
        side.insert(u"panels"_s, QJsonArray{u"mixer"_s});
        side.insert(u"current"_s, u"mixer"_s);
        areas.replace(int(Area::Side), side);
        json.insert(u"areas"_s, areas);
        const auto r = workspaceFromJson(json, registry);
        QVERIFY(r.ok());
        QVERIFY(r.value().areas[int(Area::Side)].panels.isEmpty());
        QVERIFY(r.value().areas[int(Area::Side)].current.isEmpty());
    }

    void focusOrderAndNext() {
        WorkspaceManager m(registry, *settings);
        QCOMPARE(m.focusOrder(), (QStringList{u"viewer"_s, u"inspector"_s, u"timeline"_s}));
        QCOMPARE(m.nextPanel(u"timeline"_s, 1), u"viewer"_s);
        QCOMPARE(m.nextPanel(u"viewer"_s, -1), u"timeline"_s);
        QCOMPARE(m.nextPanel(u"unknown"_s, 1), u"viewer"_s);
    }
};

QTEST_MAIN(TestWorkspaces)
#include "tst_workspaces.moc"
