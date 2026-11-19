#pragma once

#include "core/Result.h"

#include <QByteArray>
#include <QHash>
#include <QKeySequence>
#include <QList>
#include <QString>

namespace cutline {

// The user's changes on top of the default shortcuts. An empty list means "no shortcut".
struct ShortcutProfile {
    QString name = QStringLiteral("Default");
    QHash<QString, QList<QKeySequence>> overrides;
};

// {"format":"cutline.shortcuts","version":1,"name":…,"base":"default","overrides":{id:["Ctrl+S", …]}}
QByteArray serializeShortcutProfile(const ShortcutProfile& profile);
Result<ShortcutProfile> parseShortcutProfile(const QByteArray& json);

// Keeps the active profile at <directory>/shortcuts.json.
class ShortcutStore {
public:
    explicit ShortcutStore(QString directory);

    // Missing file: defaults, no warning. Unreadable file: defaults plus a warning for the status bar.
    ShortcutProfile loadActive(QString* warning = nullptr) const;
    Status saveActive(const ShortcutProfile& profile) const;
    Status exportTo(const ShortcutProfile& profile, const QString& path) const;
    Result<ShortcutProfile> importFrom(const QString& path) const;

    QString activePath() const;

private:
    QString m_directory;
};

} // namespace cutline
