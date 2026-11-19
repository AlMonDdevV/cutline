#include "app/theme/ThemeController.h"

#include "app/theme/ThemeLibrary.h"

#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
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

Palette ThemeController::builtinPalette(const QString& name) { return cutline::builtinPalette(name); }

ThemeController::ThemeController(const QString& mode, qreal scale, ThemeLibrary* library, QObject* parent)
    : QObject(parent), m_library(library), m_scale(std::clamp(scale, 0.9, 1.5)) {
    if (auto* hints = QGuiApplication::styleHints()) {
        connect(hints, &QStyleHints::colorSchemeChanged, this, [this] {
            if (m_mode == QLatin1String("system"))
                applySystemScheme();
        });
    }
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &ThemeController::reloadCurrent);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        emit themesChanged();
        reloadCurrent(); // editors that save by replacing the file show up as a directory change
    });
    connect(this, &ThemeController::paletteChanged, this, &ThemeController::metricsChanged);
    connect(this, &ThemeController::scaleChanged, this, &ThemeController::metricsChanged);
    setMode(mode);
}

QString ThemeController::uiFont() const {
    const QString wanted = m_palette.uiFont;
    return !wanted.isEmpty() && QFontDatabase::families().contains(wanted) ? wanted : QStringLiteral("Inter");
}

QString ThemeController::monoFont() const {
    const QString wanted = m_palette.monoFont;
    return !wanted.isEmpty() && QFontDatabase::families().contains(wanted) ? wanted : QStringLiteral("JetBrains Mono");
}

void ThemeController::watchCurrent() {
    if (!m_watcher.files().isEmpty())
        m_watcher.removePaths(m_watcher.files());
    if (!m_library)
        return;
    if (!m_library->directory().isEmpty() && QFileInfo::exists(m_library->directory()) &&
        !m_watcher.directories().contains(m_library->directory()))
        m_watcher.addPath(m_library->directory());
    if (m_mode.startsWith(kCustomPrefix)) {
        const QString path = m_library->filePath(m_mode.mid(kCustomPrefix.size()));
        if (QFileInfo::exists(path))
            m_watcher.addPath(path);
    }
}

// Live reload while someone edits their theme file: keep the last good palette if the file is mid-edit/broken.
void ThemeController::reloadCurrent() {
    if (!m_library || !m_mode.startsWith(kCustomPrefix))
        return;
    const QString name = m_mode.mid(kCustomPrefix.size());
    auto loaded = m_library->load(name);
    if (loaded.ok()) {
        setWarning({});
        setPalette(loaded.value());
    } else if (QFileInfo::exists(m_library->filePath(name))) {
        setWarning(QStringLiteral("“%1” has an error: %2").arg(name, loaded.error().message));
    }
    watchCurrent();
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
    watchCurrent();
}

QStringList ThemeController::themeNames() const {
    QStringList names = ThemeLibrary::reservedNames();
    if (m_library)
        names << m_library->customNames();
    return names;
}

void ThemeController::refreshThemes() { emit themesChanged(); }

QVariantMap ThemeController::importTheme(const QUrl& file) {
    if (!m_library)
        return {{QStringLiteral("ok"), false}, {QStringLiteral("error"), QStringLiteral("Custom themes aren't available.")}};
    const QString path = file.isLocalFile() ? file.toLocalFile() : file.toString();
    auto imported = m_library->importFrom(path);
    if (!imported.ok())
        return {{QStringLiteral("ok"), false}, {QStringLiteral("error"), imported.error().message}, {QStringLiteral("warnings"), QStringList()}};
    QFile f(path);
    QString name;
    if (f.open(QIODevice::ReadOnly)) {
        auto parsed = parsePalette(f.readAll());
        if (parsed.ok())
            name = parsed.value().name.trimmed();
    }
    refreshThemes();
    if (!name.isEmpty())
        setMode(QStringLiteral("custom:") + name);
    return {{QStringLiteral("ok"), true}, {QStringLiteral("name"), name}, {QStringLiteral("warnings"), imported.value().warnings}};
}

QString ThemeController::exportTheme(const QUrl& file) const {
    const QString path = file.isLocalFile() ? file.toLocalFile() : file.toString();
    Palette p = m_palette;
    if (p.name.isEmpty() || p.name == QLatin1String("dark") || p.name == QLatin1String("light"))
        p.name = QStringLiteral("My %1 theme").arg(dark() ? QStringLiteral("dark") : QStringLiteral("light"));
    ThemeLibrary scratch(QString{});
    const Status s = (m_library ? *m_library : scratch).exportTo(p, path);
    return s.ok ? QString() : s.message;
}

void ThemeController::setScale(qreal scale) {
    const qreal clamped = std::clamp(scale, 0.9, 1.5);
    if (qFuzzyCompare(clamped, m_scale))
        return;
    m_scale = clamped;
    emit scaleChanged();
}

} // namespace cutline
