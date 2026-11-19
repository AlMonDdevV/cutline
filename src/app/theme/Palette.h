#pragma once

#include "core/Result.h"

#include <QByteArray>
#include <QColor>
#include <QHash>
#include <QString>
#include <QStringList>

namespace cutline {

// Every colour token a theme must define, in display order.
inline const QStringList kPaletteKeys = {
    QStringLiteral("window"),       QStringLiteral("panel"),          QStringLiteral("panelRaised"),
    QStringLiteral("field"),        QStringLiteral("divider"),        QStringLiteral("hover"),
    QStringLiteral("pressed"),      QStringLiteral("selection"),      QStringLiteral("text"),
    QStringLiteral("textMuted"),    QStringLiteral("textDisabled"),   QStringLiteral("textOnAccent"),
    QStringLiteral("accent"),       QStringLiteral("accentHover"),    QStringLiteral("focusRing"),
    QStringLiteral("viewerSurround"), QStringLiteral("clipVideo"),    QStringLiteral("clipAudio"),
    QStringLiteral("clipTitle"),    QStringLiteral("clipGenerator"),  QStringLiteral("danger"),
    QStringLiteral("warning"),      QStringLiteral("success"),        QStringLiteral("timecodeDim"),
};

// Optional colour tokens: when a theme leaves them out they follow another token
// (playhead -> accent, waveform -> clipAudio, icon -> textMuted).
inline const QStringList kOptionalPaletteKeys = {QStringLiteral("playhead"), QStringLiteral("waveform"),
                                                 QStringLiteral("icon")};

// A complete, resolved theme. Files may inherit from a built-in base and override only some values.
struct Palette {
    QString name;
    QString author;
    QString description;
    QHash<QString, QColor> colors; // every key in kPaletteKeys and kOptionalPaletteKeys
    QString uiFont;                // empty = Inter
    QString monoFont;              // empty = JetBrains Mono
    int radius = 3;                // corner radius in px, 0-16
    double density = 1.0;          // spacing and row-height multiplier, 0.8-1.25

    QColor color(const QString& key) const { return colors.value(key); }
    friend bool operator==(const Palette&, const Palette&) = default;
};

inline constexpr int kThemeFormatVersion = 1;

// The built-in "dark" or "light" palette from the app's resources.
Palette builtinPalette(const QString& name);

// The theme file format (docs/themes.md, schema/cutline-theme-v1.schema.json):
// {"format":"cutline.theme","version":1,"name":...,"base":"dark"|"light","author":...,"description":...,
//  "colors":{token:"#rrggbb"|"#aarrggbb"},"fonts":{"ui":...,"mono":...},"metrics":{"radius":0-16,"density":0.8-1.25}}
// Without "base", every key in kPaletteKeys is required. "format" may be omitted (older files).
Result<Palette> parsePalette(const QByteArray& json);
QByteArray serializePalette(const Palette& palette);

// WCAG 2.x contrast ratio between two opaque colours (1.0 to 21.0).
double contrastRatio(const QColor& a, const QColor& b);

// Readability and colour-accuracy warnings for a theme. Empty for the built-in themes.
struct ThemeCheck {
    QStringList warnings;
};
ThemeCheck checkPalette(const Palette& palette);

} // namespace cutline
