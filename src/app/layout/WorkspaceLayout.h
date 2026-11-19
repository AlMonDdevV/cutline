#pragma once

#include "core/Result.h"

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

#include <array>

namespace cutline {

// The window is a fixed frame of five areas:
//   ┌ Left ┬ Viewer ┬ Right ┐   top row (topSizes)
//   ├──────┴───┬────┴───────┤   top/bottom (verticalSizes)
//   │ Timeline │ Side       │   bottom row (bottomSizes)
// Each area holds a tab group of panels; an empty area is hidden.
enum class Area { Left = 0, Viewer = 1, Right = 2, Timeline = 3, Side = 4 };
inline constexpr int kAreaCount = 5;

struct AreaState {
    QStringList panels;
    QString current;
};

struct WorkspaceLayout {
    QString name;
    std::array<AreaState, kAreaCount> areas;
    QList<double> topSizes{0.22, 0.50, 0.28};
    QList<double> verticalSizes{0.55, 0.45};
    QList<double> bottomSizes{0.78, 0.22};
};

class PanelRegistry;

WorkspaceLayout builtinEditLayout();
QJsonObject toJson(const WorkspaceLayout& layout);
// Unknown panel ids (e.g. from a newer build) are dropped; sizes are clamped and normalised.
Result<WorkspaceLayout> workspaceFromJson(const QJsonObject& json, const PanelRegistry& panels);

// Each fraction becomes at least `minimum` and together they sum to 1.
QList<double> normalizedSizes(QList<double> sizes, double minimum = 0.1);

} // namespace cutline
