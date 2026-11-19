#pragma once

#include <QAbstractListModel>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

namespace cutline {

class Action;
class ActionRegistry;
class ShortcutStore;

// The Keyboard Shortcuts page: every registered action, searchable by name or key, with reassignment,
// conflict detection, reset and profile import/export. Every change is saved immediately.
class ShortcutsModel : public QAbstractListModel {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(QString category READ category WRITE setCategory NOTIFY filterChanged)
    Q_PROPERTY(QStringList categories READ categories CONSTANT)
    Q_PROPERTY(QString profileName READ profileName NOTIFY profileChanged)

public:
    enum Role {
        ActionIdRole = Qt::UserRole + 1, TitleRole, CategoryRole, DescriptionRole, ShortcutsTextRole,
        ContextsTextRole, OverriddenRole, FixedRole
    };

    // From QML: uses the running app's services.
    explicit ShortcutsModel(QObject* parent = nullptr);
    ShortcutsModel(ActionRegistry& registry, ShortcutStore& store, QObject* parent = nullptr);

    QString filter() const { return m_filter; }
    void setFilter(const QString& filter);
    QString category() const { return m_category; }
    void setCategory(const QString& category);
    QStringList categories() const;
    QString profileName() const;

    Q_INVOKABLE QVariantList conflictsFor(const QString& actionId, const QString& portableSequence) const;
    // Makes `portableSequence` the action's first shortcut. With replaceConflicts, removes it from clashing actions.
    Q_INVOKABLE bool assign(const QString& actionId, const QString& portableSequence, bool replaceConflicts);
    Q_INVOKABLE bool clear(const QString& actionId);
    Q_INVOKABLE void resetAction(const QString& actionId);
    Q_INVOKABLE void resetAll();
    Q_INVOKABLE bool importProfile(const QUrl& file);
    Q_INVOKABLE bool exportProfile(const QUrl& file);
    Q_INVOKABLE QString lastError() const { return m_lastError; }
    Q_INVOKABLE QString nativeText(const QString& portableSequence) const;

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void filterChanged();
    void profileChanged();

private:
    void rebuild();
    void persist();
    bool matches(const Action* action) const;

    ActionRegistry* m_registry = nullptr;
    ShortcutStore* m_store = nullptr;
    QList<Action*> m_rows;
    QString m_filter;
    QString m_category;
    QString m_lastError;
};

} // namespace cutline
