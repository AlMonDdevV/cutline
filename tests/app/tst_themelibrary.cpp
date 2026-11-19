#include "app/theme/Fonts.h"
#include "app/theme/Palette.h"
#include "app/theme/ThemeController.h"
#include "app/theme/ThemeLibrary.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

namespace {

QByteArray readResource(const QString& path) {
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
}

// The dark palette renamed and with chosen colours overridden, written to `path`.
QString writeTheme(const QString& dir, const QString& name, const QHash<QString, QString>& overrides = {}) {
    QJsonObject doc = QJsonDocument::fromJson(readResource(u":/cutline/theme/dark.json"_s)).object();
    doc.insert(u"name"_s, name);
    QJsonObject colors = doc.value(u"colors").toObject();
    for (auto it = overrides.cbegin(); it != overrides.cend(); ++it)
        colors.insert(it.key(), it.value());
    doc.insert(u"colors"_s, colors);
    const QString path = dir + u"/incoming-"_s + name + u".json"_s;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly))
        qFatal("cannot write %s", qPrintable(path));
    f.write(QJsonDocument(doc).toJson());
    return path;
}

} // namespace

class TestThemeLibrary : public QObject {
    Q_OBJECT

private slots:
    void builtinsPassCheck() {
        for (const auto* name : {"dark", "light"}) {
            const auto p = parsePalette(readResource(u":/cutline/theme/%1.json"_s.arg(QLatin1String(name))));
            QVERIFY(p.ok());
            QVERIFY2(checkPalette(p.value()).warnings.isEmpty(), name);
        }
    }

    void importValidTheme() {
        QTemporaryDir lib, src;
        ThemeLibrary library(lib.path());
        const auto r = library.importFrom(writeTheme(src.path(), u"Midnight"_s));
        QVERIFY2(r.ok(), r.ok() ? "" : qPrintable(r.error().message));
        QVERIFY(r.value().warnings.isEmpty());
        QCOMPARE(library.customNames(), QStringList{u"Midnight"_s});
        QVERIFY(library.load(u"Midnight"_s).ok());
    }

    void importInvalidRejected() {
        QTemporaryDir lib, src;
        QJsonObject doc = QJsonDocument::fromJson(readResource(u":/cutline/theme/dark.json"_s)).object();
        doc.insert(u"name"_s, u"Broken"_s);
        QJsonObject colors = doc.value(u"colors").toObject();
        colors.remove(u"panel"_s);
        doc.insert(u"colors"_s, colors);
        const QString path = src.path() + u"/broken.json"_s;
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(QJsonDocument(doc).toJson());
        f.close();

        ThemeLibrary library(lib.path());
        const auto r = library.importFrom(path);
        QVERIFY(!r.ok());
        QCOMPARE(r.error().message, u"Theme \"Broken\" is missing colour \"panel\"."_s);
        QVERIFY(library.customNames().isEmpty());
    }

    void importDuplicateRejected() {
        QTemporaryDir lib, src;
        ThemeLibrary library(lib.path());
        const auto builtinClash = library.importFrom(writeTheme(src.path(), u"Dark"_s));
        QVERIFY(!builtinClash.ok());
        QCOMPARE(builtinClash.error().message, u"There's already a theme called “Dark”."_s);

        QVERIFY(library.importFrom(writeTheme(src.path(), u"Midnight"_s)).ok());
        const auto again = library.importFrom(writeTheme(src.path(), u"midnight"_s));
        QVERIFY(!again.ok());
        QCOMPARE(again.error().message, u"There's already a theme called “midnight”."_s);
    }

    void tintedThemeWarns() {
        QTemporaryDir lib, src;
        ThemeLibrary library(lib.path());
        const auto r = library.importFrom(writeTheme(src.path(), u"Navy"_s, {{u"panel"_s, u"#1C1C2C"_s}}));
        QVERIFY(r.ok());
        QVERIFY(r.value().warnings.contains(
            u"“Navy” tints the panels, which can affect how you judge colour in the viewer."_s));
    }

    void lowContrastWarns() {
        QTemporaryDir lib, src;
        ThemeLibrary library(lib.path());
        const auto r = library.importFrom(writeTheme(src.path(), u"Murky"_s, {{u"textMuted"_s, u"#333333"_s}}));
        QVERIFY(r.ok());
        QVERIFY(r.value().warnings.contains(u"Some text in “Murky” may be hard to read."_s));
    }

    void exportRoundTrip() {
        QTemporaryDir dir;
        const auto p = parsePalette(readResource(u":/cutline/theme/light.json"_s));
        QVERIFY(p.ok());
        ThemeLibrary library(dir.path());
        const QString out = dir.path() + u"/exported.json"_s;
        QVERIFY(library.exportTo(p.value(), out).ok);
        QFile f(out);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const auto back = parsePalette(f.readAll());
        QVERIFY(back.ok());
        QCOMPARE(back.value(), p.value());
    }

