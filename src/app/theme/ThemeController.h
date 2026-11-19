#pragma once

#include "app/theme/Palette.h"

#include <QColor>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>

namespace cutline {

class ThemeLibrary;

// Current colours, type sizes and spacing for the UI. Exposed to QML as the `Theme` singleton (Task 13).
class ThemeController : public QObject {
    Q_OBJECT

    Q_PROPERTY(QColor window READ window NOTIFY paletteChanged)
    Q_PROPERTY(QColor panel READ panel NOTIFY paletteChanged)
    Q_PROPERTY(QColor panelRaised READ panelRaised NOTIFY paletteChanged)
    Q_PROPERTY(QColor field READ field NOTIFY paletteChanged)
    Q_PROPERTY(QColor divider READ divider NOTIFY paletteChanged)
    Q_PROPERTY(QColor hover READ hover NOTIFY paletteChanged)
    Q_PROPERTY(QColor pressed READ pressed NOTIFY paletteChanged)
    Q_PROPERTY(QColor selection READ selection NOTIFY paletteChanged)
    Q_PROPERTY(QColor text READ text NOTIFY paletteChanged)
    Q_PROPERTY(QColor textMuted READ textMuted NOTIFY paletteChanged)
    Q_PROPERTY(QColor textDisabled READ textDisabled NOTIFY paletteChanged)
    Q_PROPERTY(QColor textOnAccent READ textOnAccent NOTIFY paletteChanged)
    Q_PROPERTY(QColor accent READ accent NOTIFY paletteChanged)
    Q_PROPERTY(QColor accentHover READ accentHover NOTIFY paletteChanged)
    Q_PROPERTY(QColor focusRing READ focusRing NOTIFY paletteChanged)
    Q_PROPERTY(QColor viewerSurround READ viewerSurround NOTIFY paletteChanged)
    Q_PROPERTY(QColor clipVideo READ clipVideo NOTIFY paletteChanged)
    Q_PROPERTY(QColor clipAudio READ clipAudio NOTIFY paletteChanged)
    Q_PROPERTY(QColor clipTitle READ clipTitle NOTIFY paletteChanged)
    Q_PROPERTY(QColor clipGenerator READ clipGenerator NOTIFY paletteChanged)
    Q_PROPERTY(QColor danger READ danger NOTIFY paletteChanged)
    Q_PROPERTY(QColor warningColor READ warningColor NOTIFY paletteChanged)
    Q_PROPERTY(QColor success READ success NOTIFY paletteChanged)
    Q_PROPERTY(QColor timecodeDim READ timecodeDim NOTIFY paletteChanged)
    Q_PROPERTY(bool dark READ dark NOTIFY paletteChanged)

    Q_PROPERTY(QString mode READ mode NOTIFY modeChanged)
    Q_PROPERTY(QStringList themeNames READ themeNames NOTIFY themesChanged)
    Q_PROPERTY(QString warning READ warning NOTIFY warningChanged)

    Q_PROPERTY(qreal scale READ scale WRITE setScale NOTIFY scaleChanged)
    Q_PROPERTY(int fontXs READ fontXs NOTIFY scaleChanged)
    Q_PROPERTY(int fontSm READ fontSm NOTIFY scaleChanged)
    Q_PROPERTY(int fontMd READ fontMd NOTIFY scaleChanged)
    Q_PROPERTY(int fontLg READ fontLg NOTIFY scaleChanged)
    Q_PROPERTY(int fontXl READ fontXl NOTIFY scaleChanged)
    Q_PROPERTY(int fontTimecode READ fontTimecode NOTIFY scaleChanged)
    Q_PROPERTY(int space1 READ space1 NOTIFY scaleChanged)
    Q_PROPERTY(int space2 READ space2 NOTIFY scaleChanged)
    Q_PROPERTY(int space3 READ space3 NOTIFY scaleChanged)
    Q_PROPERTY(int space4 READ space4 NOTIFY scaleChanged)
    Q_PROPERTY(int space6 READ space6 NOTIFY scaleChanged)
    Q_PROPERTY(int rowHeight READ rowHeight NOTIFY scaleChanged)
    Q_PROPERTY(int headerHeight READ headerHeight NOTIFY scaleChanged)
    Q_PROPERTY(int radius READ radius CONSTANT)
    Q_PROPERTY(QString uiFont READ uiFont CONSTANT)
    Q_PROPERTY(QString monoFont READ monoFont CONSTANT)

public:
    // `library` may be null (no custom themes). Mode: "dark" | "light" | "system" | "custom:<name>".
    explicit ThemeController(const QString& mode = QStringLiteral("dark"), qreal scale = 1.0,
                             ThemeLibrary* library = nullptr, QObject* parent = nullptr);

