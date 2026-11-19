#include "app/controllers/TracksModel.h"

#include "core/Project.h"

namespace cutline {

TracksModel::TracksModel(QObject* parent) : QAbstractListModel(parent) {}

void TracksModel::setSources(std::function<const Sequence*()> sequence, std::function<QString()> selected) {
    m_sequence = std::move(sequence);
    m_selected = std::move(selected);
    beginResetModel();
    m_rows = read();
    m_selectedId = m_selected ? m_selected() : QString();
    endResetModel();
}

QList<TracksModel::Row> TracksModel::read() const {
    QList<Row> video, audio;
    const Sequence* s = m_sequence ? m_sequence() : nullptr;
    if (!s)
        return {};
    for (const Track& t : s->tracks) {
        Row r{t.id, trackCode(*s, t.id), t.name, t.kind == TrackKind::Video ? QStringLiteral("video") : QStringLiteral("audio"),
              t.locked, t.enabled, t.solo};
        if (t.kind == TrackKind::Video)
            video.prepend(r);
        else
            audio.append(r);
    }
    return video + audio;
}

void TracksModel::refresh() {
    const QList<Row> next = read();
    const QString selected = m_selected ? m_selected() : QString();
    const bool selectionChanged = selected != m_selectedId;
    m_selectedId = selected;

    const auto ids = [](const QList<Row>& rows) {
        QStringList out;
        for (const Row& r : rows)
            out << r.id;
        return out;
    };
    const QStringList before = ids(m_rows), after = ids(next);

    if (before == after) {
        const bool contentChanged = next != m_rows;
        m_rows = next;
        if ((contentChanged || selectionChanged) && !m_rows.isEmpty())
            emit dataChanged(index(0), index(int(m_rows.size()) - 1));
        return;
    }
    if (after.size() == before.size() + 1) {
        for (int i = 0; i < after.size(); ++i) {
            QStringList withoutI = after;
            withoutI.removeAt(i);
            if (withoutI == before) {
                beginInsertRows({}, i, i);
                m_rows.insert(i, next.at(i));
                endInsertRows();
                m_rows = next; // codes of neighbours may have shifted
                emit dataChanged(index(0), index(int(m_rows.size()) - 1));
                return;
            }
        }
    }
    if (after.size() + 1 == before.size()) {
        for (int i = 0; i < before.size(); ++i) {
            QStringList withoutI = before;
            withoutI.removeAt(i);
            if (withoutI == after) {
                beginRemoveRows({}, i, i);
                m_rows.removeAt(i);
                endRemoveRows();
                m_rows = next;
                if (!m_rows.isEmpty())
                    emit dataChanged(index(0), index(int(m_rows.size()) - 1));
                return;
            }
        }
    }
    beginResetModel();
    m_rows = next;
    endResetModel();
}

int TracksModel::rowCount(const QModelIndex& parent) const { return parent.isValid() ? 0 : int(m_rows.size()); }

QVariant TracksModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_rows.size())
        return {};
    const Row& r = m_rows.at(index.row());
    switch (role) {
    case IdRole: return r.id;
    case CodeRole: return r.code;
    case NameRole:
    case Qt::DisplayRole: return r.name;
    case KindRole: return r.kind;
    case LockedRole: return r.locked;
    case EnabledRole: return r.enabled;
    case SoloRole: return r.solo;
    case SelectedRole: return r.id == m_selectedId;
    default: return {};
    }
}

QHash<int, QByteArray> TracksModel::roleNames() const {
    return {{IdRole, "id"},         {CodeRole, "code"},       {NameRole, "name"}, {KindRole, "kind"},
            {LockedRole, "locked"}, {EnabledRole, "enabled"}, {SoloRole, "solo"}, {SelectedRole, "selected"}};
}

} // namespace cutline
