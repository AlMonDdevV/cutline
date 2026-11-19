#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

namespace cutline {

// App-wide preferences, stored as INI at <configDir>/cutline.ini (not in the Windows registry, so they're
// easy to inspect, back up and delete).
class AppSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(double uiScale READ uiScale WRITE setUiScale NOTIFY uiScaleChanged)
    Q_PROPERTY(bool showWelcome READ showWelcome WRITE setShowWelcome NOTIFY showWelcomeChanged)

public:
    explicit AppSettings(const QString& configDir, QObject* parent = nullptr);

    QSettings& raw() { return m_settings; }
    QString configDir() const { return m_configDir; }

    QString theme() const;
    void setTheme(const QString& theme);
    double uiScale() const;
    void setUiScale(double scale);
    bool showWelcome() const;
    void setShowWelcome(bool show);

signals:
    void themeChanged();
    void uiScaleChanged();
    void showWelcomeChanged();

private:
    QString m_configDir;
    QSettings m_settings;
};

} // namespace cutline
