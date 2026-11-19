#pragma once

#include "app/theme/Palette.h"

#include <QString>
#include <QStringList>

namespace cutline {

// The user's custom themes: one palette JSON per theme in a folder (<config>/themes/<name>.json).
class ThemeLibrary {
public:
    explicit ThemeLibrary(QString directory);

    QString directory() const { return m_directory; }
    QStringList customNames() const;
    Result<Palette> load(const QString& name) const;

    // Validates the file, refuses names that clash, copies it in, and returns readability warnings.
    Result<ThemeCheck> importFrom(const QString& path);
    Status exportTo(const Palette& palette, const QString& path) const;
    Status remove(const QString& name);

    // Names a custom theme may never take.
    static const QStringList& reservedNames();

    QString filePath(const QString& name) const;

private:
    QString m_directory;
};

} // namespace cutline
