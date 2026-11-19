#pragma once

#include <QAbstractListModel>
#include <QString>

#include <functional>

namespace cutline {

struct Sequence;

// The current sequence's tracks as the timeline shows them: video top-down (Vn…V1), then audio A1…An.
class TracksModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Role { IdRole = Qt::UserRole + 1, CodeRole, NameRole, KindRole, LockedRole, EnabledRole, SoloRole, SelectedRole };

    explicit TracksModel(QObject* parent = nullptr);

    // `sequence` returns the sequence to show (may be null); `selected` returns the selected track id.
    void setSources(std::function<const Sequence*()> sequence, std::function<QString()> selected);
    // Re-reads the sequence; emits row inserts/removes for single-track changes, otherwise data changes or a reset.
    void refresh();

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    struct Row {
        QString id, code, name, kind;
        bool locked = false, enabled = true, solo = false;
        friend bool operator==(const Row&, const Row&) = default;
    };
    QList<Row> read() const;

    std::function<const Sequence*()> m_sequence;
    std::function<QString()> m_selected;
    QList<Row> m_rows;
    QString m_selectedId;
};

} // namespace cutline
