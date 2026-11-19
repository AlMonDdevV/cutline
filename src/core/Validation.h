#pragma once

#include "core/Project.h"
#include "core/Result.h"

namespace cutline {

// The same rules apply to commands and to files being opened; the messages are shown to the user as-is.
Status validateProjectName(const QString& name);
Status validateSequenceName(const QString& name);
Status validateTrackName(const QString& name);
Status validateSequenceFormat(const SequenceFormat& format);
Status validateSequenceSettings(const SequenceSettings& settings);

} // namespace cutline
