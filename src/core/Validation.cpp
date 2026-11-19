#include "core/Validation.h"

namespace cutline {
namespace {

Status validateName(const QString& name, qsizetype maxLength, const char* noun) {
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty())
        return Status::failure(QStringLiteral("%1 names can't be empty.").arg(QLatin1String(noun)));
    if (trimmed.size() > maxLength)
        return Status::failure(
            QStringLiteral("%1 names can be at most %2 characters.").arg(QLatin1String(noun)).arg(maxLength));
    return Status::success();
}

bool validDimension(int v) { return v >= 16 && v <= 8192 && v % 2 == 0; }

} // namespace

Status validateProjectName(const QString& name) { return validateName(name, 120, "Project"); }
Status validateSequenceName(const QString& name) { return validateName(name, 120, "Sequence"); }
Status validateTrackName(const QString& name) { return validateName(name, 64, "Track"); }

Status validateSequenceFormat(const SequenceFormat& f) {
    if (!validDimension(f.width) || !validDimension(f.height))
        return Status::failure(QStringLiteral("Width and height must be even numbers between 16 and 8192."));
    if (!f.frameRate.isValid() || f.frameRate.fps() < 1.0 || f.frameRate.fps() > 240.0)
        return Status::failure(QStringLiteral("Frame rate must be between 1 and 240 fps."));
    if (f.sampleRate != 44100 && f.sampleRate != 48000 && f.sampleRate != 96000)
        return Status::failure(QStringLiteral("Sample rate must be 44.1, 48 or 96 kHz."));
    if (f.channels != 2)
        return Status::failure(QStringLiteral("Sequences are stereo in this version."));
    return Status::success();
}

Status validateSequenceSettings(const SequenceSettings& s) {
    if (Status name = validateSequenceName(s.name); !name)
        return name;
    if (Status format = validateSequenceFormat(s.format); !format)
        return format;
    if (s.timecodeStartHour != 0 && s.timecodeStartHour != 1)
        return Status::failure(QStringLiteral("Timecode can start at 00:00:00:00 or 01:00:00:00."));
    return Status::success();
}

} // namespace cutline
