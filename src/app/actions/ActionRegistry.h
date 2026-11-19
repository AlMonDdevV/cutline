#pragma once

#include "app/actions/Action.h"
#include "app/actions/ShortcutProfile.h"
#include "core/Result.h"

#include <QHash>
#include <QKeyCombination>
#include <QObject>
#include <QStringList>

#include <optional>

namespace cutline {

// The single list of everything the user can do. Menus, tooltips, the shortcut page and the
// shortcut router all read from it.
class ActionRegistry : public QObject {
    Q_OBJECT

public:
    explicit ActionRegistry(QObject* parent = nullptr);

    Action* add(ActionSpec spec, std::function<void()> handler);
    // An action handled by the QML shell (dialogs, focus): triggering it emits uiRequested(id).
    Action* addUi(ActionSpec spec);

    Q_INVOKABLE cutline::Action* action(const QString& id) const;
    QList<Action*> all() const { return m_actions; }
    Q_INVOKABLE QStringList uiActionIds() const { return m_uiIds; }

    struct Conflict {
        QString actionId;
        QKeySequence sequence;
    };
    // Other actions already using `sequence` in contexts that overlap `contexts`.
    QList<Conflict> conflicts(const QKeySequence& sequence, Contexts contexts, const QString& ignoreId) const;

    Status assign(const QString& id, const QList<QKeySequence>& sequences);
    void resetToDefault(const QString& id);
    void resetAll();
    bool isOverridden(const QString& id) const { return m_profile.overrides.contains(id); }

    // Unknown action ids are kept (so a newer profile survives a round trip) and counted.
    void applyProfile(const ShortcutProfile& profile, int* ignoredUnknown = nullptr);
    ShortcutProfile profile() const { return m_profile; }

    // Actions whose shortcut is `combo` and that may fire with focus in `panel` (none = no panel).
    // Panel-scoped actions come before global ones. In text entry only worksInTextEntry actions match.
    QList<Action*> matching(QKeyCombination combo, std::optional<Context> panel, bool textEntry) const;

    void refreshAll();

signals:
    void uiRequested(const QString& id);
    void shortcutsChanged();

private:
    void applyEffective(Action* action);

    QList<Action*> m_actions;
    QHash<QString, Action*> m_byId;
    QStringList m_uiIds;
    ShortcutProfile m_profile;
};

} // namespace cutline
