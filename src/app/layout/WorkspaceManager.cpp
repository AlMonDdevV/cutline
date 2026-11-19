#include "app/layout/WorkspaceManager.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace cutline {
namespace {

const QString kDataKey = QStringLiteral("workspaces/data");
const QString kCurrentKey = QStringLiteral("workspaces/current");
const QString kEdit = QStringLiteral("Edit");

bool validArea(int area) { return area >= 0 && area < kAreaCount; }

} // namespace

WorkspaceManager::WorkspaceManager(const PanelRegistry& panels, QSettings& settings, QObject* parent)
    : QObject(parent), m_panels(panels), m_settings(settings) {
    m_persistTimer.setSingleShot(true);
    m_persistTimer.setInterval(500);
    connect(&m_persistTimer, &QTimer::timeout, this, &WorkspaceManager::persistNow);
    load();
}

WorkspaceManager::~WorkspaceManager() {
    if (m_persistTimer.isActive())
        persistNow();
}

void WorkspaceManager::load() {
    const WorkspaceLayout edit = builtinEditLayout();
    m_entries = {Entry{edit, edit}};
    m_current = 0;

    const QString raw = m_settings.value(kDataKey).toString();
    if (!raw.isEmpty()) {
        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(raw.toUtf8(), &error);
        bool ok = error.error == QJsonParseError::NoError && doc.isArray();
        QList<Entry> loaded;
        if (ok) {
            for (const QJsonValue& v : doc.array()) {
                auto saved = workspaceFromJson(v.toObject().value(QLatin1String("saved")).toObject(), m_panels);
                auto liveLayout = workspaceFromJson(v.toObject().value(QLatin1String("live")).toObject(), m_panels);
                if (!saved.ok() || !liveLayout.ok()) {
                    ok = false;
                    break;
                }
                loaded << Entry{saved.value(), liveLayout.value()};
            }
        }
        if (!ok) {
            m_loadWarning = QStringLiteral("Your saved workspaces couldn't be read, so the default layout is in use.");
        } else {
            for (const Entry& e : loaded) {
                if (e.live.name.compare(kEdit, Qt::CaseInsensitive) == 0)
                    m_entries[0].live = e.live; // Edit's saved state is always the built-in layout
                else if (indexOf(e.live.name) < 0)
                    m_entries << e;
            }
            const int current = indexOf(m_settings.value(kCurrentKey).toString());
            m_current = current < 0 ? 0 : current;
        }
    }
}

void WorkspaceManager::persistNow() {
    m_persistTimer.stop();
    QJsonArray data;
    for (const Entry& e : m_entries)
        data.append(QJsonObject{{QStringLiteral("saved"), toJson(e.saved)}, {QStringLiteral("live"), toJson(e.live)}});
    m_settings.setValue(kDataKey, QString::fromUtf8(QJsonDocument(data).toJson(QJsonDocument::Compact)));
    m_settings.setValue(kCurrentKey, current());
    m_settings.sync();
}

void WorkspaceManager::changed(bool structural) {
    ++m_revision;
    emit layoutChanged();
    if (structural)
        persistNow();
    else
        m_persistTimer.start();
}

int WorkspaceManager::indexOf(const QString& name) const {
    for (int i = 0; i < m_entries.size(); ++i)
        if (m_entries[i].live.name.compare(name, Qt::CaseInsensitive) == 0)
            return i;
    return -1;
}

QStringList WorkspaceManager::names() const {
    QStringList out;
    for (const Entry& e : m_entries)
        out << e.live.name;
    return out;
}

QStringList WorkspaceManager::panelsIn(int area) const {
    return validArea(area) ? layout().areas[size_t(area)].panels : QStringList();
}

QString WorkspaceManager::currentIn(int area) const {
    return validArea(area) ? layout().areas[size_t(area)].current : QString();
}

void WorkspaceManager::setCurrentIn(int area, const QString& panelId) {
    if (!validArea(area) || !panelsIn(area).contains(panelId) || currentIn(area) == panelId)
        return;
    live().areas[size_t(area)].current = panelId;
    changed();
}

bool WorkspaceManager::isAreaVisible(int area) const { return validArea(area) && !panelsIn(area).isEmpty(); }

int WorkspaceManager::areaOf(const QString& id) const {
    for (int i = 0; i < kAreaCount; ++i)
        if (layout().areas[size_t(i)].panels.contains(id))
            return i;
    return -1;
}

bool WorkspaceManager::isPanelVisible(const QString& id) const { return areaOf(id) >= 0; }

void WorkspaceManager::showPanel(const QString& id) {
    const PanelInfo* info = m_panels.find(id);
    if (!info)
        return;
    int area = areaOf(id);
    if (area < 0) {
        area = int(info->defaultArea);
        live().areas[size_t(area)].panels << id;
    }
    live().areas[size_t(area)].current = id;
    changed();
}

