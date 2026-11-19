#pragma once

#include "core/Command.h"
#include "core/Project.h"
#include "core/Result.h"

#include <QObject>
#include <QString>

#include <memory>
#include <vector>

namespace cutline {

// An open project: its state, its undo history, its file and whether it has unsaved changes.
// Every change to the project goes through execute().
class Document : public QObject {
    Q_OBJECT

public:
    explicit Document(Project project, QString filePath = {}, QObject* parent = nullptr);
    ~Document() override;

    static std::unique_ptr<Document> createNew();
    static Result<std::unique_ptr<Document>> open(const QString& path);

    const Project& project() const { return m_project; }
    QString filePath() const { return m_filePath; }
    quint64 revision() const { return m_revision; }
    bool isDirty() const { return m_index != m_cleanIndex; }

    // On failure the project, history and revision are unchanged and the message says why.
    Status execute(std::unique_ptr<Command> command);

    bool canUndo() const { return m_index > 0; }
    bool canRedo() const { return m_index < int(m_history.size()); }
    QString undoTitle() const;
    QString redoTitle() const;
    void undo();
    void redo();

    int historyCount() const { return int(m_history.size()); }
    QString historyTitle(int index) const;
    // Number of commands currently applied (0 = the state the document was created or opened in).
    int historyIndex() const { return m_index; }
    void jumpTo(int index);

    Status save();
    Status saveAs(const QString& path);

signals:
    void changed(cutline::ChangeHint hint);
    void historyChanged();
    void dirtyChanged(bool dirty);
    void filePathChanged();

private:
    void afterChange(const ChangeHint& hint, bool wasDirty);

    Project m_project;
    QString m_filePath;
    std::vector<std::unique_ptr<Command>> m_history;
    int m_index = 0;
    int m_cleanIndex = 0; // -1 when the saved state is no longer reachable
    quint64 m_revision = 0;
};

} // namespace cutline
