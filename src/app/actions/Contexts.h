#pragma once

#include <QFlags>
#include <QString>
#include <QStringList>

namespace cutline {

// Where an action's shortcut works. An empty set means everywhere (global).
enum class Context : quint32 {
    Timeline = 1u << 0,
    Viewer = 1u << 1,
    Media = 1u << 2,
    Inspector = 1u << 3,
    Effects = 1u << 4,
    Mixer = 1u << 5,
    History = 1u << 6,
    Export = 1u << 7,
    Claude = 1u << 8,
};
Q_DECLARE_FLAGS(Contexts, Context)
Q_DECLARE_OPERATORS_FOR_FLAGS(Contexts)

// Two actions can clash only when their contexts overlap; global overlaps everything.
inline bool contextsOverlap(Contexts a, Contexts b) { return a == Contexts() || b == Contexts() || (a & b) != Contexts(); }

inline QString contextsText(Contexts c) {
    if (c == Contexts())
        return QStringLiteral("Everywhere");
    static const std::pair<Context, const char*> names[] = {
        {Context::Timeline, "Timeline"}, {Context::Viewer, "Viewer"}, {Context::Media, "Media"},
        {Context::Inspector, "Inspector"}, {Context::Effects, "Effects"}, {Context::Mixer, "Mixer"},
        {Context::History, "History"}, {Context::Export, "Export"}, {Context::Claude, "Claude"},
    };
    QStringList parts;
    for (const auto& [flag, name] : names)
        if (c.testFlag(flag))
            parts << QLatin1String(name);
    return parts.join(QStringLiteral(", "));
}

} // namespace cutline
