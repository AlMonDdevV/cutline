#include "app/controllers/TimecodeHelper.h"

#include "core/Time.h"

namespace cutline {

QString TimecodeHelper::format(qint64 frame, int num, int den, bool drop, int startHour) const {
    return formatTimecode(frame, {num, den}, drop ? TimecodeMode::Drop : TimecodeMode::NonDrop, startHour);
}

int TimecodeHelper::significantIndex(const QString& timecode) const { return int(timecodeSignificantIndex(timecode)); }

} // namespace cutline
