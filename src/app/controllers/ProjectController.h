#pragma once

#include "core/Document.h"

#include <QObject>
#include <QString>
#include <QUrl>

#include <memory>

namespace cutline {

class AppSettings;
class HistoryModel;
class RecentProjects;
class SequenceInspector;
class TracksModel;

// The open project as the UI sees it (the `Project` singleton). Every edit goes through the Document's commands.
class ProjectController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString projectName READ projectName NOTIFY projectChanged)
    Q_PROPERTY(int revision READ revision NOTIFY projectChanged)
    Q_PROPERTY(QString filePath READ filePath NOTIFY projectChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY titleChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY historyChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY historyChanged)
    Q_PROPERTY(QString undoText READ undoText NOTIFY historyChanged)
    Q_PROPERTY(QString redoText READ redoText NOTIFY historyChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString selectedTrackId READ selectedTrackId WRITE setSelectedTrackId NOTIFY selectionChanged)
    Q_PROPERTY(QObject* selectedTrack READ selectedTrackObject NOTIFY selectionChanged)
    Q_PROPERTY(cutline::TracksModel* tracks READ tracks CONSTANT)
    Q_PROPERTY(cutline::HistoryModel* history READ history CONSTANT)
    Q_PROPERTY(cutline::SequenceInspector* sequence READ sequence CONSTANT)

public:
    ProjectController(AppSettings& settings, RecentProjects& recent, QObject* parent = nullptr);
    ~ProjectController() override;

    Document& document() { return *m_document; }
    const Document& document() const { return *m_document; }
    const Sequence* currentSequence() const;

    QString projectName() const { return m_document->project().name; }
    int revision() const { return int(m_document->revision()); }
    QString filePath() const { return m_document->filePath(); }
    QString windowTitle() const;
    bool dirty() const { return m_document->isDirty(); }
    bool canUndo() const { return m_document->canUndo(); }
    bool canRedo() const { return m_document->canRedo(); }
    QString undoText() const { return m_document->undoTitle(); }
    QString redoText() const { return m_document->redoTitle(); }
    QString statusMessage() const { return m_statusMessage; }
    QString selectedTrackId() const { return m_selectedTrackId; }
    void setSelectedTrackId(const QString& id);
    QObject* selectedTrackObject() const { return nullptr; } // reserved

    TracksModel* tracks() const { return m_tracks; }
    HistoryModel* history() const { return m_history; }
    SequenceInspector* sequence() const { return m_sequence; }

    // "" or "youtube" = 1920×1080 30 fps; "vertical" = 1080×1920 30 fps (Shorts, TikTok, Reels).
    Q_INVOKABLE void newProject(const QString& preset = {});
    Q_INVOKABLE bool openProject(const QUrl& file);
    Q_INVOKABLE bool openPath(const QString& path) { return openProject(QUrl::fromLocalFile(path)); }
    Q_INVOKABLE bool save();
    Q_INVOKABLE bool saveAs(const QUrl& file);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void jumpToHistory(int row);
    Q_INVOKABLE void clearStatus();

    Q_INVOKABLE bool setProjectName(const QString& name);
    Q_INVOKABLE QString addTrack(const QString& kind);
    Q_INVOKABLE bool removeTrack(const QString& id);
    Q_INVOKABLE bool setTrackName(const QString& id, const QString& name);
    Q_INVOKABLE bool setTrackLocked(const QString& id, bool locked);
    Q_INVOKABLE bool setTrackEnabled(const QString& id, bool enabled);
    Q_INVOKABLE bool setTrackSolo(const QString& id, bool solo);
    // Selected track's fields for the Inspector ("" when none / unknown key).
    Q_INVOKABLE QVariant trackField(const QString& id, const QString& field) const;
    bool setSequenceSettings(const SequenceSettings& settings);

signals:
    void projectChanged();
    void titleChanged();
    void dirtyChanged();
    void historyChanged();
    void statusMessageChanged();
    void selectionChanged();
    void saveAsRequested();
    void errorOccurred(const QString& title, const QString& message);

private:
    void setDocument(std::unique_ptr<Document> document);
    bool run(std::unique_ptr<Command> command);
    void setStatusMessage(const QString& message);
    static QString localPath(const QUrl& url);

    AppSettings& m_settings;
    RecentProjects& m_recent;
    std::unique_ptr<Document> m_document;
    TracksModel* m_tracks = nullptr;
    HistoryModel* m_history = nullptr;
    SequenceInspector* m_sequence = nullptr;
    QString m_statusMessage;
    QString m_selectedTrackId;
};

} // namespace cutline
