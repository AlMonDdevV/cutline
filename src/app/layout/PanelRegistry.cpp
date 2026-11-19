#include "app/layout/PanelRegistry.h"

namespace cutline {

void PanelRegistry::add(PanelInfo info) {
    Q_ASSERT_X(!find(info.id), "PanelRegistry::add", qPrintable(info.id));
    m_panels << std::move(info);
}

const PanelInfo* PanelRegistry::find(QStringView id) const {
    for (const PanelInfo& p : m_panels)
        if (p.id == id)
            return &p;
    return nullptr;
}

} // namespace cutline
