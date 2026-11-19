#include "app/theme/Palette.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include <cmath>

namespace cutline {
namespace {

const QString kNotATheme = QStringLiteral("This file isn't a CUTLINE theme.");

double channel(double v) { return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); }

double luminance(const QColor& c) {
    return 0.2126 * channel(c.redF()) + 0.7152 * channel(c.greenF()) + 0.0722 * channel(c.blueF());
}

bool isNeutral(const QColor& c) { return c.red() == c.green() && c.green() == c.blue(); }

QString colorText(const QColor& c) {
    return (c.alpha() == 255 ? c.name(QColor::HexRgb) : c.name(QColor::HexArgb)).toUpper();
}

} // namespace

Palette builtinPalette(const QString& name) {
    QFile file(QStringLiteral(":/cutline/theme/%1.json").arg(name));
    if (file.open(QIODevice::ReadOnly)) {
        auto parsed = parsePalette(file.readAll());
        if (parsed.ok())
            return parsed.value();
        Q_ASSERT_X(false, "builtinPalette", qPrintable(parsed.error().message));
    }
    Q_ASSERT_X(false, "builtinPalette", "built-in palette missing from resources");
    Palette fallback; // release fallback: neutral greys so the UI stays usable
    fallback.name = name;
    for (const QString& key : kPaletteKeys + kOptionalPaletteKeys)
        fallback.colors.insert(key, key.startsWith(QLatin1String("text")) ? QColor(0xE6, 0xE6, 0xE6) : QColor(0x1C, 0x1C, 0x1C));
    return fallback;
}

Result<Palette> parsePalette(const QByteArray& json) {
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return Error{Error::Code::Corrupt, kNotATheme};

    const QJsonObject root = doc.object();
    if (root.contains(QLatin1String("format")) && root.value(QLatin1String("format")).toString() != QLatin1String("cutline.theme"))
        return Error{Error::Code::NotACutlineProject, kNotATheme};
    const QString name = root.value(QLatin1String("name")).toString().trimmed();
    if (name.isEmpty())
        return Error{Error::Code::NotACutlineProject, kNotATheme};
    if (root.value(QLatin1String("version")).toInt(1) > kThemeFormatVersion)
        return Error{Error::Code::NewerVersion, QStringLiteral("“%1” was made for a newer version of CUTLINE.").arg(name)};
    const QJsonValue colorsValue = root.value(QLatin1String("colors"));
    if (!colorsValue.isUndefined() && !colorsValue.isObject())
        return Error{Error::Code::WrongType, kNotATheme};
    const auto invalid = [&](const QString& what) {
        return Error{Error::Code::InvalidValue, QStringLiteral("Theme \"%1\": %2").arg(name, what)};
    };

    Palette palette;
    const QString base = root.value(QLatin1String("base")).toString();
    if (root.contains(QLatin1String("base"))) {
        if (base != QLatin1String("dark") && base != QLatin1String("light"))
            return invalid(QStringLiteral("base must be \"dark\" or \"light\"."));
        palette = builtinPalette(base);
    } else if (!colorsValue.isObject()) {
        return Error{Error::Code::NotACutlineProject, kNotATheme};
    }
    palette.name = name;
    palette.author = root.value(QLatin1String("author")).toString();
    palette.description = root.value(QLatin1String("description")).toString();

    const QJsonObject colors = colorsValue.toObject();
    for (const QString& key : kPaletteKeys + kOptionalPaletteKeys) {
        if (!colors.contains(key)) {
            if (!base.isEmpty() || kOptionalPaletteKeys.contains(key))
                continue;
            return Error{Error::Code::MissingField, QStringLiteral("Theme \"%1\" is missing colour \"%2\".").arg(name, key)};
        }
        const QColor color = QColor::fromString(colors.value(key).toString());
        if (!color.isValid())
            return Error{Error::Code::InvalidValue,
                         QStringLiteral("Theme \"%1\" has an invalid colour for \"%2\".").arg(name, key)};
        palette.colors.insert(key, color);
    }
    // Optional tokens follow their partners unless this file sets them.
    const auto follow = [&](const char* key, const char* partner) {
        if (!colors.contains(QLatin1String(key)))
            palette.colors.insert(QLatin1String(key), palette.color(QLatin1String(partner)));
    };
    follow("playhead", "accent");
    follow("waveform", "clipAudio");
    follow("icon", "textMuted");

    const QJsonObject fonts = root.value(QLatin1String("fonts")).toObject();
    palette.uiFont = fonts.value(QLatin1String("ui")).toString(palette.uiFont).trimmed();
    palette.monoFont = fonts.value(QLatin1String("mono")).toString(palette.monoFont).trimmed();

    const QJsonObject metrics = root.value(QLatin1String("metrics")).toObject();
    if (metrics.contains(QLatin1String("radius"))) {
        const double r = metrics.value(QLatin1String("radius")).toDouble(-1);
        if (r < 0 || r > 16)
            return invalid(QStringLiteral("radius must be between 0 and 16."));
        palette.radius = int(std::lround(r));
    }
    if (metrics.contains(QLatin1String("density"))) {
        const double d = metrics.value(QLatin1String("density")).toDouble(-1);
        if (d < 0.8 || d > 1.25)
            return invalid(QStringLiteral("density must be between 0.8 and 1.25."));
        palette.density = d;
    }
    return palette;
}

