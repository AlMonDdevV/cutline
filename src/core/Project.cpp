#include "core/Project.h"

#include <QUuid>

#include <algorithm>

namespace cutline {

QString newId() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

Project makeDefaultProject() {
    Project project;
    project.id = newId();
    project.name = QStringLiteral("Untitled");

    Sequence sequence;
    sequence.id = newId();
    sequence.name = QStringLiteral("Sequence 1");
    sequence.tracks = {
        {newId(), TrackKind::Video, QStringLiteral("Video 1")},
        {newId(), TrackKind::Video, QStringLiteral("Video 2")},
        {newId(), TrackKind::Audio, QStringLiteral("Audio 1")},
        {newId(), TrackKind::Audio, QStringLiteral("Audio 2")},
    };
    project.defaultSequenceId = sequence.id;
    project.sequences.push_back(std::move(sequence));
    return project;
}

Sequence* findSequence(Project& project, QStringView id) {
    auto it = std::find_if(project.sequences.begin(), project.sequences.end(), [&](const Sequence& s) { return s.id == id; });
    return it == project.sequences.end() ? nullptr : &*it;
}

const Sequence* findSequence(const Project& project, QStringView id) {
    return findSequence(const_cast<Project&>(project), id);
}

Track* findTrack(Sequence& sequence, QStringView id) {
    auto it = std::find_if(sequence.tracks.begin(), sequence.tracks.end(), [&](const Track& t) { return t.id == id; });
    return it == sequence.tracks.end() ? nullptr : &*it;
}

const Track* findTrack(const Sequence& sequence, QStringView id) { return findTrack(const_cast<Sequence&>(sequence), id); }

QString trackCode(const Sequence& sequence, QStringView trackId) {
    int position = 0;
    for (const Track& t : sequence.tracks) {
        const Track* target = findTrack(sequence, trackId);
        if (!target)
            return {};
        if (t.kind == target->kind)
            ++position;
        if (t.id == trackId)
            return (t.kind == TrackKind::Video ? QStringLiteral("V") : QStringLiteral("A")) + QString::number(position);
    }
    return {};
}

int trackCount(const Sequence& sequence, TrackKind kind) {
    return int(std::count_if(sequence.tracks.begin(), sequence.tracks.end(), [&](const Track& t) { return t.kind == kind; }));
}

SequenceSettings settingsOf(const Sequence& sequence) {
    return {sequence.name, sequence.format, sequence.timecodeStartHour};
}

void applySettings(Sequence& sequence, const SequenceSettings& settings) {
    sequence.name = settings.name;
    sequence.format = settings.format;
    sequence.timecodeStartHour = settings.timecodeStartHour;
}

} // namespace cutline