    const Palette& palette() const { return m_palette; }

    QColor window() const { return c("window"); }
    QColor panel() const { return c("panel"); }
    QColor panelRaised() const { return c("panelRaised"); }
    QColor field() const { return c("field"); }
    QColor divider() const { return c("divider"); }
    QColor hover() const { return c("hover"); }
    QColor pressed() const { return c("pressed"); }
    QColor selection() const { return c("selection"); }
    QColor text() const { return c("text"); }
    QColor textMuted() const { return c("textMuted"); }
    QColor textDisabled() const { return c("textDisabled"); }
    QColor textOnAccent() const { return c("textOnAccent"); }
    QColor accent() const { return c("accent"); }
    QColor accentHover() const { return c("accentHover"); }
    QColor focusRing() const { return c("focusRing"); }
    QColor viewerSurround() const { return c("viewerSurround"); }
    QColor clipVideo() const { return c("clipVideo"); }
    QColor clipAudio() const { return c("clipAudio"); }
    QColor clipTitle() const { return c("clipTitle"); }
    QColor clipGenerator() const { return c("clipGenerator"); }
    QColor danger() const { return c("danger"); }
    QColor warningColor() const { return c("warning"); }
    QColor success() const { return c("success"); }
    QColor timecodeDim() const { return c("timecodeDim"); }
    bool dark() const;

    QString mode() const { return m_mode; }
    Q_INVOKABLE void setMode(const QString& mode);
    QStringList themeNames() const;
    QString warning() const { return m_warning; }
    // Re-reads the custom theme list (after an import or removal).
    Q_INVOKABLE void refreshThemes();
    // Imports a palette JSON into the custom theme library and switches to it.
    // Returns {ok, name, warnings: [..], error}.
    Q_INVOKABLE QVariantMap importTheme(const QUrl& file);
    // Writes the current palette; returns "" or an error message.
    Q_INVOKABLE QString exportTheme(const QUrl& file) const;

    qreal scale() const { return m_scale; }
    void setScale(qreal scale);
    int fontXs() const { return s(11); }
    int fontSm() const { return s(12); }
    int fontMd() const { return s(13); }
    int fontLg() const { return s(15); }
    int fontXl() const { return s(20); }
    int fontTimecode() const { return s(22); }
    int space1() const { return s(4); }
    int space2() const { return s(8); }
    int space3() const { return s(12); }
    int space4() const { return s(16); }
    int space6() const { return s(24); }
    int rowHeight() const { return s(24); }
    int headerHeight() const { return s(32); }
    int radius() const { return 3; }
    QString uiFont() const { return QStringLiteral("Inter"); }
    QString monoFont() const { return QStringLiteral("JetBrains Mono"); }

    static Palette builtinPalette(const QString& name); // "dark" or "light"

signals:
    void paletteChanged();
    void modeChanged();
    void themesChanged();
    void warningChanged();
    void scaleChanged();

private:
    QColor c(const char* key) const { return m_palette.color(QLatin1String(key)); }
    int s(int base) const { return qRound(base * m_scale); }
    void applySystemScheme();
    void setPalette(const Palette& palette);
    void setWarning(const QString& warning);

    ThemeLibrary* m_library = nullptr;
    Palette m_palette;
    QString m_mode;
    QString m_warning;
    qreal m_scale = 1.0;
};

} // namespace cutline
