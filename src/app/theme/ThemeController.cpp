#include "app/theme/ThemeController.h"

#include "app/theme/ThemeLibrary.h"

#include <QFile>
#include <QGuiApplication>
#include <QStyleHints>

#include <algorithm>

namespace cutline {
namespace {

const QString kCustomPrefix = QStringLiteral("custom:");

double luminance(const QColor& c) {
    auto ch = [](double v) { return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
    return 0.2126 * ch(c.redF()) + 0.7152 * ch(c.greenF()) + 0.0722 * ch(c.blueF());
}

} // namespace

Palette ThemeController::builtinPalette(const QString& name) {
    QFile file(QStringLiteral(":/cutline/theme/%1.json").arg(name));
    if (file.open(QIODevice::ReadOnly)) {
        auto parsed = parsePalette(file.readAll());
        if (parsed.ok())
            return parsed.value();
        Q_ASSERT_X(false, "ThemeController", qPrintable(parsed.error().message));
    }
    Q_ASSERT_X(false, "ThemeController", "built-in palette missing from resources");
    // Release fallback: neutral greys so the UI stays usable.
    Palette fallback;
    fallback.name = name;
    for (const QString& key : kPaletteKeys)
        fallback.colors.insert(key, key.startsWith(QLatin1String("text")) ? QColor(0xE6, 0xE6, 0xE6) : QColor(0x1C, 0x1C, 0x1C));
    return fallback;
}

ThemeController::ThemeController(const QString& mode, qreal scale, ThemeLibrary* library, QObject* parent)
    : QObject(parent), m_library(library), m_scale(std::clamp(scale, 0.9, 1.5)) {
    if (auto* hints = QGuiApplication::styleHints()) {
        connect(hints, &QStyleHints::colorSchemeChanged, this, [this] {
            if (m_mode == QLatin1String("system"))
                applySystemScheme();
        });
    }
    setMode(mode);
}

bool ThemeController::dark() const { return luminance(panel()) < 0.2; }

void ThemeController::setPalette(const Palette& palette) {
    if (palette == m_palette)
        return;
    m_palette = palette;
    emit paletteChanged();
}

void ThemeController::setWarning(const QString& warning) {
    if (warning == m_warning)
        return;
    m_warning = warning;
    emit warningChanged();
}

void ThemeController::applySystemScheme() {
    const auto* hints = QGuiApplication::styleHints();
    const bool systemLight = hints && hints->colorScheme() == Qt::ColorScheme::Light;
    setPalette(builtinPalette(systemLight ? QStringLiteral("light") : QStringLiteral("dark")));
}

void ThemeController::setMode(const QString& mode) {
    QString effective = mode;
    setWarning({});

    if (mode == QLatin1String("light")) {
        setPalette(builtinPalette(QStringLiteral("light")));
    } else if (mode == QLatin1String("system")) {
        applySystemScheme();
    } else if (mode.startsWith(kCustomPrefix)) {
        const QString name = mode.mid(kCustomPrefix.size());
        auto loaded = m_library ? m_library->load(name)
                                : Result<Palette>(Error{Error::Code::Io, QStringLiteral("No custom themes are available.")});
        if (loaded.ok()) {
            setPalette(loaded.value());
        } else {
            setWarning(QStringLiteral("The theme “%1” couldn't be loaded, so the Dark theme is in use.").arg(name));
            setPalette(builtinPalette(QStringLiteral("dark")));
            effective = QStringLiteral("dark");
        }
    } else {
        setPalette(builtinPalette(QStringLiteral("dark")));
        effective = QStringLiteral("dark");
    }

    if (effective != m_mode) {
        m_mode = effective;
        emit modeChanged();
    }
}

QStringList ThemeController::themeNames() const {
    QStringList names = ThemeLibrary::reservedNames();
    if (m_library)
        names << m_library->customNames();
    return names;
}

void ThemeController::refreshThemes() { emit themesChanged(); }

void ThemeController::setScale(qreal scale) {
    const qreal clamped = std::clamp(scale, 0.9, 1.5);
    if (qFuzzyCompare(clamped, m_scale))
        return;
    m_scale = clamped;
    emit scaleChanged();
}

} // namespace cutline