void WorkspaceManager::hidePanel(const QString& id) {
    const int area = areaOf(id);
    if (area < 0)
        return;
    AreaState& state = live().areas[size_t(area)];
    state.panels.removeAll(id);
    if (state.current == id)
        state.current = state.panels.isEmpty() ? QString() : state.panels.first();
    changed();
}

void WorkspaceManager::togglePanel(const QString& id) { isPanelVisible(id) ? hidePanel(id) : showPanel(id); }

void WorkspaceManager::movePanel(const QString& id, int area) {
    if (!validArea(area) || !m_panels.find(id))
        return;
    const int from = areaOf(id);
    if (from == area)
        return;
    if (from >= 0) {
        AreaState& old = live().areas[size_t(from)];
        old.panels.removeAll(id);
        if (old.current == id)
            old.current = old.panels.isEmpty() ? QString() : old.panels.first();
    }
    live().areas[size_t(area)].panels << id;
    live().areas[size_t(area)].current = id;
    changed();
}

QString WorkspaceManager::panelTitle(const QString& id) const {
    const PanelInfo* info = m_panels.find(id);
    return info ? info->title : QString();
}

QString WorkspaceManager::panelComponent(const QString& id) const {
    const PanelInfo* info = m_panels.find(id);
    return info ? info->component : QString();
}

QList<double>* WorkspaceManager::sizesFor(const QString& which) {
    if (which == QLatin1String("top"))
        return &live().topSizes;
    if (which == QLatin1String("vertical"))
        return &live().verticalSizes;
    if (which == QLatin1String("bottom"))
        return &live().bottomSizes;
    return nullptr;
}

QVariantList WorkspaceManager::sizes(const QString& which) const {
    QVariantList out;
    if (auto* s = const_cast<WorkspaceManager*>(this)->sizesFor(which))
        for (double d : *s)
            out << d;
    return out;
}

void WorkspaceManager::setSizes(const QString& which, const QVariantList& fractions) {
    QList<double>* target = sizesFor(which);
    if (!target || fractions.size() != target->size())
        return;
    QList<double> values;
    for (const QVariant& v : fractions)
        values << v.toDouble();
    const QList<double> normalized = normalizedSizes(values);
    if (normalized == *target)
        return;
    *target = normalized;
    changed();
}

bool WorkspaceManager::saveAs(const QString& name) {
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        m_lastError = QStringLiteral("Workspace names can't be empty.");
        return false;
    }
    if (trimmed.size() > 40) {
        m_lastError = QStringLiteral("Workspace names can be at most 40 characters.");
        return false;
    }
    if (indexOf(trimmed) >= 0) {
        m_lastError = QStringLiteral("There's already a workspace called “%1”.").arg(trimmed);
        return false;
    }
    WorkspaceLayout copy = layout();
    copy.name = trimmed;
    m_entries << Entry{copy, copy};
    m_current = int(m_entries.size()) - 1;
    m_lastError.clear();
    emit namesChanged();
    emit currentChanged();
    changed(true);
    return true;
}

bool WorkspaceManager::switchTo(const QString& name) {
    const int index = indexOf(name);
    if (index < 0) {
        m_lastError = QStringLiteral("There's no workspace called “%1”.").arg(name);
        return false;
    }
    if (index != m_current) {
        m_current = index;
        emit currentChanged();
        changed(true);
    }
    return true;
}

void WorkspaceManager::resetCurrent() {
    m_entries[m_current].live = m_entries[m_current].saved;
    changed(true);
}

bool WorkspaceManager::remove(const QString& name) {
    const int index = indexOf(name);
    if (index == 0) {
        m_lastError = QStringLiteral("The Edit workspace can't be removed.");
        return false;
    }
    if (index < 0) {
        m_lastError = QStringLiteral("There's no workspace called “%1”.").arg(name);
        return false;
    }
    const bool wasCurrent = index == m_current;
    m_entries.removeAt(index);
    if (wasCurrent)
        m_current = 0;
    else if (m_current > index)
        --m_current;
    emit namesChanged();
    if (wasCurrent)
        emit currentChanged();
    changed(true);
    return true;
}

QStringList WorkspaceManager::focusOrder() const {
    QStringList out;
    for (int i = 0; i < kAreaCount; ++i)
        if (isAreaVisible(i))
            out << currentIn(i);
    return out;
}

QString WorkspaceManager::nextPanel(const QString& from, int step) const {
    const QStringList order = focusOrder();
    if (order.isEmpty())
        return {};
    const int index = int(order.indexOf(from));
    if (index < 0)
        return order.first();
    const int n = int(order.size());
    return order.at(((index + step) % n + n) % n);
}

} // namespace cutline
