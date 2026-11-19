#include "app/actions/ShortcutsModel.h"

#include "app/AppServices.h"
#include "app/actions/ActionRegistry.h"
#include "app/actions/ShortcutProfile.h"

namespace cutline {
namespace {

QString localPath(const QUrl& url) { return url.isLocalFile() ? url.toLocalFile() : url.toString(); }

QString keysText(const Action* a, QKeySequence::SequenceFormat format) {
    QStringList parts;
    for (const QKeySequence& seq : a->shortcuts())
        parts << seq.toString(format);
    return parts.join(QStringLiteral(" · "));
}

} // namespace

ShortcutsModel::ShortcutsModel(QObject* parent) : QAbstractListModel(parent) {
    if (AppServices* services = AppServices::instance()) {
        m_registry = &services->actions();
        m_store = &services->shortcutStore();
        connect(m_registry, &ActionRegistry::shortcutsChanged, this, &ShortcutsModel::rebuild);
    }
    rebuild();
}

ShortcutsModel::ShortcutsModel(ActionRegistry& registry, ShortcutStore& store, QObject* parent)
    : QAbstractListModel(parent), m_registry(&registry), m_store(&store) {
    connect(m_registry, &ActionRegistry::shortcutsChanged, this, &ShortcutsModel::rebuild);
    rebuild();
}

bool ShortcutsModel::matches(const Action* a) const {
    if (!m_category.isEmpty() && a->category() != m_category)
        return false;
    const QString needle = m_filter.trimmed().toLower();
    if (needle.isEmpty())
        return true;
    // Words match anywhere in the title, category or description...
    const QString words = QStringList{a->title(), a->category(), a->description()}.join(QLatin1Char('\n')).toLower();
    if (words.contains(needle))
        return true;
    // ...but keys match whole shortcuts, so "ctrl+s" doesn't also find Ctrl+Shift+Z.
    for (const QKeySequence& seq : a->shortcuts())
        if (seq.toString(QKeySequence::PortableText).toLower() == needle ||
            seq.toString(QKeySequence::NativeText).toLower() == needle)
            return true;
    return false;
}

void ShortcutsModel::rebuild() {
    beginResetModel();
    m_rows.clear();
    if (m_registry)
        for (Action* a : m_registry->all())
            if (matches(a))
                m_rows << a;
    endResetModel();
    emit profileChanged();
}

void ShortcutsModel::setFilter(const QString& filter) {
    if (filter == m_filter)
        return;
    m_filter = filter;
    rebuild();
    emit filterChanged();
}

void ShortcutsModel::setCategory(const QString& category) {
    const QString value = category == QLatin1String("All") ? QString() : category;
    if (value == m_category)
        return;
    m_category = value;
    rebuild();
    emit filterChanged();
}

QStringList ShortcutsModel::categories() const {
    QStringList out;
    if (m_registry)
        for (Action* a : m_registry->all())
            if (!out.contains(a->category()))
                out << a->category();
    return out;
}

QString ShortcutsModel::profileName() const { return m_registry ? m_registry->profile().name : QString(); }

QString ShortcutsModel::nativeText(const QString& portableSequence) const {
    return QKeySequence::fromString(portableSequence, QKeySequence::PortableText).toString(QKeySequence::NativeText);
}

QVariantList ShortcutsModel::conflictsFor(const QString& actionId, const QString& portableSequence) const {
    QVariantList out;
    const Action* target = m_registry ? m_registry->action(actionId) : nullptr;
    if (!target)
        return out;
    const QKeySequence seq = QKeySequence::fromString(portableSequence, QKeySequence::PortableText);
    for (const auto& c : m_registry->conflicts(seq, target->spec().contexts, actionId)) {
        const Action* other = m_registry->action(c.actionId);
        out << QVariantMap{{QStringLiteral("actionId"), c.actionId},
                           {QStringLiteral("title"), other->title()},
                           {QStringLiteral("contextsText"), contextsText(other->spec().contexts)}};
    }
    return out;
}

void ShortcutsModel::persist() {
    if (m_store && m_registry)
        m_store->saveActive(m_registry->profile());
}

bool ShortcutsModel::assign(const QString& actionId, const QString& portableSequence, bool replaceConflicts) {
    m_lastError.clear();
    Action* target = m_registry ? m_registry->action(actionId) : nullptr;
    if (!target) {
        m_lastError = QStringLiteral("That action no longer exists.");
        return false;
    }
    if (target->spec().fixedShortcut) {
        m_lastError = QStringLiteral("This shortcut can't be changed.");
        return false;
    }
    const QKeySequence seq = QKeySequence::fromString(portableSequence, QKeySequence::PortableText);
    if (seq.isEmpty()) {
        m_lastError = QStringLiteral("That isn't a valid shortcut.");
        return false;
    }
    const auto clashes = m_registry->conflicts(seq, target->spec().contexts, actionId);
    if (!clashes.isEmpty() && !replaceConflicts) {
        m_lastError = QStringLiteral("%1 is already used by “%2”.")
                          .arg(seq.toString(QKeySequence::NativeText), m_registry->action(clashes.first().actionId)->title());
        return false;
    }
    for (const auto& c : clashes) {
        QList<QKeySequence> remaining = m_registry->action(c.actionId)->shortcuts();
        remaining.removeAll(seq);
        if (Status s = m_registry->assign(c.actionId, remaining); !s) {
            m_lastError = s.message;
            return false;
        }
    }
    QList<QKeySequence> keys = target->shortcuts();
    keys.removeAll(seq);
    keys.prepend(seq);
    if (Status s = m_registry->assign(actionId, keys); !s) {
        m_lastError = s.message;
        return false;
    }
    persist();
    return true;
}

bool ShortcutsModel::clear(const QString& actionId) {
    m_lastError.clear();
    if (Status s = m_registry->assign(actionId, {}); !s) {
        m_lastError = s.message;
        return false;
    }
    persist();
    return true;
}

void ShortcutsModel::resetAction(const QString& actionId) {
    m_registry->resetToDefault(actionId);
    persist();
}

void ShortcutsModel::resetAll() {
    m_registry->resetAll();
    persist();
}

bool ShortcutsModel::importProfile(const QUrl& file) {
    m_lastError.clear();
    auto imported = m_store->importFrom(localPath(file));
    if (!imported.ok()) {
        m_lastError = imported.error().message;
        return false;
    }
    m_registry->applyProfile(imported.value());
    persist();
    return true;
}

bool ShortcutsModel::exportProfile(const QUrl& file) {
    m_lastError.clear();
    if (Status s = m_store->exportTo(m_registry->profile(), localPath(file)); !s) {
        m_lastError = s.message;
        return false;
    }
    return true;
}

int ShortcutsModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : int(m_rows.size()); }

QVariant ShortcutsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_rows.size())
        return {};
    const Action* a = m_rows.at(index.row());
    switch (role) {
    case ActionIdRole: return a->id();
    case TitleRole:
    case Qt::DisplayRole: return a->title();
    case CategoryRole: return a->category();
    case DescriptionRole: return a->description();
    case ShortcutsTextRole: return keysText(a, QKeySequence::NativeText);
    case ContextsTextRole: return contextsText(a->spec().contexts);
    case OverriddenRole: return m_registry->isOverridden(a->id());
    case FixedRole: return a->spec().fixedShortcut;
    default: return {};
    }
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const {
    return {{ActionIdRole, "actionId"},         {TitleRole, "title"},
            {CategoryRole, "category"},         {DescriptionRole, "description"},
            {ShortcutsTextRole, "shortcutsText"}, {ContextsTextRole, "contextsText"},
            {OverriddenRole, "overridden"},     {FixedRole, "fixed"}};
}

} // namespace cutline
