#include "app/theme/Palette.h"
#include "app/theme/ThemeController.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

QByteArray readResource(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return {};
    return f.readAll();
}

Palette builtin(const QString& name) {
    const auto r = parsePalette(readResource(u":/cutline/theme/%1.json"_s.arg(name)));
    if (!r.ok())
        qFatal("builtin palette %s failed: %s", qPrintable(name), qPrintable(r.error().message));
    return r.value();
}

double luminance(const QColor& c) {
    auto ch = [](double v) { return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4); };
    return 0.2126 * ch(c.redF()) + 0.7152 * ch(c.greenF()) + 0.0722 * ch(c.blueF());
}

} // namespace

class TestPalette : public QObject {
    Q_OBJECT

private slots:
    void themes_data() {
        QTest::addColumn<QString>("name");
        QTest::newRow("dark") << u"dark"_s;
        QTest::newRow("light") << u"light"_s;
    }

    void allKeysPresent_data() { themes_data(); }
    void allKeysPresent() {
        QFETCH(QString, name);
        const auto r = parsePalette(readResource(u":/cutline/theme/%1.json"_s.arg(name)));
        QVERIFY2(r.ok(), r.ok() ? "" : qPrintable(r.error().message));
        for (const QString& key : kPaletteKeys)
            QVERIFY2(r.value().colors.value(key).isValid(), qPrintable(key));
    }

    void missingKeyReportsName() {
        QJsonObject doc = QJsonDocument::fromJson(readResource(u":/cutline/theme/dark.json"_s)).object();
        QJsonObject colors = doc.value(u"colors").toObject();
        colors.remove(u"accent"_s);
        doc.insert(u"colors"_s, colors);
        const auto r = parsePalette(QJsonDocument(doc).toJson());
        QVERIFY(!r.ok());
        QCOMPARE(r.error().code, Error::Code::MissingField);
        QCOMPARE(r.error().message, u"Theme \"dark\" is missing colour \"accent\"."_s);
    }

    void textContrastAA_data() { themes_data(); }
    void textContrastAA() {
        QFETCH(QString, name);
        const Palette p = builtin(name);
        for (const auto* fg : {"text", "textMuted"})
            for (const auto* bg : {"window", "panel", "panelRaised", "field"}) {
                const double ratio = contrastRatio(p.color(QLatin1String(fg)), p.color(QLatin1String(bg)));
                QVERIFY2(ratio >= 4.5, qPrintable(u"%1 on %2 = %3"_s.arg(QLatin1String(fg), QLatin1String(bg)).arg(ratio)));
            }
    }

    void accentContrastUi_data() { themes_data(); }
    void accentContrastUi() {
        QFETCH(QString, name);
        const Palette p = builtin(name);
        for (const auto* fg : {"accent", "focusRing"})
            for (const auto* bg : {"panel", "panelRaised"}) {
                const double ratio = contrastRatio(p.color(QLatin1String(fg)), p.color(QLatin1String(bg)));
                QVERIFY2(ratio >= 3.0, qPrintable(u"%1 on %2 = %3"_s.arg(QLatin1String(fg), QLatin1String(bg)).arg(ratio)));
            }
    }

    void textOnAccentContrast_data() { themes_data(); }
    void textOnAccentContrast() {
        QFETCH(QString, name);
        const Palette p = builtin(name);
        QVERIFY(contrastRatio(p.color(u"textOnAccent"_s), p.color(u"accent"_s)) >= 4.5);
    }

    void dividerVisible_data() { themes_data(); }
    void dividerVisible() {
        QFETCH(QString, name);
        const Palette p = builtin(name);
        QVERIFY(contrastRatio(p.color(u"divider"_s), p.color(u"panel"_s)) >= 1.25);
    }

    void surfacesAreNeutral_data() { themes_data(); }
    void surfacesAreNeutral() {
        QFETCH(QString, name);
        const Palette p = builtin(name);
        for (const auto* key : {"window", "panel", "panelRaised", "field", "divider", "hover", "pressed",
                                "viewerSurround", "text", "textMuted", "textDisabled"}) {
            const QColor c = p.color(QLatin1String(key));
            QVERIFY2(c.red() == c.green() && c.green() == c.blue(), key);
        }
    }

    void viewerSurroundStaysDark() { QVERIFY(luminance(builtin(u"light"_s).color(u"viewerSurround"_s)) < 0.05); }

    void contrastKnownValues() {
        QVERIFY(qAbs(contrastRatio(Qt::black, Qt::white) - 21.0) < 0.01);
        QVERIFY(qAbs(contrastRatio(QColor(u"#1c1c1c"_s), QColor(u"#1c1c1c"_s)) - 1.0) < 1e-9);
    }

    void scaleClamps() {
        ThemeController theme(u"dark"_s, 1.0);
        theme.setScale(2.0);
        QCOMPARE(theme.scale(), 1.5);
        QCOMPARE(theme.fontMd(), qRound(13 * 1.5));
        theme.setScale(0.5);
        QCOMPARE(theme.scale(), 0.9);
    }
};

QTEST_MAIN(TestPalette)
#include "tst_palette.moc"
