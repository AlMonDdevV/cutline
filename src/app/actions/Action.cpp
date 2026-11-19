#include "app/actions/Action.h"

namespace cutline {

Action::Action(ActionSpec spec, std::function<void()> handler, QObject* parent)
    : QObject(parent), m_spec(std::move(spec)), m_handler(std::move(handler)), m_shortcuts(m_spec.defaultShortcuts) {}

QString Action::shortcutText() const {
    return m_shortcuts.isEmpty() ? QString() : m_shortcuts.first().toString(QKeySequence::NativeText);
}

bool Action::trigger() {
    if (!isEnabled())
        return false;
    emit triggered();
    if (m_handler)
        m_handler();
    return true;
}

void Action::setEnabledWhen(std::function<bool()> predicate) {
    m_enabledWhen = std::move(predicate);
    refresh();
}

void Action::setCheckedWhen(std::function<bool()> predicate) {
    m_checkedWhen = std::move(predicate);
    refresh();
}

void Action::refresh() {
    const bool enabled = isEnabled();
    const bool checked = isChecked();
    if (enabled != m_lastEnabled || checked != m_lastChecked) {
        m_lastEnabled = enabled;
        m_lastChecked = checked;
        emit stateChanged();
    }
}

void Action::setShortcuts(const QList<QKeySequence>& shortcuts) {
    if (shortcuts == m_shortcuts)
        return;
    m_shortcuts = shortcuts;
    emit shortcutsChanged();
}

} // namespace cutline
