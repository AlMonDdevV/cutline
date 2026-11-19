#pragma once

#include "app/actions/Contexts.h"
#include "app/layout/WorkspaceLayout.h"

#include <QList>
#include <QSize>
#include <QString>

namespace cutline {

struct PanelInfo {
    QString id;          // "inspector"
    QString title;       // "Inspector"
    Area defaultArea = Area::Right;
    Context context = Context::Inspector;
    QString component;   // QML component in the Cutline module, e.g. "InspectorPanel"
    QSize minimumSize;
};

// Panels that exist in this build. A panel is registered only once it does real work.
class PanelRegistry {
public:
    void add(PanelInfo info);
    const PanelInfo* find(QStringView id) const;
    QList<PanelInfo> all() const { return m_panels; }

private:
    QList<PanelInfo> m_panels;
};

} // namespace cutline
