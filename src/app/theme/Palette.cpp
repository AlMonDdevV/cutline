#include "app/theme/Palette.h"

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

Result<Palette> parsePalette(const QByteArray& json) {
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        return Error{Error::Code::Corrupt, kNotATheme};

    const QJsonObject root = doc.object();
    const QString name = root.value(QLatin1String("name")).toString();
    if (name.trimmed().isEmpty() || !root.value(QLatin1String("colors")).isObject())
        return Error{Error::Code::NotACutlineProject, kNotATheme};

    Palette palette;
    palette.name = name;
    const QJsonObject colors = root.value(QLatin1String("colors")).toObject();
    for (const QString& key : kPaletteKeys) {
        if (!colors.contains(key))
            return Error{Error::Code::MissingField,
                         QStringLiteral("Theme \"%1\" is missing colour \"%2\".").arg(name, key)};
        const QColor color = QColor::fromString(colors.value(key).toString());
        if (!color.isValid())
            return Error{Error::Code::InvalidValue,
                         QStringLiteral("Theme \"%1\" has an invalid colour for \"%2\".").arg(name, key)};
        palette.colors.insert(key, color);
    }
    return palette;
}

QByteArray serializePalette(const Palette& palette) {
    QJsonObject colors;
    for (const QString& key : kPaletteKeys)
        colors.insert(key, colorText(palette.color(key)));
    QJsonObject root;
    root.insert(QStringLiteral("name"), palette.name);
    root.insert(QStringLiteral("colors"), colors);
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
