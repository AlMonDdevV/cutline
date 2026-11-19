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
};

QTEST_MAIN(TestThemeLibrary)
#include "tst_themelibrary.moc"
