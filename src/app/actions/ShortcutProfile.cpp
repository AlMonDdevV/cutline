#include "app/actions/ShortcutProfile.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace cutline {
namespace {

const QString kFormat = QStringLiteral("cutline.shortcuts");
const QString kNotAProfile = QStringLiteral("This file isn't a CUTLINE shortcuts profile.");

Status writeAtomically(const QString& path, const QByteArray& bytes) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        return Status::failure(QStringLiteral("Couldn't save the shortcuts: %1").arg(file.errorString()));
    return Status::success();
}

} // namespace

QByteArray serializeShortcutProfile(const ShortcutProfile& profile) {
    QJsonObject overrides;
    for (auto it = profile.overrides.cbegin(); it != profile.overrides.cend(); ++it) {
        QJsonArray keys;
        for (const QKeySequence& seq : it.value())
            keys.append(seq.toString(QKeySequence::PortableText));
        overrides.insert(it.key(), keys);
    }
    const QJsonObject root{
        {QStringLiteral("format"), kFormat},
        {QStringLiteral("version"), 1},
        {QStringLiteral("name"), profile.name},
        {QStringLiteral("base"), QStringLiteral("default")},
        {QStringLiteral("overrides"), overrides},
    };
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

Result<ShortcutProfile> parseShortcutProfile(const QByteArray& json) {
    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject() ||
        doc.object().value(QLatin1String("format")).toString() != kFormat)
        return Error{Error::Code::NotACutlineProject, kNotAProfile};

    const QJsonObject root = doc.object();
    ShortcutProfile profile;
    const QString name = root.value(QLatin1String("name")).toString().trimmed();
    if (!name.isEmpty())
        profile.name = name;
    const QJsonObject overrides = root.value(QLatin1String("overrides")).toObject();
    for (auto it = overrides.constBegin(); it != overrides.constEnd(); ++it) {
        QList<QKeySequence> keys;
        for (const QJsonValue& v : it.value().toArray()) {
            const QKeySequence seq = QKeySequence::fromString(v.toString(), QKeySequence::PortableText);
            if (!seq.isEmpty())
                keys << seq;
        }
        profile.overrides.insert(it.key(), keys);
    }
    return profile;
}

ShortcutStore::ShortcutStore(QString directory) : m_directory(std::move(directory)) {}

QString ShortcutStore::activePath() const { return QDir(m_directory).filePath(QStringLiteral("shortcuts.json")); }

ShortcutProfile ShortcutStore::loadActive(QString* warning) const {
    QFile file(activePath());
    if (!file.exists())
        return {};
    if (file.open(QIODevice::ReadOnly)) {
        auto parsed = parseShortcutProfile(file.readAll());
        if (parsed.ok())
            return parsed.value();
    }
    if (warning)
        *warning = QStringLiteral("Your keyboard shortcuts couldn't be read, so the defaults are in use.");
    return {};
}

Status ShortcutStore::saveActive(const ShortcutProfile& profile) const {
    return writeAtomically(activePath(), serializeShortcutProfile(profile));
}

Status ShortcutStore::exportTo(const ShortcutProfile& profile, const QString& path) const {
    return writeAtomically(path, serializeShortcutProfile(profile));
}

Result<ShortcutProfile> ShortcutStore::importFrom(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return Error{Error::Code::Io, QStringLiteral("Couldn't read %1.").arg(QFileInfo(path).fileName())};
    return parseShortcutProfile(file.readAll());
}

} // namespace cutline
