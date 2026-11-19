#include "app/actions/ShortcutRecorder.h"

#include "app/actions/ShortcutRouter.h"

#include <QKeyEvent>
#include <QKeySequence>

namespace cutline {

ShortcutRecorder::ShortcutRecorder(QQuickItem* parent) : QQuickItem(parent) {
    setProperty("cutlineShortcutRecorder", true);
    setFlag(QQuickItem::ItemIsFocusScope, false);
    setActiveFocusOnTab(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void ShortcutRecorder::setRecording(bool recording) {
    if (recording == m_recording)
        return;
    m_recording = recording;
    emit recordingChanged();
}

void ShortcutRecorder::keyPressEvent(QKeyEvent* event) {
    if (!m_recording) {
        QQuickItem::keyPressEvent(event);
        return;
    }
    event->accept();
    if (event->key() == Qt::Key_Escape && event->modifiers() == Qt::NoModifier) {
        setRecording(false);
        emit cancelled();
        return;
    }
    const QList<QKeyCombination> combos = ShortcutRouter::candidates(*event);
    if (combos.isEmpty())
        return; // lone modifier: keep waiting
    const QKeyCombination combo = combos.first();
    setRecording(false);
    emit captured(QKeySequence(combo).toString(QKeySequence::PortableText));
}

} // namespace cutline
