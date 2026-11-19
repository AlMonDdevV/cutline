#include "core/ProjectIO.h"

#include "core/Validation.h"
#include "core/Version.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>

#include <optional>

namespace cutline::io {
namespace {

const QString kFormat = QStringLiteral("cutline.project");
const QString kNotAProject = QStringLiteral("This file isn't a CUTLINE project.");

QString kindName(TrackKind k) { return k == TrackKind::Video ? QStringLiteral("video") : QStringLiteral("audio"); }

// Reads typed fields out of a JSON object, remembering the path for error messages.
class Reader {
public:
    Reader(QJsonObject object, QString path) : m_object(std::move(object)), m_path(std::move(path)) {}

    QString at(const QString& key) const { return m_path.isEmpty() ? key : m_path + QLatin1Char('.') + key; }

    std::optional<Error> require(const QString& key, QJsonValue::Type type, QJsonValue& out) const {
        if (!m_object.contains(key))
            return Error{Error::Code::MissingField, QStringLiteral("The project file is missing %1.").arg(at(key))};
        out = m_object.value(key);
        if (out.type() != type)
            return wrongType(at(key), type);
        return std::nullopt;
    }

    static Error wrongType(const QString& path, QJsonValue::Type type) {
        const char* noun = "value";
        switch (type) {
        case QJsonValue::Double: noun = "number"; break;
        case QJsonValue::String: noun = "string"; break;
        case QJsonValue::Bool: noun = "boolean"; break;
        case QJsonValue::Array: noun = "array"; break;
        case QJsonValue::Object: noun = "object"; break;
        default: break;
        }
        return Error{Error::Code::WrongType, QStringLiteral("%1 should be a %2.").arg(path, QLatin1String(noun))};
    }

