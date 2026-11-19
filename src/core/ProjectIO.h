#pragma once

#include "core/Project.h"
#include "core/Result.h"

#include <QByteArray>
#include <QString>

namespace cutline::io {

inline constexpr int kProjectFormatVersion = 1;

// Indented JSON with sorted keys, so the same project always serializes to the same bytes.
QByteArray serializeProject(const Project& project);
// Every error message names the field (e.g. "sequences[0].format.width") and is shown to the user.
Result<Project> deserializeProject(const QByteArray& json);

// Atomic (QSaveFile): the old file stays intact if anything fails.
Status saveProject(const Project& project, const QString& path);
Result<Project> loadProject(const QString& path);

} // namespace cutline::io
