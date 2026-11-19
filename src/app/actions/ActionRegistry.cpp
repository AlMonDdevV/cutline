#include "app/actions/ActionRegistry.h"

namespace cutline {

ActionRegistry::ActionRegistry(QObject* parent) : QObject(parent) {}

Action* ActionRegistry::add(ActionSpec spec, std::function<void()> handler) {
    Q_ASSERT_X(!m_byId.contains(spec.id), "ActionRegistry::add", qPrintable(spec.id));
    Q_ASSERT_X(bool(handler), "ActionRegistry::add", "an action needs a working handler");
    auto* action = new Action(std::move(spec), std::move(handler), this);
    m_actions << action;
    m_byId.insert(action->id(), action);
    applyEffective(action);
    return action;
}

Action* ActionRegistry::addUi(ActionSpec spec) {
    const QString id = spec.id;
    m_uiIds << id;
    return add(std::move(spec), [this, id] { emit uiRequested(id); });
}

Action* ActionRegistry::action(const QString& id) const { return m_byId.value(id, nullptr); }

QList<ActionRegistry::Conflict> ActionRegistry::conflicts(const QKeySequence& sequence, Contexts contexts,
                                                          const QString& ignoreId) const {
    QList<Conflict> out;
    if (sequence.isEmpty())
        return out;
    for (Action* a : m_actions) {
        if (a->id() == ignoreId || !contextsOverlap(a->spec().contexts, contexts))
            continue;
        if (a->shortcuts().contains(sequence))
            out << Conflict{a->id(), sequence};
    }
    return out;
}

void ActionRegistry::applyEffective(Action* action) {
    const auto it = m_profile.overrides.constFind(action->id());
    if (it != m_profile.overrides.cend() && !action->spec().fixedShortcut)
        action->setShortcuts(it.value());
    else
        action->setShortcuts(action->spec().defaultShortcuts);
}

Status ActionRegistry::assign(const QString& id, const QList<QKeySequence>& sequences) {
    Action* a = action(id);
    if (!a)
        return Status::failure(QStringLiteral("That action no longer exists."));
    if (a->spec().fixedShortcut)
        return Status::failure(QStringLiteral("This shortcut can't be changed."));
    if (sequences == a->spec().defaultShortcuts)
        m_profile.overrides.remove(id);
    else
        m_profile.overrides.insert(id, sequences);
    applyEffective(a);
    emit shortcutsChanged();
    return Status::success();
}

void ActionRegistry::resetToDefault(const QString& id) {
    if (Action* a = action(id); a && m_profile.overrides.remove(id)) {
        applyEffective(a);
        emit shortcutsChanged();
    }
}

void ActionRegistry::resetAll() {
    m_profile.overrides.clear();
    for (Action* a : m_actions)
        applyEffective(a);
    emit shortcutsChanged();
}

void ActionRegistry::applyProfile(const ShortcutProfile& profile, int* ignoredUnknown) {
    m_profile = profile;
    int unknown = 0;
    for (auto it = profile.overrides.cbegin(); it != profile.overrides.cend(); ++it)
        if (!m_byId.contains(it.key()))
            ++unknown;
    if (ignoredUnknown)
        *ignoredUnknown = unknown;
    for (Action* a : m_actions)
        applyEffective(a);
    emit shortcutsChanged();
}

QList<Action*> ActionRegistry::matching(QKeyCombination combo, std::optional<Context> panel, bool textEntry) const {
    const QKeySequence sequence(combo);
    QList<Action*> scoped, global;
    for (Action* a : m_actions) {
        if (textEntry && !a->spec().worksInTextEntry)
            continue;
        if (!a->shortcuts().contains(sequence))
            continue;
        const Contexts contexts = a->spec().contexts;
        if (contexts == Contexts())
            global << a;
        else if (panel && contexts.testFlag(*panel))
            scoped << a;
    }
    return scoped + global;
}

void ActionRegistry::refreshAll() {
    for (Action* a : m_actions)
        a->refresh();
}

} // namespace cutline
