#include "app/controllers/HistoryModel.h"

#include "core/Document.h"

namespace cutline {

HistoryModel::HistoryModel(QObject* parent) : QAbstractListModel(parent) {}

void HistoryModel::setDocument(const Document* document) {
    beginResetModel();
    m_document = document;
    endResetModel();
    emit currentRowChanged();
}

void HistoryModel::refresh() {
    beginResetModel();
    endResetModel();
    emit currentRowChanged();
}

int HistoryModel::currentRow() const { return m_document ? m_document->historyIndex() : 0; }

int HistoryModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || !m_document)
        return 0;
    return m_document->historyCount() + 1;
}

QVariant HistoryModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || !m_document)
        return {};
    const int row = index.row();
    switch (role) {
    case TitleRole:
    case Qt::DisplayRole: return row == 0 ? QStringLiteral("Initial state") : m_document->historyTitle(row - 1);
    case IsCurrentRole: return row == m_document->historyIndex();
    case IsUndoneRole: return row > m_document->historyIndex();
    default: return {};
    }
}

QHash<int, QByteArray> HistoryModel::roleNames() const {
    return {{TitleRole, "title"}, {IsCurrentRole, "isCurrent"}, {IsUndoneRole, "isUndone"}};
}

} // namespace cutline
