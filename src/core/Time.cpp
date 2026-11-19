#include "core/Time.h"

#include <QRegularExpression>

#include <algorithm>
#include <cmath>

namespace cutline {
namespace {

std::int64_t floorDiv(std::int64_t a, std::int64_t b) {
    std::int64_t q = a / b;
    const std::int64_t r = a % b;
    if (r != 0 && ((r < 0) != (b < 0)))
        --q;
    return q;
}

std::int64_t floorMod(std::int64_t a, std::int64_t b) { return a - floorDiv(a, b) * b; }

struct DropParams {
    int dropFrames;       // labels skipped per minute (2 for 29.97, 4 for 59.94)
    int framesPerMinute;  // real frames in a dropping minute
    int framesPer10Min;   // real frames in ten minutes
};

DropParams dropParams(int nominal) {
    const int drop = nominal / 15;
    return {drop, nominal * 60 - drop, nominal * 600 - drop * 9};
}

} // namespace

Ticks framesToTicks(std::int64_t frames, FrameRate rate) {
    if (!rate.isValid())
        return 0;
    // ticks = frames * K / num, split so no intermediate overflows int64 (MSVC has no __int128).
    const std::int64_t K = kTicksPerSecond * rate.den;
    const std::int64_t q = floorDiv(frames, rate.num);
    const std::int64_t r = floorMod(frames, rate.num);
    return q * K + (r * K + rate.num / 2) / rate.num;
}

std::int64_t ticksToFrames(Ticks ticks, FrameRate rate) {
    if (!rate.isValid())
        return 0;
    const std::int64_t K = kTicksPerSecond * rate.den;
    const std::int64_t q = floorDiv(ticks, K);
    const std::int64_t r = floorMod(ticks, K);
    return q * rate.num + floorDiv(r * rate.num, K);
}

Ticks samplesToTicks(std::int64_t samples, int sampleRate) { return framesToTicks(samples, {sampleRate, 1}); }

int nominalFps(FrameRate rate) { return rate.isValid() ? std::max(1, int(std::lround(rate.fps()))) : 1; }

bool supportsDropFrame(FrameRate rate) {
    return rate.den == 1001 && (rate.num == 30000 || rate.num == 60000);
}

QString formatTimecode(std::int64_t frame, FrameRate rate, TimecodeMode mode, int startHour) {
    const int nominal = nominalFps(rate);
    const bool drop = mode == TimecodeMode::Drop && supportsDropFrame(rate);
    std::int64_t f = std::max<std::int64_t>(0, frame);

    if (drop) {
        const DropParams p = dropParams(nominal);
        const std::int64_t d = f / p.framesPer10Min;
        const std::int64_t m = f % p.framesPer10Min;
        f += std::int64_t(p.dropFrames) * 9 * d;
        if (m > p.dropFrames)
            f += std::int64_t(p.dropFrames) * ((m - p.dropFrames) / p.framesPerMinute);
    }

    const std::int64_t ff = f % nominal;
    const std::int64_t ss = (f / nominal) % 60;
    const std::int64_t mm = (f / (std::int64_t(nominal) * 60)) % 60;
    const std::int64_t hh = (f / (std::int64_t(nominal) * 3600) + startHour) % 24;
    const int frameDigits = nominal > 100 ? 3 : 2;

    return QStringLiteral("%1:%2:%3%4%5")
        .arg(hh, 2, 10, QLatin1Char('0'))
        .arg(mm, 2, 10, QLatin1Char('0'))
        .arg(ss, 2, 10, QLatin1Char('0'))
        .arg(drop ? QLatin1Char(';') : QLatin1Char(':'))
        .arg(ff, frameDigits, 10, QLatin1Char('0'));
}

std::optional<std::int64_t> parseTimecode(QStringView text, FrameRate rate, TimecodeMode mode, int startHour) {
    static const QRegularExpression re(QStringLiteral("^(\\d{1,2}):(\\d{2}):(\\d{2})[:;](\\d{2,3})$"));
    const auto match = re.matchView(text.trimmed());
    if (!match.hasMatch())
        return std::nullopt;

    const int nominal = nominalFps(rate);
    const bool drop = mode == TimecodeMode::Drop && supportsDropFrame(rate);
    const std::int64_t hh = match.capturedView(1).toLongLong() - startHour;
    const std::int64_t mm = match.capturedView(2).toLongLong();
    const std::int64_t ss = match.capturedView(3).toLongLong();
    const std::int64_t ff = match.capturedView(4).toLongLong();
    if (hh < 0 || mm >= 60 || ss >= 60 || ff >= nominal)
        return std::nullopt;

    std::int64_t frame = ((hh * 3600 + mm * 60 + ss) * nominal) + ff;
    if (drop) {
        const DropParams p = dropParams(nominal);
        if (ss == 0 && ff < p.dropFrames && mm % 10 != 0)
            return std::nullopt; // drop-frame never shows this label
        const std::int64_t totalMinutes = 60 * hh + mm;
        frame -= std::int64_t(p.dropFrames) * (totalMinutes - totalMinutes / 10);
    }
    return frame;
}

qsizetype timecodeSignificantIndex(QStringView timecode) {
    for (qsizetype i = 0; i < timecode.size(); ++i) {
        const QChar c = timecode[i];
        if (c.isDigit() && c != QLatin1Char('0'))
            return i;
    }
    return timecode.isEmpty() ? 0 : timecode.size() - 1;
}

} // namespace cutline
