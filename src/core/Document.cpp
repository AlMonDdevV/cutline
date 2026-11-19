#include "core/Document.h"

#include "core/ProjectIO.h"

namespace cutline {

Document::Document(Project project, QString filePath, QObject* parent)
    : QObject(parent), m_project(std::move(project)), m_filePath(std::move(filePath)) {}

Document::~Document() = default;

std::unique_ptr<Document> Document::createNew() { return std::make_unique<Document>(makeDefaultProject()); }

Result<std::unique_ptr<Document>> Document::open(const QString& path) {
    auto loaded = io::loadProject(path);
    if (!loaded.ok())
        return loaded.error();
    return std::make_unique<Document>(std::move(loaded).value(), path);
}

QString Document::undoTitle() const { return canUndo() ? m_history[size_t(m_index - 1)]->title() : QString(); }
QString Document::redoTitle() const { return canRedo() ? m_history[size_t(m_index)]->title() : QString(); }

QString Document::historyTitle(int index) const {
    return index >= 0 && index < historyCount() ? m_history[size_t(index)]->title() : QString();
}

void Document::afterChange(const ChangeHint& hint, bool wasDirty) {
    ++m_revision;
    emit changed(hint);
    emit historyChanged();
    if (wasDirty != isDirty())
        emit dirtyChanged(isDirty());
}

Status Document::execute(std::unique_ptr<Command> command) {
    if (!command)
        return Status::failure(QStringLiteral("Nothing to do."));
    const bool wasDirty = isDirty();
    if (Status status = command->apply(m_project); !status)
        return status;

    if (canRedo()) {
        m_history.erase(m_history.begin() + m_index, m_history.end());
        if (m_cleanIndex > m_index)
            m_cleanIndex = -1;
    }
    const ChangeHint hint = command->hint();
    m_history.push_back(std::move(command));
    ++m_index;
    afterChange(hint, wasDirty);
    return Status::success();
}

void Document::undo() {
    if (!canUndo())
        return;
    const bool wasDirty = isDirty();
    Command& command = *m_history[size_t(m_index - 1)];
    command.revert(m_project);
    --m_index;
    afterChange(command.hint(), wasDirty);
}

void Document::redo() {
    if (!canRedo())
        return;
    const bool wasDirty = isDirty();
    Command& command = *m_history[size_t(m_index)];
    const Status status = command.apply(m_project);
    Q_ASSERT_X(status.ok, "Document::redo", qPrintable(status.message));
    if (!status)
        return;
    ++m_index;
    afterChange(command.hint(), wasDirty);
}

void Document::jumpTo(int index) {
    index = std::clamp(index, 0, historyCount());
    while (m_index > index)
        undo();
    while (m_index < index)
        redo();
}

Status Document::save() {
    if (m_filePath.isEmpty())
        return Status::failure(QStringLiteral("This project hasn't been saved yet."));
    return saveAs(m_filePath);
}

Status Document::saveAs(const QString& path) {
    if (Status status = io::saveProject(m_project, path); !status)
        return status;
    const bool wasDirty = isDirty();
    const bool pathChanged = path != m_filePath;
    m_filePath = path;
    m_cleanIndex = m_index;
    if (pathChanged)
        emit filePathChanged();
    if (wasDirty)
        emit dirtyChanged(false);
    return Status::success();
}

} // namespace cutline
