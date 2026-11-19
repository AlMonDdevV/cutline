#pragma once

#include <QObject>
#include <QString>

namespace cutline {

// Timecode formatting for QML (the `Timecode` singleton).
class TimecodeHelper : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;

    Q_INVOKABLE QString format(qint64 frame, int num, int den, bool drop, int startHour) const;
    // Characters before this index are drawn dimmed (leading zeros).
    Q_INVOKABLE int significantIndex(const QString& timecode) const;
};

} // namespace cutline