QByteArray serializePalette(const Palette& palette) {
    QJsonObject colors;
    for (const QString& key : kPaletteKeys + kOptionalPaletteKeys)
        colors.insert(key, colorText(palette.color(key)));
    QJsonObject root;
    root.insert(QStringLiteral("format"), QStringLiteral("cutline.theme"));
    root.insert(QStringLiteral("version"), kThemeFormatVersion);
    root.insert(QStringLiteral("name"), palette.name);
    if (!palette.author.isEmpty())
        root.insert(QStringLiteral("author"), palette.author);
    if (!palette.description.isEmpty())
        root.insert(QStringLiteral("description"), palette.description);
    root.insert(QStringLiteral("colors"), colors);
    QJsonObject fonts;
    if (!palette.uiFont.isEmpty())
        fonts.insert(QStringLiteral("ui"), palette.uiFont);
    if (!palette.monoFont.isEmpty())
        fonts.insert(QStringLiteral("mono"), palette.monoFont);
    if (!fonts.isEmpty())
        root.insert(QStringLiteral("fonts"), fonts);
    root.insert(QStringLiteral("metrics"),
                QJsonObject{{QStringLiteral("radius"), palette.radius}, {QStringLiteral("density"), palette.density}});
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

double contrastRatio(const QColor& a, const QColor& b) {
    const double la = luminance(a);
    const double lb = luminance(b);
    return (std::max(la, lb) + 0.05) / (std::min(la, lb) + 0.05);
}

ThemeCheck checkPalette(const Palette& p) {
    ThemeCheck check;

    bool textOk = true;
    for (const auto* fg : {"text", "textMuted"})
        for (const auto* bg : {"window", "panel", "panelRaised", "field"})
            if (contrastRatio(p.color(QLatin1String(fg)), p.color(QLatin1String(bg))) < 4.5)
                textOk = false;
    if (!textOk)
        check.warnings << QStringLiteral("Some text in “%1” may be hard to read.").arg(p.name);

    if (contrastRatio(p.color(QStringLiteral("accent")), p.color(QStringLiteral("panel"))) < 3.0)
        check.warnings << QStringLiteral("The accent colour in “%1” may be hard to see.").arg(p.name);

    bool neutral = true;
    for (const auto* key : {"window", "panel", "panelRaised", "field", "divider", "hover", "pressed", "viewerSurround"})
        if (!isNeutral(p.color(QLatin1String(key))))
            neutral = false;
    if (!neutral)
        check.warnings << QStringLiteral("“%1” tints the panels, which can affect how you judge colour in the viewer.")
                              .arg(p.name);

    return check;
}

} // namespace cutline
