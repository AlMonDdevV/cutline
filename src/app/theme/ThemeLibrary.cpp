#include "app/theme/ThemeLibrary.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

#include <algorithm>

namespace cutline {
namespace {

Status writeAtomically(const QString& path, const QByteArray& bytes) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        return Status::failure(QStringLiteral("Couldn't save the theme: %1").arg(file.errorString()));
    return Status::success();
}

bool validFileName(const QString& name) {
    static const QString forbidden = QStringLiteral("\\/:*?\"<>|");
    return std::none_of(name.cbegin(), name.cend(), [](QChar c) { return forbidden.contains(c) || c < QChar(32); });
}

} // namespace

ThemeLibrary::ThemeLibrary(QString directory) : m_directory(std::move(directory)) {}

const QStringList& ThemeLibrary::reservedNames() {
    static const QStringList names = {QStringLiteral("Dark"), QStringLiteral("Light"), QStringLiteral("Match System")};
    return names;
}

QString ThemeLibrary::filePath(const QString& name) const { return QDir(m_directory).filePath(name + QStringLiteral(".json")); }

QStringList ThemeLibrary::customNames() const {
    QStringList names;
    const QFileInfoList files = QDir(m_directory).entryInfoList({QStringLiteral("*.json")}, QDir::Files, QDir::Name);
    for (const QFileInfo& info : files)
        names << info.completeBaseName();
    return names;
}

Result<Palette> ThemeLibrary::load(const QString& name) const {
    QFile file(filePath(name));
    if (!file.open(QIODevice::ReadOnly))
        return Error{Error::Code::Io, QStringLiteral("The theme “%1” couldn't be found.").arg(name)};
    return parsePalette(file.readAll());
}

Result<ThemeCheck> ThemeLibrary::importFrom(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return Error{Error::Code::Io, QStringLiteral("Couldn't read %1.").arg(QFileInfo(path).fileName())};
    auto parsed = parsePalette(file.readAll());
    if (!parsed.ok())
        return parsed.error();

    const Palette palette = parsed.value();
    const QString name = palette.name.trimmed();
    if (name.size() > 40)
        return Error{Error::Code::Validation, QStringLiteral("Theme names can be at most 40 characters.")};
    if (!validFileName(name))
        return Error{Error::Code::Validation, QStringLiteral("Theme names can't contain \\ / : * ? \" < > or |.")};

    QStringList taken = reservedNames() + customNames();
    if (taken.contains(name, Qt::CaseInsensitive))
        return Error{Error::Code::Validation, QStringLiteral("There's already a theme called “%1”.").arg(name)};

    QDir().mkpath(m_directory);
    Palette stored = palette;
    stored.name = name;
    if (const Status saved = writeAtomically(filePath(name), serializePalette(stored)); !saved)
        return Error{Error::Code::Io, saved.message};
    return checkPalette(stored);
}

Status ThemeLibrary::exportTo(const Palette& palette, const QString& path) const {
    return writeAtomically(path, serializePalette(palette));
}

Status ThemeLibrary::remove(const QString& name) {
    QFile file(filePath(name));
    if (!file.exists())
        return Status::failure(QStringLiteral("That theme no longer exists."));
    if (!file.remove())
        return Status::failure(QStringLiteral("Couldn't remove the theme: %1").arg(file.errorString()));
    return Status::success();
}

} // namespace cutline
