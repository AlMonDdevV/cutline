#include "app/controllers/RecentProjects.h"

#include "app/controllers/AppSettings.h"

#include <QFileInfo>

namespace cutline {
namespace {

const QString kKey = QStringLiteral("recent/projects");

Qt::CaseSensitivity pathCase() {
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    return Qt::CaseInsensitive;
#else
    return Qt::CaseSensitive;
#endif
}

} // namespace

RecentProjects::RecentProjects(AppSettings& settings, QObject* parent) : QObject(parent), m_settings(settings) {}

void RecentProjects::store(const QStringList& paths) const { m_settings.raw().setValue(kKey, paths); }

void RecentProjects::add(const QString& path) {
    const QString absolute = QFileInfo(path).absoluteFilePath();
    QStringList list = m_settings.raw().value(kKey).toStringList();
    list.removeIf([&](const QString& p) { return p.compare(absolute, pathCase()) == 0; });
    list.prepend(absolute);
    while (list.size() > 10)
        list.removeLast();
    store(list);
    emit changed();
}

QStringList RecentProjects::paths() const {
    const QStringList stored = m_settings.raw().value(kKey).toStringList();
    QStringList existing;
    for (const QString& p : stored)
        if (QFileInfo::exists(p))
            existing << p;
    if (existing.size() != stored.size())
        store(existing);
    return existing;
}

QString RecentProjects::displayName(const QString& path) const { return QFileInfo(path).completeBaseName(); }

} // namespace cutline
