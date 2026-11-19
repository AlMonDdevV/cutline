#pragma once

#include "core/Time.h"

#include <QString>
#include <QStringView>

#include <vector>

namespace cutline {

struct SequenceFormat {
    int width = 1920;
    int height = 1080;
    FrameRate frameRate{30, 1};
    int sampleRate = 48000;
    int channels = 2;

    friend bool operator==(const SequenceFormat&, const SequenceFormat&) = default;
};

enum class TrackKind { Video, Audio };

struct Track {
    QString id;
    TrackKind kind = TrackKind::Video;
    QString name;
    bool locked = false;
    bool enabled = true; // video: visible in the output; audio: not muted
    bool solo = false;   // audio only

    friend bool operator==(const Track&, const Track&) = default;
};

// Tracks are stored video V1..Vn (V1 = bottom layer) followed by audio A1..An.
struct Sequence {
    QString id;
    QString name;
    SequenceFormat format;
    int timecodeStartHour = 0;
    std::vector<Track> tracks;
};

struct SequenceSettings {
    QString name;
    SequenceFormat format;
    int timecodeStartHour = 0;
};

struct Project {
    QString id;
    QString name;
    std::vector<Sequence> sequences;
    QString defaultSequenceId;
};

QString newId();
// "Untitled" with one "Sequence 1" (1920×1080, 30 fps, 48 kHz stereo) and tracks V1 V2 A1 A2.
Project makeDefaultProject();

Sequence* findSequence(Project& project, QStringView id);
const Sequence* findSequence(const Project& project, QStringView id);
Track* findTrack(Sequence& sequence, QStringView id);
const Track* findTrack(const Sequence& sequence, QStringView id);

// "V2", "A1": 1-based position among tracks of the same kind. Empty if the id is unknown.
QString trackCode(const Sequence& sequence, QStringView trackId);
int trackCount(const Sequence& sequence, TrackKind kind);

SequenceSettings settingsOf(const Sequence& sequence);
void applySettings(Sequence& sequence, const SequenceSettings& settings);

} // namespace cutline
