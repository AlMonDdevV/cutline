#include "app/actions/ShortcutRouter.h"

#include <QKeyEvent>

namespace cutline {
namespace {

// Which digit (0-9) sits under this key on the number row, from the platform's native key codes.
std::optional<int> digitRowDigit(const QKeyEvent& e) {
#if defined(Q_OS_WIN)
    const quint32 scan = e.nativeScanCode();
    if (scan >= 0x02 && scan <= 0x0B)
        return scan == 0x0B ? 0 : int(scan - 0x01);
#elif defined(Q_OS_MACOS)
    switch (e.nativeVirtualKey()) {
    case 0x1D: return 0;
    case 0x12: return 1;
    case 0x13: return 2;
    case 0x14: return 3;
    case 0x15: return 4;
    case 0x17: return 5;
    case 0x16: return 6;
    case 0x1A: return 7;
    case 0x1C: return 8;
    case 0x19: return 9;
    default: break;
    }
#else
    const quint32 code = e.nativeScanCode(); // X11/xkb keycode = evdev + 8
    if (code >= 10 && code <= 19)
        return code == 19 ? 0 : int(code - 9);
#endif
    return std::nullopt;
}

bool isLoneModifier(int key) {
    return key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Meta ||
           key == Qt::Key_AltGr || key == Qt::Key_CapsLock || key == 0 || key == Qt::Key_unknown;
}

bool isShiftedSymbol(int key) {
    const bool printable = key >= Qt::Key_Space && key <= Qt::Key_AsciiTilde;
    const bool letter = key >= Qt::Key_A && key <= Qt::Key_Z;
    const bool digit = key >= Qt::Key_0 && key <= Qt::Key_9;
    return printable && !letter && !digit;
}

} // namespace

ShortcutRouter::ShortcutRouter(ActionRegistry& registry, QObject* parent) : QObject(parent), m_registry(registry) {}

void ShortcutRouter::attach(QQuickWindow* window) {
    if (window)
        window->installEventFilter(this);
}

void ShortcutRouter::pushModal(QObject* modal) {
    if (modal && !m_modals.contains(modal))
        m_modals << modal;
}

void ShortcutRouter::popModal(QObject* modal) { m_modals.removeAll(modal); }

void ShortcutRouter::beginOperation(const QString& name, std::function<void()> cancel) {
    m_operationName = name;
    m_cancelOperation = std::move(cancel);
}

void ShortcutRouter::endOperation() {
    m_operationName.clear();
    m_cancelOperation = nullptr;
}

QList<QKeyCombination> ShortcutRouter::candidates(const QKeyEvent& event) {
    int key = event.key();
    if (isLoneModifier(key))
        return {};
    Qt::KeyboardModifiers mods = event.modifiers() & ~Qt::KeypadModifier;
    if (key == Qt::Key_Backtab) {
        key = Qt::Key_Tab;
        mods |= Qt::ShiftModifier;
    }

    QList<QKeyCombination> out;
    if (mods.testFlag(Qt::ShiftModifier)) {
        if (const auto digit = digitRowDigit(event); digit && key != Qt::Key_0 + *digit)
            out << QKeyCombination(mods, Qt::Key(Qt::Key_0 + *digit));
    }
    out << QKeyCombination(mods, Qt::Key(key));
    if (mods == Qt::ShiftModifier && isShiftedSymbol(key))
        out << QKeyCombination(Qt::NoModifier, Qt::Key(key));
    return out;
}

QKeyCombination ShortcutRouter::primaryCombination(const QKeyEvent& event) {
    const auto list = candidates(event);
    return list.isEmpty() ? QKeyCombination() : list.first();
}

bool ShortcutRouter::isTextEntry(const QQuickItem* item) {
    if (!item)
        return false;
    if (item->property("cutlineTextEntry").toBool())
        return true;
    return item->flags().testFlag(QQuickItem::ItemAcceptsInputMethod) && !isRecorder(item);
}

bool ShortcutRouter::isRecorder(const QQuickItem* item) {
    return item && item->property("cutlineShortcutRecorder").toBool();
}

QQuickItem* ShortcutRouter::panelRoot(QQuickItem* item) {
    for (QQuickItem* i = item; i; i = i->parentItem())
        if (i->property("cutlinePanelContext").toInt() > 0)
            return i;
    return nullptr;
}

std::optional<Context> ShortcutRouter::panelContext(const QQuickItem* item) {
    if (QQuickItem* root = panelRoot(const_cast<QQuickItem*>(item)))
        return Context(root->property("cutlinePanelContext").toUInt());
    return std::nullopt;
}

bool ShortcutRouter::handleKey(QQuickWindow* window, QKeyEvent* event, bool dryRun) {
    QQuickItem* focus = window->activeFocusItem();
    const bool escape = event->key() == Qt::Key_Escape && event->modifiers() == Qt::NoModifier;

    if (m_cancelOperation) {
        if (!escape)
            return false;
        if (!dryRun) {
            auto cancel = m_cancelOperation;
            endOperation();
            cancel();
        }
        return true;
    }

    if (isRecorder(focus))
        return false;

    m_modals.removeAll(nullptr);
    if (!m_modals.isEmpty()) {
        if (!escape)
            return false;
        if (!dryRun)
            QMetaObject::invokeMethod(m_modals.last(), "close");
        return true;
    }

    const bool textEntry = isTextEntry(focus);
    if (textEntry && escape) {
        if (!dryRun) {
            QQuickItem* root = panelRoot(focus);
            if (focus->metaObject()->indexOfMethod("cancelEditing()") >= 0)
                QMetaObject::invokeMethod(focus, "cancelEditing"); // Escape reverts, then leaves the field
            focus->setFocus(false);
            if (root && !root->hasActiveFocus())
                root->forceActiveFocus();
            else if (!root)
                window->contentItem()->forceActiveFocus();
        }
        return true;
    }

    const auto panel = panelContext(focus);
    for (const QKeyCombination& combo : candidates(*event)) {
        const QList<Action*> matches = m_registry.matching(combo, panel, textEntry);
        if (matches.isEmpty())
            continue;
        if (!dryRun)
            matches.first()->trigger(); // a disabled action still consumes its key
        return true;
    }
    return false;
}

bool ShortcutRouter::eventFilter(QObject* watched, QEvent* event) {
    auto* window = qobject_cast<QQuickWindow*>(watched);
    if (!window)
        return QObject::eventFilter(watched, event);

    if (event->type() == QEvent::ShortcutOverride) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (handleKey(window, key, true)) {
            key->accept();
            return true;
        }
    } else if (event->type() == QEvent::KeyPress) {
        auto* key = static_cast<QKeyEvent*>(event);
        if (handleKey(window, key, false)) {
            key->accept();
            return true;
        }
    }
    return QObject::eventFilter(watched, event);
}

} // namespace cutline
