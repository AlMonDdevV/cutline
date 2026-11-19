#include "app/layout/WorkspaceLayout.h"

#include "app/layout/PanelRegistry.h"

#include <QJsonArray>

#include <numeric>

namespace cutline {
namespace {

QJsonArray toArray(const QList<double>& v) {
    QJsonArray a;
    for (double d : v)
        a.append(d);
    return a;
}

QList<double> fromArray(const QJsonValue& value, const QList<double>& fallback) {
    const QJsonArray a = value.toArray();
    if (a.size() != fallback.size())
        return fallback;
    QList<double> out;
    for (const QJsonValue& v : a) {
        if (!v.isDouble() || v.toDouble() <= 0)
            return fallback;
        out << v.toDouble();
    }
    return normalizedSizes(out);
}

} // namespace

WorkspaceLayout builtinEditLayout() {
    WorkspaceLayout l;
    l.name = QStringLiteral("Edit");
    l.areas[int(Area::Viewer)] = {{QStringLiteral("viewer")}, QStringLiteral("viewer")};
    l.areas[int(Area::Right)] = {{QStringLiteral("inspector"), QStringLiteral("history")}, QStringLiteral("inspector")};
    l.areas[int(Area::Timeline)] = {{QStringLiteral("timeline")}, QStringLiteral("timeline")};
    return l;
}

QList<double> normalizedSizes(QList<double> sizes, double minimum) {
    if (sizes.isEmpty())
        return sizes;
    const int n = int(sizes.size());
    if (minimum * n > 1.0)
        minimum = 1.0 / n;
    for (double& s : sizes)
        s = std::max(s, 0.0);
    double total = std::accumulate(sizes.begin(), sizes.end(), 0.0);
    if (total <= 0) {
        sizes.fill(1.0 / n);
        return sizes;
    }
    for (double& s : sizes)
        s /= total;

    // Pin anything under the minimum and share the rest among the others, until nothing is under.
    QList<bool> pinned(n, false);
    for (int round = 0; round < n; ++round) {
        bool changed = false;
        for (int i = 0; i < n; ++i)
            if (!pinned[i] && sizes[i] < minimum) {
                pinned[i] = true;
                changed = true;
            }
        if (!changed)
            break;
        const int pinnedCount = int(std::count(pinned.begin(), pinned.end(), true));
        const double freeShare = 1.0 - minimum * pinnedCount;
        double freeTotal = 0;
        for (int i = 0; i < n; ++i)
            if (!pinned[i])
                freeTotal += sizes[i];
        for (int i = 0; i < n; ++i)
            sizes[i] = pinned[i] ? minimum : (freeTotal > 0 ? sizes[i] / freeTotal * freeShare : freeShare / (n - pinnedCount));
    }
    return sizes;
}

QJsonObject toJson(const WorkspaceLayout& layout) {
    QJsonArray areas;
    for (const AreaState& a : layout.areas)
        areas.append(QJsonObject{{QStringLiteral("panels"), QJsonArray::fromStringList(a.panels)},
                                 {QStringLiteral("current"), a.current}});
    return QJsonObject{
        {QStringLiteral("name"), layout.name},
        {QStringLiteral("areas"), areas},
        {QStringLiteral("topSizes"), toArray(layout.topSizes)},
        {QStringLiteral("verticalSizes"), toArray(layout.verticalSizes)},
        {QStringLiteral("bottomSizes"), toArray(layout.bottomSizes)},
    };
}

Result<WorkspaceLayout> workspaceFromJson(const QJsonObject& json, const PanelRegistry& panels) {
    const QString name = json.value(QLatin1String("name")).toString().trimmed();
    const QJsonArray areas = json.value(QLatin1String("areas")).toArray();
    if (name.isEmpty() || areas.size() != kAreaCount)
        return Error{Error::Code::Corrupt, QStringLiteral("This isn't a CUTLINE workspace.")};

    WorkspaceLayout l;
    l.name = name;
    QStringList seen;
    for (int i = 0; i < kAreaCount; ++i) {
        const QJsonObject area = areas.at(i).toObject();
        AreaState& state = l.areas[size_t(i)];
        for (const QJsonValue& v : area.value(QLatin1String("panels")).toArray()) {
            const QString id = v.toString();
            if (panels.find(id) && !seen.contains(id)) {
                state.panels << id;
                seen << id;
            }
        }
        const QString current = area.value(QLatin1String("current")).toString();
        state.current = state.panels.contains(current) ? current : (state.panels.isEmpty() ? QString() : state.panels.first());
    }
    const WorkspaceLayout defaults;
    l.topSizes = fromArray(json.value(QLatin1String("topSizes")), defaults.topSizes);
    l.verticalSizes = fromArray(json.value(QLatin1String("verticalSizes")), defaults.verticalSizes);
    l.bottomSizes = fromArray(json.value(QLatin1String("bottomSizes")), defaults.bottomSizes);
    return l;
}

} // namespace cutline
