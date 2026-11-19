#pragma once

#include <QString>
#include <QStringView>

#include <cstdint>
#include <optional>

namespace cutline {

// All positions are integer ticks. 1 tick = 1/705,600,000 s (a "flick"): every common frame
// duration (incl. 1001-based rates) and every 44.1/48 kHz sample is a whole number of ticks.
using Ticks = std::int64_t;
inline constexpr Ticks kTicksPerSecond = 705'600'000;

struct FrameRate {
    std::int32_t num = 30;
    std::int32_t den = 1;

    bool isValid() const { return num > 0 && den > 0; }
    double fps() const { return isValid() ? double(num) / double(den) : 0.0; }
    friend bool operator==(FrameRate, FrameRate) = default;
};

// Start time of `frames` frames; rounds to the nearest tick when the rate is not exact.
Ticks framesToTicks(std::int64_t frames, FrameRate rate);
// Index of the frame showing at `ticks` (floor).
std::int64_t ticksToFrames(Ticks ticks, FrameRate rate);
Ticks samplesToTicks(std::int64_t samples, int sampleRate);

// Timecode frame base: 24000/1001 -> 24, 30000/1001 -> 30.
int nominalFps(FrameRate rate);
// Drop-frame timecode exists only for 29.97 (30000/1001) and 59.94 (60000/1001).
bool supportsDropFrame(FrameRate rate);

enum class TimecodeMode { NonDrop, Drop };

// "HH:MM:SS:FF", or "HH:MM:SS;FF" for drop-frame. Drop on a rate without drop-frame falls back to NonDrop.
// `startHour` offsets the hours field (sequences may start at 01:00:00:00).
QString formatTimecode(std::int64_t frame, FrameRate rate, TimecodeMode mode, int startHour = 0);
// Inverse of formatTimecode. Accepts ':' or ';' before the frames field. Returns nullopt for
// malformed text, out-of-range fields, labels that drop-frame skips, or times before startHour.
std::optional<std::int64_t> parseTimecode(QStringView text, FrameRate rate, TimecodeMode mode, int startHour = 0);

// Index of the first non-zero digit (characters before it are drawn dimmed); the last index if all zero.
qsizetype timecodeSignificantIndex(QStringView timecode);

} // namespace cutline
