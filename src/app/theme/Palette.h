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

struct Palette {
    QString name;
    QHash<QString, QColor> colors;

    QColor color(const QString& key) const { return colors.value(key); }
    friend bool operator==(const Palette&, const Palette&) = default;
};

// {"name": "...", "colors": {"<key>": "#rrggbb" | "#aarrggbb", ...}} with every key in kPaletteKeys.
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
