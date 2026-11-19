#include "app/theme/Fonts.h"

#include <QDirIterator>
#include <QFontDatabase>

namespace cutline {

int loadBundledFonts() {
    int loaded = 0;
    QDirIterator it(QStringLiteral(":/cutline/fonts"), {QStringLiteral("*.ttf"), QStringLiteral("*.otf")}, QDir::Files);
    while (it.hasNext()) {
        if (QFontDatabase::addApplicationFont(it.next()) >= 0)
            ++loaded;
    }
    return loaded;
}

} // namespace cutline
