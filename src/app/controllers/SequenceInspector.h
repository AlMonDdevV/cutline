#pragma once

#include <QObject>
#include <QStringList>

#include <functional>

namespace cutline {

class ProjectController;
struct SequenceSettings;

// The Inspector's view of the current sequence's settings. Every setter is one undoable command.
class SequenceInspector : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY changed)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY changed)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY changed)
    Q_PROPERTY(int resolutionIndex READ resolutionIndex WRITE setResolutionIndex NOTIFY changed)
    Q_PROPERTY(int frameRateIndex READ frameRateIndex WRITE setFrameRateIndex NOTIFY changed)
    Q_PROPERTY(int sampleRateIndex READ sampleRateIndex WRITE setSampleRateIndex NOTIFY changed)
    Q_PROPERTY(int timecodeStartHour READ timecodeStartHour WRITE setTimecodeStartHour NOTIFY changed)
    Q_PROPERTY(QString summary READ summary NOTIFY changed)
    Q_PROPERTY(double fps READ fps NOTIFY changed)
    Q_PROPERTY(int frameRateNum READ frameRateNum NOTIFY changed)
    Q_PROPERTY(int frameRateDen READ frameRateDen NOTIFY changed)
    Q_PROPERTY(QStringList resolutionOptions READ resolutionOptions CONSTANT)
    Q_PROPERTY(QStringList frameRateOptions READ frameRateOptions CONSTANT)
    Q_PROPERTY(QStringList sampleRateOptions READ sampleRateOptions CONSTANT)

public:
    explicit SequenceInspector(ProjectController& project, QObject* parent = nullptr);

    QString name() const;
    void setName(const QString& name);
    int width() const;
    void setWidth(int width);
    int height() const;
    void setHeight(int height);
    int resolutionIndex() const;
    void setResolutionIndex(int index);
    int frameRateIndex() const;
    void setFrameRateIndex(int index);
    int sampleRateIndex() const;
    void setSampleRateIndex(int index);
    int timecodeStartHour() const;
    void setTimecodeStartHour(int hour);
    QString summary() const; // "1920 × 1080 · 30 fps"
    double fps() const;
    int frameRateNum() const;
    int frameRateDen() const;

    QStringList resolutionOptions() const;
    QStringList frameRateOptions() const;
    QStringList sampleRateOptions() const;

    void refresh() { emit changed(); }

signals:
    void changed();

private:
    bool apply(const std::function<void(SequenceSettings&)>& change);
    ProjectController& m_project;
};

} // namespace cutline
