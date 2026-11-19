#pragma once

#include <QAbstractListModel>

namespace cutline {

class Document;

// Row 0 is "Initial state", then one row per command. Rows after the current one are undone.
class HistoryModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int currentRow READ currentRow NOTIFY currentRowChanged)

public:
    enum Role { TitleRole = Qt::UserRole + 1, IsCurrentRole, IsUndoneRole };

    explicit HistoryModel(QObject* parent = nullptr);
    void setDocument(const Document* document);
    void refresh();

    int currentRow() const;
    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

signals:
    void currentRowChanged();

private:
    const Document* m_document = nullptr;
};

} // namespace cutline
