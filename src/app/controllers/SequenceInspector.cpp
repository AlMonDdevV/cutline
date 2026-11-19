#include "app/controllers/SequenceInspector.h"

#include "app/controllers/ProjectController.h"
#include "core/Project.h"

#include <array>

namespace cutline {
namespace {

struct Preset {
    int width, height;
    const char* label;
};
// User decision: YouTube first, then vertical for Shorts/TikTok/Reels.
constexpr std::array<Preset, 6> kResolutions = {{
    {1920, 1080, "1920 × 1080 (YouTube HD)"},
    {3840, 2160, "3840 × 2160 (YouTube 4K)"},
    {1080, 1920, "1080 × 1920 (Vertical: Shorts, TikTok, Reels)"},
    {1280, 720, "1280 × 720 (HD)"},
    {2560, 1440, "2560 × 1440 (QHD)"},
    {4096, 2160, "4096 × 2160 (DCI 4K)"},
}};
constexpr std::array<FrameRate, 8> kFrameRates = {{
    {24000, 1001}, {24, 1}, {25, 1}, {30000, 1001}, {30, 1}, {50, 1}, {60000, 1001}, {60, 1},
}};
constexpr std::array<const char*, 8> kFrameRateLabels = {"23.976", "24", "25", "29.97", "30", "50", "59.94", "60"};
constexpr std::array<int, 3> kSampleRates = {44100, 48000, 96000};

} // namespace

SequenceInspector::SequenceInspector(ProjectController& project, QObject* parent) : QObject(parent), m_project(project) {}

static const Sequence* current(const ProjectController& p) { return p.currentSequence(); }

QString SequenceInspector::name() const { return current(m_project) ? current(m_project)->name : QString(); }
int SequenceInspector::width() const { return current(m_project) ? current(m_project)->format.width : 0; }
int SequenceInspector::height() const { return current(m_project) ? current(m_project)->format.height : 0; }
int SequenceInspector::timecodeStartHour() const { return current(m_project) ? current(m_project)->timecodeStartHour : 0; }

double SequenceInspector::fps() const { return current(m_project) ? current(m_project)->format.frameRate.fps() : 0; }
int SequenceInspector::frameRateNum() const { return current(m_project) ? current(m_project)->format.frameRate.num : 30; }
int SequenceInspector::frameRateDen() const { return current(m_project) ? current(m_project)->format.frameRate.den : 1; }

int SequenceInspector::resolutionIndex() const {
    for (size_t i = 0; i < kResolutions.size(); ++i)
        if (kResolutions[i].width == width() && kResolutions[i].height == height())
            return int(i);
    return int(kResolutions.size()); // Custom
}

int SequenceInspector::frameRateIndex() const {
    const Sequence* s = current(m_project);
    if (!s)
        return -1;
    for (size_t i = 0; i < kFrameRates.size(); ++i)
        if (kFrameRates[i] == s->format.frameRate)
            return int(i);
    return -1;
}

int SequenceInspector::sampleRateIndex() const {
    const Sequence* s = current(m_project);
    for (size_t i = 0; s && i < kSampleRates.size(); ++i)
        if (kSampleRates[i] == s->format.sampleRate)
            return int(i);
    return -1;
}

QString SequenceInspector::summary() const {
    const Sequence* s = current(m_project);
    if (!s)
        return {};
    const int fi = frameRateIndex();
    const QString rate = fi >= 0 ? QString::fromUtf8(kFrameRateLabels[size_t(fi)]) : QString::number(s->format.frameRate.fps(), 'g', 5);
    return QStringLiteral("%1 × %2 · %3 fps").arg(s->format.width).arg(s->format.height).arg(rate);
}

QStringList SequenceInspector::resolutionOptions() const {
    QStringList out;
    for (const Preset& p : kResolutions)
        out << QString::fromUtf8(p.label);
    out << QStringLiteral("Custom");
    return out;
}

QStringList SequenceInspector::frameRateOptions() const {
    QStringList out;
    for (const char* l : kFrameRateLabels)
        out << QString::fromUtf8(l);
    return out;
}

QStringList SequenceInspector::sampleRateOptions() const {
    return {QStringLiteral("44.1 kHz"), QStringLiteral("48 kHz"), QStringLiteral("96 kHz")};
}

bool SequenceInspector::apply(const std::function<void(SequenceSettings&)>& change) {
    const Sequence* s = current(m_project);
    if (!s)
        return false;
    SequenceSettings next = settingsOf(*s);
    change(next);
    return m_project.setSequenceSettings(next);
}

void SequenceInspector::setName(const QString& name) {
    if (name != this->name())
        apply([&](SequenceSettings& s) { s.name = name; });
}
void SequenceInspector::setWidth(int width) {
    if (width != this->width())
        apply([&](SequenceSettings& s) { s.format.width = width; });
}
void SequenceInspector::setHeight(int height) {
    if (height != this->height())
        apply([&](SequenceSettings& s) { s.format.height = height; });
}
void SequenceInspector::setResolutionIndex(int index) {
    if (index < 0 || index >= int(kResolutions.size()) || index == resolutionIndex())
        return;
    apply([&](SequenceSettings& s) {
        s.format.width = kResolutions[size_t(index)].width;
        s.format.height = kResolutions[size_t(index)].height;
    });
}
void SequenceInspector::setFrameRateIndex(int index) {
    if (index < 0 || index >= int(kFrameRates.size()) || index == frameRateIndex())
        return;
    apply([&](SequenceSettings& s) { s.format.frameRate = kFrameRates[size_t(index)]; });
}
void SequenceInspector::setSampleRateIndex(int index) {
    if (index < 0 || index >= int(kSampleRates.size()) || index == sampleRateIndex())
        return;
    apply([&](SequenceSettings& s) { s.format.sampleRate = kSampleRates[size_t(index)]; });
}
void SequenceInspector::setTimecodeStartHour(int hour) {
    if (hour != timecodeStartHour())
        apply([&](SequenceSettings& s) { s.timecodeStartHour = hour; });
}

} // namespace cutline
