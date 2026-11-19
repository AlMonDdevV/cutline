#pragma once

#include <QQuickItem>
#include <QtQml/qqmlregistration.h>

namespace cutline {

// Captures the next key combination for the Keyboard Shortcuts page. While it has focus the
// ShortcutRouter passes every key to it, so recording Ctrl+S doesn't save the project.
class ShortcutRecorder : public QQuickItem {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool recording READ recording WRITE setRecording NOTIFY recordingChanged)

public:
    explicit ShortcutRecorder(QQuickItem* parent = nullptr);

    bool recording() const { return m_recording; }
    void setRecording(bool recording);

signals:
    void captured(const QString& portableText);
    void cancelled();
    void recordingChanged();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    bool m_recording = false;
};

} // namespace cutline
