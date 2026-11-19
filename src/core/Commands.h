#pragma once

#include "core/Command.h"

#include <memory>

namespace cutline {

std::unique_ptr<Command> makeRenameProject(QString name);
std::unique_ptr<Command> makeSetSequenceSettings(QString sequenceId, SequenceSettings settings);
// Appended after the last track of its kind (a new video track becomes the top layer).
std::unique_ptr<Command> makeAddTrack(QString sequenceId, TrackKind kind, QString newTrackId = newId());
std::unique_ptr<Command> makeRemoveTrack(QString sequenceId, QString trackId);
std::unique_ptr<Command> makeSetTrackName(QString sequenceId, QString trackId, QString name);
std::unique_ptr<Command> makeSetTrackLocked(QString sequenceId, QString trackId, bool locked);
std::unique_ptr<Command> makeSetTrackEnabled(QString sequenceId, QString trackId, bool enabled);
std::unique_ptr<Command> makeSetTrackSolo(QString sequenceId, QString trackId, bool solo);

} // namespace cutline
