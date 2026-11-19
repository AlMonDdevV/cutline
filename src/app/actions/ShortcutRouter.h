#pragma once

#include "app/actions/ActionRegistry.h"

#include <QKeyCombination>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QQuickWindow>

#include <functional>
#include <optional>

class QKeyEvent;

namespace cutline {

// Routes every key press in CUTLINE's windows to the right action, in this order:
//   1. an operation in progress (drag/trim): Escape cancels it
//   2. a shortcut recorder: gets every key
//   3. a modal dialog or popup: Escape closes it, other keys go to it
//   4. a text field: keys type; Escape leaves the field; only "works in text entry" actions fire
//   5. actions scoped to the focused panel, then global actions
class ShortcutRouter : public QObject {
    Q_OBJECT

public:
    explicit ShortcutRouter(ActionRegistry& registry, QObject* parent = nullptr);

    void attach(QQuickWindow* window);

    // Dialogs and popups register themselves while open. `modal` must have an invokable close().
    Q_INVOKABLE void pushModal(QObject* modal);
    Q_INVOKABLE void popModal(QObject* modal);

    void beginOperation(const QString& name, std::function<void()> cancel);
    void endOperation();
    bool operationActive() const { return bool(m_cancelOperation); }

    // Normalised key combinations for an event, most specific first: digit-row keys with Shift become
    // "Shift+<digit>" whatever the layout prints, Backtab becomes Shift+Tab, the keypad flag is dropped,
    // and a shifted symbol also matches without Shift. Empty for a lone modifier key.
    static QList<QKeyCombination> candidates(const QKeyEvent& event);
    static QKeyCombination primaryCombination(const QKeyEvent& event);

    static bool isTextEntry(const QQuickItem* item);
    static bool isRecorder(const QQuickItem* item);
    static std::optional<Context> panelContext(const QQuickItem* item);
    static QQuickItem* panelRoot(QQuickItem* item);

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    bool handleKey(QQuickWindow* window, QKeyEvent* event, bool dryRun);

    ActionRegistry& m_registry;
    QList<QPointer<QObject>> m_modals;
    std::function<void()> m_cancelOperation;
    QString m_operationName;
};

} // namespace cutline
