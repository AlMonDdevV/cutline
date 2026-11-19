#include "core/Commands.h"

#include "core/Validation.h"

#include <algorithm>

namespace cutline {
namespace {

const QString kNoSequence = QStringLiteral("That sequence no longer exists.");
const QString kNoTrack = QStringLiteral("That track no longer exists.");

class RenameProject final : public Command {
public:
    explicit RenameProject(QString name) : m_name(std::move(name)) {}
    QString title() const override { return QStringLiteral("Rename project"); }
    ChangeHint hint() const override { return {ChangeHint::Scope::Project, {}}; }
    Status apply(Project& p) override {
        if (Status s = validateProjectName(m_name); !s)
            return s;
        m_previous = p.name;
        p.name = m_name.trimmed();
        return Status::success();
    }
    void revert(Project& p) override { p.name = m_previous; }

private:
    QString m_name, m_previous;
};

class SetSequenceSettings final : public Command {
public:
    SetSequenceSettings(QString sequenceId, SequenceSettings settings)
        : m_sequenceId(std::move(sequenceId)), m_settings(std::move(settings)) {}
    QString title() const override { return QStringLiteral("Change sequence settings"); }
    ChangeHint hint() const override { return {ChangeHint::Scope::Sequence, m_sequenceId}; }
    Status apply(Project& p) override {
        Sequence* s = findSequence(p, m_sequenceId);
        if (!s)
            return Status::failure(kNoSequence);
        SequenceSettings next = m_settings;
        next.name = next.name.trimmed();
        if (Status v = validateSequenceSettings(next); !v)
            return v;
        m_previous = settingsOf(*s);
        applySettings(*s, next);
        return Status::success();
    }
    void revert(Project& p) override {
        if (Sequence* s = findSequence(p, m_sequenceId))
            applySettings(*s, m_previous);
    }

private:
    QString m_sequenceId;
    SequenceSettings m_settings, m_previous;
};

class AddTrack final : public Command {
public:
    AddTrack(QString sequenceId, TrackKind kind, QString trackId)
        : m_sequenceId(std::move(sequenceId)), m_kind(kind), m_trackId(std::move(trackId)) {}
    QString title() const override {
        return m_kind == TrackKind::Video ? QStringLiteral("Add video track") : QStringLiteral("Add audio track");
    }
    ChangeHint hint() const override { return {ChangeHint::Scope::Tracks, m_sequenceId}; }
    Status apply(Project& p) override {
        Sequence* s = findSequence(p, m_sequenceId);
        if (!s)
            return Status::failure(kNoSequence);
        const int number = trackCount(*s, m_kind) + 1;
        Track track{m_trackId, m_kind,
                    (m_kind == TrackKind::Video ? QStringLiteral("Video %1") : QStringLiteral("Audio %1")).arg(number)};
        auto lastOfKind = std::find_if(s->tracks.rbegin(), s->tracks.rend(), [&](const Track& t) { return t.kind == m_kind; });
        auto position = lastOfKind == s->tracks.rend()
                            ? (m_kind == TrackKind::Video ? s->tracks.begin() : s->tracks.end())
                            : lastOfKind.base();
        s->tracks.insert(position, std::move(track));
        return Status::success();
    }
    void revert(Project& p) override {
        if (Sequence* s = findSequence(p, m_sequenceId))
            std::erase_if(s->tracks, [&](const Track& t) { return t.id == m_trackId; });
    }

private:
    QString m_sequenceId;
    TrackKind m_kind;
    QString m_trackId;
};

class RemoveTrack final : public Command {
public:
    RemoveTrack(QString sequenceId, QString trackId) : m_sequenceId(std::move(sequenceId)), m_trackId(std::move(trackId)) {}
    QString title() const override { return QStringLiteral("Remove track %1").arg(m_code); }
    ChangeHint hint() const override { return {ChangeHint::Scope::Tracks, m_sequenceId}; }
    Status apply(Project& p) override {
        Sequence* s = findSequence(p, m_sequenceId);
        if (!s)
            return Status::failure(kNoSequence);
        auto it = std::find_if(s->tracks.begin(), s->tracks.end(), [&](const Track& t) { return t.id == m_trackId; });
        if (it == s->tracks.end())
            return Status::failure(kNoTrack);
        if (trackCount(*s, it->kind) <= 1)
            return Status::failure(it->kind == TrackKind::Video
                                       ? QStringLiteral("A sequence needs at least one video track.")
                                       : QStringLiteral("A sequence needs at least one audio track."));
        if (m_code.isEmpty())
            m_code = trackCode(*s, m_trackId);
        m_index = std::distance(s->tracks.begin(), it);
        m_removed = *it;
        s->tracks.erase(it);
        return Status::success();
    }
    void revert(Project& p) override {
        if (Sequence* s = findSequence(p, m_sequenceId))
            s->tracks.insert(s->tracks.begin() + m_index, m_removed);
    }

private:
    QString m_sequenceId, m_trackId, m_code;
    std::ptrdiff_t m_index = 0;
    Track m_removed;
};

// Shared shape for edits to one field of one track.
class TrackEdit : public Command {
public:
    TrackEdit(QString sequenceId, QString trackId) : m_sequenceId(std::move(sequenceId)), m_trackId(std::move(trackId)) {}
    ChangeHint hint() const override { return {ChangeHint::Scope::Tracks, m_sequenceId}; }
    QString title() const override { return m_title; }
    Status apply(Project& p) override {
        Sequence* s = findSequence(p, m_sequenceId);
        if (!s)
            return Status::failure(kNoSequence);
        Track* t = findTrack(*s, m_trackId);
        if (!t)
            return Status::failure(kNoTrack);
        if (Status v = validate(*t); !v)
            return v;
        if (m_title.isEmpty())
            m_title = makeTitle(*t, trackCode(*s, m_trackId));
        m_before = *t;
        change(*t);
        return Status::success();
    }
    void revert(Project& p) override {
        if (Sequence* s = findSequence(p, m_sequenceId))
            if (Track* t = findTrack(*s, m_trackId))
                *t = m_before;
    }

protected:
    virtual Status validate(const Track&) const { return Status::success(); }
    virtual void change(Track& track) const = 0;
    virtual QString makeTitle(const Track& track, const QString& code) const = 0;

private:
    QString m_sequenceId, m_trackId, m_title;
    Track m_before;
};

class SetTrackName final : public TrackEdit {
public:
    SetTrackName(QString seq, QString track, QString name) : TrackEdit(std::move(seq), std::move(track)), m_name(std::move(name)) {}

protected:
    Status validate(const Track&) const override { return validateTrackName(m_name); }
    void change(Track& t) const override { t.name = m_name.trimmed(); }
    QString makeTitle(const Track&, const QString& code) const override { return QStringLiteral("Rename track %1").arg(code); }

private:
    QString m_name;
};

class SetTrackLocked final : public TrackEdit {
public:
    SetTrackLocked(QString seq, QString track, bool locked) : TrackEdit(std::move(seq), std::move(track)), m_locked(locked) {}

protected:
    void change(Track& t) const override { t.locked = m_locked; }
    QString makeTitle(const Track&, const QString& code) const override {
        return (m_locked ? QStringLiteral("Lock track %1") : QStringLiteral("Unlock track %1")).arg(code);
    }

private:
    bool m_locked;
};

class SetTrackEnabled final : public TrackEdit {
public:
    SetTrackEnabled(QString seq, QString track, bool enabled)
        : TrackEdit(std::move(seq), std::move(track)), m_enabled(enabled) {}

protected:
    void change(Track& t) const override { t.enabled = m_enabled; }
    QString makeTitle(const Track& t, const QString& code) const override {
        if (t.kind == TrackKind::Video)
            return (m_enabled ? QStringLiteral("Show track %1") : QStringLiteral("Hide track %1")).arg(code);
        return (m_enabled ? QStringLiteral("Unmute track %1") : QStringLiteral("Mute track %1")).arg(code);
    }

private:
    bool m_enabled;
};

class SetTrackSolo final : public TrackEdit {
public:
    SetTrackSolo(QString seq, QString track, bool solo) : TrackEdit(std::move(seq), std::move(track)), m_solo(solo) {}

protected:
    Status validate(const Track& t) const override {
        return t.kind == TrackKind::Audio ? Status::success()
                                          : Status::failure(QStringLiteral("Solo is only available on audio tracks."));
    }
    void change(Track& t) const override { t.solo = m_solo; }
    QString makeTitle(const Track&, const QString& code) const override {
        return (m_solo ? QStringLiteral("Solo track %1") : QStringLiteral("Unsolo track %1")).arg(code);
    }

private:
    bool m_solo;
};

} // namespace

std::unique_ptr<Command> makeRenameProject(QString name) { return std::make_unique<RenameProject>(std::move(name)); }
std::unique_ptr<Command> makeSetSequenceSettings(QString sequenceId, SequenceSettings settings) {
    return std::make_unique<SetSequenceSettings>(std::move(sequenceId), std::move(settings));
}
std::unique_ptr<Command> makeAddTrack(QString sequenceId, TrackKind kind, QString newTrackId) {
    return std::make_unique<AddTrack>(std::move(sequenceId), kind, std::move(newTrackId));
}
std::unique_ptr<Command> makeRemoveTrack(QString sequenceId, QString trackId) {
    return std::make_unique<RemoveTrack>(std::move(sequenceId), std::move(trackId));
}
std::unique_ptr<Command> makeSetTrackName(QString sequenceId, QString trackId, QString name) {
    return std::make_unique<SetTrackName>(std::move(sequenceId), std::move(trackId), std::move(name));
}
std::unique_ptr<Command> makeSetTrackLocked(QString sequenceId, QString trackId, bool locked) {
    return std::make_unique<SetTrackLocked>(std::move(sequenceId), std::move(trackId), locked);
}
std::unique_ptr<Command> makeSetTrackEnabled(QString sequenceId, QString trackId, bool enabled) {
    return std::make_unique<SetTrackEnabled>(std::move(sequenceId), std::move(trackId), enabled);
}
std::unique_ptr<Command> makeSetTrackSolo(QString sequenceId, QString trackId, bool solo) {
    return std::make_unique<SetTrackSolo>(std::move(sequenceId), std::move(trackId), solo);
}

} // namespace cutline
