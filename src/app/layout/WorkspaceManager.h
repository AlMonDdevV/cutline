#pragma once

#include "app/layout/PanelRegistry.h"
#include "app/layout/WorkspaceLayout.h"

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <QVariantList>

namespace cutline {

// Named workspaces (panel arrangement + splitter sizes). "Edit" is built in. Every workspace keeps a
// saved copy (restored by Reset) and a live copy that follows the user's changes and persists itself.
class WorkspaceManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString current READ current NOTIFY currentChanged)
    Q_PROPERTY(QStringList names READ names NOTIFY namesChanged)
    Q_PROPERTY(int revision READ revision NOTIFY layoutChanged)

public:
    WorkspaceManager(const PanelRegistry& panels, QSettings& settings, QObject* parent = nullptr);
    ~WorkspaceManager() override;

    const WorkspaceLayout& layout() const { return m_entries[m_current].live; }
    QString current() const { return layout().name; }
    QStringList names() const;
    int revision() const { return m_revision; }
    QString loadWarning() const { return m_loadWarning; }
    const PanelRegistry& panels() const { return m_panels; }

    Q_INVOKABLE QStringList panelsIn(int area) const;
    Q_INVOKABLE QString currentIn(int area) const;
    Q_INVOKABLE void setCurrentIn(int area, const QString& panelId);
    Q_INVOKABLE bool isAreaVisible(int area) const;
    Q_INVOKABLE bool isPanelVisible(const QString& id) const;
    Q_INVOKABLE int areaOf(const QString& id) const; // -1 when hidden
    Q_INVOKABLE void showPanel(const QString& id);
    Q_INVOKABLE void hidePanel(const QString& id);
    Q_INVOKABLE void togglePanel(const QString& id);
    Q_INVOKABLE void movePanel(const QString& id, int area);
    Q_INVOKABLE QString panelTitle(const QString& id) const;
    Q_INVOKABLE QString panelComponent(const QString& id) const;

    Q_INVOKABLE QVariantList sizes(const QString& which) const; // "top" | "vertical" | "bottom"
    Q_INVOKABLE void setSizes(const QString& which, const QVariantList& fractions);

    Q_INVOKABLE bool saveAs(const QString& name);
    Q_INVOKABLE bool switchTo(const QString& name);
    Q_INVOKABLE void resetCurrent();
    Q_INVOKABLE bool remove(const QString& name);
    Q_INVOKABLE QString lastError() const { return m_lastError; }

    Q_INVOKABLE QStringList focusOrder() const;
    Q_INVOKABLE QString nextPanel(const QString& from, int step) const;

    void persistNow();

signals:
    void currentChanged();
    void namesChanged();
    void layoutChanged();

private:
    struct Entry {
        WorkspaceLayout saved;
        WorkspaceLayout live;
    };

    WorkspaceLayout& live() { return m_entries[m_current].live; }
    void load();
    void changed(bool structural = false);
    QList<double>* sizesFor(const QString& which);
    int indexOf(const QString& name) const;

    const PanelRegistry& m_panels;
    QSettings& m_settings;
    QList<Entry> m_entries;
    int m_current = 0;
    int m_revision = 0;
    QString m_lastError;
    QString m_loadWarning;
    QTimer m_persistTimer;
};

} // namespace cutline