    void controllerUsesCustom() {
        QTemporaryDir lib, src;
        ThemeLibrary library(lib.path());
        QVERIFY(library.importFrom(writeTheme(src.path(), u"Midnight"_s, {{u"panel"_s, u"#101010"_s}})).ok());

        ThemeController theme(u"dark"_s, 1.0, &library);
        QVERIFY(theme.themeNames().contains(u"Midnight"_s));
        theme.setMode(u"custom:Midnight"_s);
        QCOMPARE(theme.panel(), QColor(u"#101010"_s));
        QVERIFY(theme.warning().isEmpty());

        theme.setMode(u"custom:Nope"_s);
        QCOMPARE(theme.panel(), QColor(u"#1C1C1C"_s));
        QVERIFY(!theme.warning().isEmpty());
    }
    void baseInheritance() {
        const auto r = parsePalette("{\"format\":\"cutline.theme\",\"version\":1,\"name\":\"Blue\",\"base\":\"dark\",\"colors\":{\"accent\":\"#3399FF\"}}");
        QVERIFY2(r.ok(), r.ok() ? "" : qPrintable(r.error().message));
        QCOMPARE(r.value().color(u"accent"_s), QColor(u"#3399FF"_s));
        QCOMPARE(r.value().color(u"panel"_s), QColor(u"#1C1C1C"_s));
        QCOMPARE(r.value().color(u"playhead"_s), QColor(u"#3399FF"_s)); // defaults to the accent
    }

    void newerThemeVersionRefused() {
        const auto r = parsePalette("{\"format\":\"cutline.theme\",\"version\":2,\"name\":\"Future\",\"base\":\"dark\",\"colors\":{}}");
        QVERIFY(!r.ok());
        QCOMPARE(r.error().message, u"“Future” was made for a newer version of CUTLINE."_s);
    }

    void unknownBaseRejected() {
        const auto r = parsePalette("{\"format\":\"cutline.theme\",\"version\":1,\"name\":\"X\",\"base\":\"sepia\",\"colors\":{}}");
        QVERIFY(!r.ok());
        QCOMPARE(r.error().message, u"Theme \"X\": base must be \"dark\" or \"light\"."_s);
    }

    void fontsAndMetricsParsed() {
        const auto r = parsePalette("{\"format\":\"cutline.theme\",\"version\":1,\"name\":\"Roomy\",\"base\":\"light\",             \"fonts\":{\"ui\":\"JetBrains Mono\",\"mono\":\"Inter\"},\"metrics\":{\"radius\":6,\"density\":1.1}}");
        QVERIFY2(r.ok(), r.ok() ? "" : qPrintable(r.error().message));
        QCOMPARE(r.value().uiFont, u"JetBrains Mono"_s);
        QCOMPARE(r.value().monoFont, u"Inter"_s);
        QCOMPARE(r.value().radius, 6);
        QCOMPARE(r.value().density, 1.1);
    }

    void metricsOutOfRangeRejected() {
        const auto r = parsePalette("{\"format\":\"cutline.theme\",\"version\":1,\"name\":\"Big\",\"base\":\"dark\",\"metrics\":{\"radius\":40}}");
        QVERIFY(!r.ok());
        QCOMPARE(r.error().message, u"Theme \"Big\": radius must be between 0 and 16."_s);
        const auto d = parsePalette("{\"format\":\"cutline.theme\",\"version\":1,\"name\":\"Big\",\"base\":\"dark\",\"metrics\":{\"density\":3}}");
        QVERIFY(!d.ok());
        QCOMPARE(d.error().message, u"Theme \"Big\": density must be between 0.8 and 1.25."_s);
    }

    void optionalTokensDefault() {
        const auto r = parsePalette(readResource(u":/cutline/theme/dark.json"_s));
        QVERIFY(r.ok());
        QCOMPARE(r.value().color(u"playhead"_s), r.value().color(u"accent"_s));
        QCOMPARE(r.value().color(u"waveform"_s), r.value().color(u"clipAudio"_s));
        QCOMPARE(r.value().color(u"icon"_s), r.value().color(u"textMuted"_s));
    }

    void controllerAppliesFontsAndMetrics() {
        loadBundledFonts();
        QTemporaryDir lib, src;
        const QString path = src.path() + u"/roomy.json"_s;
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write("{\"format\":\"cutline.theme\",\"version\":1,\"name\":\"Roomy\",\"base\":\"dark\",\"fonts\":{\"ui\":\"JetBrains Mono\"},\"metrics\":{\"radius\":6,\"density\":1.25}}");
        f.close();
        ThemeLibrary library(lib.path());
        QVERIFY(library.importFrom(path).ok());
        ThemeController theme(u"custom:Roomy"_s, 1.0, &library);
        QCOMPARE(theme.uiFont(), u"JetBrains Mono"_s);
        QCOMPARE(theme.monoFont(), u"JetBrains Mono"_s); // not set: default
        QCOMPARE(theme.radius(), 6);
        QCOMPARE(theme.rowHeight(), qRound(24 * 1.25));
        theme.setMode(u"dark"_s);
        QCOMPARE(theme.uiFont(), u"Inter"_s);
        QCOMPARE(theme.radius(), 3);
    }

    void liveReloadOnFileChange() {
        QTemporaryDir lib, src;
        ThemeLibrary library(lib.path());
        QVERIFY(library.importFrom(writeTheme(src.path(), u"Live"_s)).ok());
        ThemeController theme(u"custom:Live"_s, 1.0, &library);
        QCOMPARE(theme.panel(), QColor(u"#1C1C1C"_s));
        QFile f(library.filePath(u"Live"_s));
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write("{\"format\":\"cutline.theme\",\"version\":1,\"name\":\"Live\",\"base\":\"dark\",\"colors\":{\"panel\":\"#202020\"}}");
        f.close();
        QTRY_COMPARE(theme.panel(), QColor(u"#202020"_s));
    }
};

QTEST_MAIN(TestThemeLibrary)
#include "tst_themelibrary.moc"
