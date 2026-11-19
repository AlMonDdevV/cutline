#include "app/theme/Fonts.h"

#include <QFontDatabase>
#include <QtTest>

using namespace cutline;
using namespace Qt::StringLiterals;

class TestFonts : public QObject {
    Q_OBJECT

private slots:
    void bundledFontsLoad() {
        QVERIFY(loadBundledFonts() >= 5);
        const QStringList families = QFontDatabase::families();
        QVERIFY(families.contains(u"Inter"_s));
        QVERIFY(families.contains(u"JetBrains Mono"_s));
    }
};

QTEST_MAIN(TestFonts)
#include "tst_fonts.moc"
