#include "app/controllers/AppSettings.h"

#include <QDir>

#include <algorithm>

namespace cutline {

AppSettings::AppSettings(const QString& configDir, QObject* parent)
    : QObject(parent), m_configDir(configDir),
      m_settings(QDir(configDir).filePath(QStringLiteral("cutline.ini")), QSettings::IniFormat) {
    QDir().mkpath(configDir);
}

QString AppSettings::theme() const { return m_settings.value(QStringLiteral("ui/theme"), QStringLiteral("dark")).toString(); }

void AppSettings::setTheme(const QString& theme) {
    if (theme == this->theme())
        return;
    m_settings.setValue(QStringLiteral("ui/theme"), theme);
    emit themeChanged();
}

double AppSettings::uiScale() const {
    return std::clamp(m_settings.value(QStringLiteral("ui/scale"), 1.0).toDouble(), 0.9, 1.5);
}

void AppSettings::setUiScale(double scale) {
    scale = std::clamp(scale, 0.9, 1.5);
    if (qFuzzyCompare(scale, uiScale()))
        return;
    m_settings.setValue(QStringLiteral("ui/scale"), scale);
    emit uiScaleChanged();
}

bool AppSettings::showWelcome() const { return m_settings.value(QStringLiteral("ui/showWelcome"), true).toBool(); }

void AppSettings::setShowWelcome(bool show) {
    if (show == showWelcome())
        return;
    m_settings.setValue(QStringLiteral("ui/showWelcome"), show);
    emit showWelcomeChanged();
}

} // namespace cutline
