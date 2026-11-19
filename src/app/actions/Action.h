#pragma once

#include "app/actions/Contexts.h"

#include <QKeySequence>
#include <QList>
#include <QObject>
#include <QString>

#include <functional>

namespace cutline {

struct ActionSpec {
    QString id;          // "file.save"
    QString category;    // "File"
    QString title;       // "Save Project"
    QString description;
    Contexts contexts;   // empty = everywhere
    QList<QKeySequence> defaultShortcuts;
    bool worksInTextEntry = false; // fires even while typing in a text field (Save, Open...)
    bool checkable = false;
    bool fixedShortcut = false;    // shown but not rebindable (Escape)
};

class ActionRegistry;

// One command the user can run from a menu, a button, a shortcut or (later) Claude.
// Only registered once its handler really works.
class Action : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString category READ category CONSTANT)
    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(bool checkable READ checkable CONSTANT)
    Q_PROPERTY(QString shortcutText READ shortcutText NOTIFY shortcutsChanged)
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY stateChanged)
    Q_PROPERTY(bool checked READ isChecked NOTIFY stateChanged)

public:
    Action(ActionSpec spec, std::function<void()> handler, QObject* parent = nullptr);

    const ActionSpec& spec() const { return m_spec; }
    QString id() const { return m_spec.id; }
    QString category() const { return m_spec.category; }
    QString title() const { return m_spec.title; }
    QString description() const { return m_spec.description; }
    bool checkable() const { return m_spec.checkable; }

    QList<QKeySequence> shortcuts() const { return m_shortcuts; }
    QString shortcutText() const;
    bool isEnabled() const { return !m_enabledWhen || m_enabledWhen(); }
    bool isChecked() const { return m_checkedWhen && m_checkedWhen(); }

    // Runs the handler. Returns false (and runs nothing) when the action is disabled.
    Q_INVOKABLE bool trigger();

    void setEnabledWhen(std::function<bool()> predicate);
    void setCheckedWhen(std::function<bool()> predicate);
    // Re-evaluates enabled/checked and notifies QML if they changed.
    void refresh();

signals:
    void triggered();
    void stateChanged();
    void shortcutsChanged();

private:
    friend class ActionRegistry;
    void setShortcuts(const QList<QKeySequence>& shortcuts);

    ActionSpec m_spec;
    std::function<void()> m_handler;
    std::function<bool()> m_enabledWhen;
    std::function<bool()> m_checkedWhen;
    QList<QKeySequence> m_shortcuts;
    bool m_lastEnabled = true;
    bool m_lastChecked = false;
};

} // namespace cutline