    const QJsonObject& object() const { return m_object; }

private:
    QJsonObject m_object;
    QString m_path;
};

#define CUTLINE_REQUIRE(reader, key, type, out)                                                                        \
    QJsonValue out;                                                                                                    \
    if (auto err = (reader).require(QStringLiteral(key), (type), out))                                                 \
        return *err;

Error invalid(const QString& path, const QString& message) {
    return Error{Error::Code::InvalidValue, QStringLiteral("%1: %2").arg(path, message)};
}

Result<int> readInt(const QJsonValue& v, const QString& path) {
    if (!v.isDouble())
        return Reader::wrongType(path, QJsonValue::Double);
    const double d = v.toDouble();
    if (d != std::floor(d) || d < -2147483648.0 || d > 2147483647.0)
        return invalid(path, QStringLiteral("Must be a whole number."));
    return int(d);
}

Result<Track> readTrack(const QJsonObject& object, const QString& path) {
    Reader r(object, path);
    CUTLINE_REQUIRE(r, "id", QJsonValue::String, id)
    CUTLINE_REQUIRE(r, "kind", QJsonValue::String, kind)
    CUTLINE_REQUIRE(r, "name", QJsonValue::String, name)
    CUTLINE_REQUIRE(r, "locked", QJsonValue::Bool, locked)
    CUTLINE_REQUIRE(r, "enabled", QJsonValue::Bool, enabled)
    CUTLINE_REQUIRE(r, "solo", QJsonValue::Bool, solo)

    Track track;
    track.id = id.toString();
    if (track.id.isEmpty())
        return invalid(r.at(QStringLiteral("id")), QStringLiteral("Ids can't be empty."));
    if (kind.toString() == QLatin1String("video"))
        track.kind = TrackKind::Video;
    else if (kind.toString() == QLatin1String("audio"))
        track.kind = TrackKind::Audio;
    else
        return invalid(r.at(QStringLiteral("kind")), QStringLiteral("Track kind must be \"video\" or \"audio\"."));
    track.name = name.toString();
    if (Status s = validateTrackName(track.name); !s)
        return invalid(r.at(QStringLiteral("name")), s.message);
    track.locked = locked.toBool();
    track.enabled = enabled.toBool();
    track.solo = solo.toBool() && track.kind == TrackKind::Audio;
    return track;
}

Result<Sequence> readSequence(const QJsonObject& object, const QString& path) {
    Reader r(object, path);
    CUTLINE_REQUIRE(r, "id", QJsonValue::String, id)
    CUTLINE_REQUIRE(r, "name", QJsonValue::String, name)
    CUTLINE_REQUIRE(r, "format", QJsonValue::Object, formatValue)
    CUTLINE_REQUIRE(r, "tracks", QJsonValue::Array, tracksValue)

    Sequence sequence;
    sequence.id = id.toString();
    if (sequence.id.isEmpty())
        return invalid(r.at(QStringLiteral("id")), QStringLiteral("Ids can't be empty."));
    sequence.name = name.toString();

    const QString formatPath = r.at(QStringLiteral("format"));
    Reader f(formatValue.toObject(), formatPath);
    CUTLINE_REQUIRE(f, "width", QJsonValue::Double, width)
    CUTLINE_REQUIRE(f, "height", QJsonValue::Double, height)
    CUTLINE_REQUIRE(f, "frameRate", QJsonValue::Array, frameRate)
    CUTLINE_REQUIRE(f, "sampleRate", QJsonValue::Double, sampleRate)
    CUTLINE_REQUIRE(f, "channels", QJsonValue::Double, channels)

    const QJsonArray rate = frameRate.toArray();
    if (rate.size() != 2)
        return invalid(f.at(QStringLiteral("frameRate")), QStringLiteral("Frame rate must be [numerator, denominator]."));
    struct {
        const QJsonValue& value;
        QString path;
        int* target;
    } ints[] = {
        {width, f.at(QStringLiteral("width")), &sequence.format.width},
        {height, f.at(QStringLiteral("height")), &sequence.format.height},
        {rate.at(0), f.at(QStringLiteral("frameRate[0]")), &sequence.format.frameRate.num},
        {rate.at(1), f.at(QStringLiteral("frameRate[1]")), &sequence.format.frameRate.den},
        {sampleRate, f.at(QStringLiteral("sampleRate")), &sequence.format.sampleRate},
        {channels, f.at(QStringLiteral("channels")), &sequence.format.channels},
    };
    for (auto& entry : ints) {
        auto v = readInt(entry.value, entry.path);
        if (!v.ok())
            return v.error();
        *entry.target = v.value();
    }

    if (object.contains(QLatin1String("timecodeStartHour"))) {
        auto hour = readInt(object.value(QLatin1String("timecodeStartHour")), r.at(QStringLiteral("timecodeStartHour")));
        if (!hour.ok())
            return hour.error();
        sequence.timecodeStartHour = hour.value();
    }

    if (Status s = validateSequenceName(sequence.name); !s)
        return invalid(r.at(QStringLiteral("name")), s.message);
    if (Status s = validateSequenceFormat(sequence.format); !s)
        return invalid(formatPath, s.message);
    if (Status s = validateSequenceSettings(settingsOf(sequence)); !s)
        return invalid(r.at(QStringLiteral("timecodeStartHour")), s.message);

    const QJsonArray tracks = tracksValue.toArray();
    std::vector<Track> video, audio;
    for (qsizetype i = 0; i < tracks.size(); ++i) {
        const QString trackPath = r.at(QStringLiteral("tracks[%1]").arg(i));
        if (!tracks.at(i).isObject())
            return Reader::wrongType(trackPath, QJsonValue::Object);
        auto track = readTrack(tracks.at(i).toObject(), trackPath);
        if (!track.ok())
            return track.error();
        (track.value().kind == TrackKind::Video ? video : audio).push_back(track.value());
    }
    if (video.empty())
        return invalid(r.at(QStringLiteral("tracks")), QStringLiteral("A sequence needs at least one video track."));
    if (audio.empty())
        return invalid(r.at(QStringLiteral("tracks")), QStringLiteral("A sequence needs at least one audio track."));
    sequence.tracks = std::move(video);
    sequence.tracks.insert(sequence.tracks.end(), audio.begin(), audio.end());
    return sequence;
}

QString lineAndColumn(const QByteArray& json, int offset) {
    int line = 1, column = 1;
    for (int i = 0; i < offset && i < json.size(); ++i) {
        if (json[i] == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }
    return QStringLiteral("line %1, column %2").arg(line).arg(column);
}

} // namespace

QByteArray serializeProject(const Project& project) {
    QJsonArray sequences;
    for (const Sequence& s : project.sequences) {
        QJsonArray tracks;
        for (const Track& t : s.tracks) {
            tracks.append(QJsonObject{
                {QStringLiteral("id"), t.id},
                {QStringLiteral("kind"), kindName(t.kind)},
                {QStringLiteral("name"), t.name},
                {QStringLiteral("locked"), t.locked},
                {QStringLiteral("enabled"), t.enabled},
                {QStringLiteral("solo"), t.solo},
                {QStringLiteral("clips"), QJsonArray()},
            });
        }
        const QJsonObject format{
            {QStringLiteral("width"), s.format.width},
            {QStringLiteral("height"), s.format.height},
            {QStringLiteral("frameRate"), QJsonArray{s.format.frameRate.num, s.format.frameRate.den}},
            {QStringLiteral("sampleRate"), s.format.sampleRate},
            {QStringLiteral("channels"), s.format.channels},
        };
        sequences.append(QJsonObject{
            {QStringLiteral("id"), s.id},
            {QStringLiteral("name"), s.name},
            {QStringLiteral("format"), format},
            {QStringLiteral("timecodeStartHour"), s.timecodeStartHour},
            {QStringLiteral("tracks"), tracks},
            {QStringLiteral("markers"), QJsonArray()},
        });
    }

    const QJsonObject root{
        {QStringLiteral("format"), kFormat},
        {QStringLiteral("version"), kProjectFormatVersion},
        {QStringLiteral("createdWith"), QStringLiteral("CUTLINE %1").arg(QLatin1String(kVersion))},
        {QStringLiteral("id"), project.id},
        {QStringLiteral("name"), project.name},
        {QStringLiteral("settings"), QJsonObject{{QStringLiteral("defaultSequence"), project.defaultSequenceId}}},
        {QStringLiteral("media"), QJsonArray()},
        {QStringLiteral("bins"), QJsonArray()},
        {QStringLiteral("sequences"), sequences},
        {QStringLiteral("ui"), QJsonObject()},
    };
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

Result<Project> deserializeProject(const QByteArray& json) {
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return Error{Error::Code::Corrupt, QStringLiteral("The project file is damaged: %1 (%2).")
                                               .arg(parseError.errorString(), lineAndColumn(json, parseError.offset))};
    if (!doc.isObject() || doc.object().value(QLatin1String("format")).toString() != kFormat)
        return Error{Error::Code::NotACutlineProject, kNotAProject};

    Reader r(doc.object(), QString());
    CUTLINE_REQUIRE(r, "version", QJsonValue::Double, version)
    auto versionNumber = readInt(version, QStringLiteral("version"));
    if (!versionNumber.ok())
        return versionNumber.error();
    if (versionNumber.value() > kProjectFormatVersion)
        return Error{Error::Code::NewerVersion,
                     QStringLiteral("This project was made with a newer version of CUTLINE (project format %1). "
                                    "Update CUTLINE to open it.")
                         .arg(versionNumber.value())};
    if (versionNumber.value() < 1)
        return invalid(QStringLiteral("version"), QStringLiteral("Unsupported project format."));

    CUTLINE_REQUIRE(r, "id", QJsonValue::String, id)
    CUTLINE_REQUIRE(r, "name", QJsonValue::String, name)
    CUTLINE_REQUIRE(r, "sequences", QJsonValue::Array, sequencesValue)

    Project project;
    project.id = id.toString();
    project.name = name.toString();
    if (Status s = validateProjectName(project.name); !s)
        return invalid(QStringLiteral("name"), s.message);

    const QJsonArray sequences = sequencesValue.toArray();
    if (sequences.isEmpty())
        return invalid(QStringLiteral("sequences"), QStringLiteral("A project needs at least one sequence."));
    QSet<QString> ids{project.id};
    for (qsizetype i = 0; i < sequences.size(); ++i) {
        const QString path = QStringLiteral("sequences[%1]").arg(i);
        if (!sequences.at(i).isObject())
            return Reader::wrongType(path, QJsonValue::Object);
        auto sequence = readSequence(sequences.at(i).toObject(), path);
        if (!sequence.ok())
            return sequence.error();
        for (const QString& newIdValue : [&] {
                 QStringList all{sequence.value().id};
                 for (const Track& t : sequence.value().tracks)
                     all << t.id;
                 return all;
             }()) {
            if (ids.contains(newIdValue))
                return invalid(path, QStringLiteral("The id \"%1\" is used more than once.").arg(newIdValue));
            ids.insert(newIdValue);
        }
        project.sequences.push_back(std::move(sequence).value());
    }

    const QString defaultSequence =
        doc.object().value(QLatin1String("settings")).toObject().value(QLatin1String("defaultSequence")).toString();
    project.defaultSequenceId =
        findSequence(project, defaultSequence) ? defaultSequence : project.sequences.front().id;
    return project;
}

Status saveProject(const Project& project, const QString& path) {
    QSaveFile file(path);
    const QByteArray bytes = serializeProject(project);
    if (!file.open(QIODevice::WriteOnly))
        return Status::failure(QStringLiteral("Couldn't save the project: %1").arg(file.errorString()));
    if (file.write(bytes) != bytes.size() || !file.commit())
        return Status::failure(QStringLiteral("Couldn't save the project: %1").arg(file.errorString()));
    return Status::success();
}

Result<Project> loadProject(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return Error{Error::Code::Io, QStringLiteral("Couldn't open the project: %1").arg(file.errorString())};
    return deserializeProject(file.readAll());
}

} // namespace cutline::io
