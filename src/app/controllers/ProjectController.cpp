#include "app/controllers/ProjectController.h"

#include "app/controllers/AppSettings.h"
#include "app/controllers/HistoryModel.h"
#include "app/controllers/RecentProjects.h"
#include "app/controllers/SequenceInspector.h"
#include "app/controllers/TracksModel.h"
#include "core/Commands.h"

namespace cutline {

ProjectController::ProjectController(AppSettings& settings, RecentProjects& recent, QObject* parent)
    : QObject(parent), m_settings(settings), m_recent(recent) {
    m_tracks = new TracksModel(this);
    m_history = new HistoryModel(this);
    m_sequence = new SequenceInspector(*this, this);
    setDocument(Document::createNew());
}

ProjectController::~ProjectController() {
    if (m_document)
        m_document->disconnect(this);
}

const Sequence* ProjectController::currentSequence() const {
    const Project& p = m_document->project();
    if (const Sequence* s = findSequence(p, p.defaultSequenceId))
        return s;
    return p.sequences.empty() ? nullptr : &p.sequences.front();
}

QString ProjectController::windowTitle() const {
    return QStringLiteral("%1%2 — CUTLINE").arg(projectName(), dirty() ? QStringLiteral("*") : QString());
}

QString ProjectController::localPath(const QUrl& url) { return url.isLocalFile() ? url.toLocalFile() : url.toString(); }

void ProjectController::setDocument(std::unique_ptr<Document> document) {
    if (m_document)
        m_document->disconnect(this);
    m_document = std::move(document);
    m_selectedTrackId.clear();

    connect(m_document.get(), &Document::changed, this, [this](const ChangeHint&) {
        if (!m_selectedTrackId.isEmpty() && currentSequence() && !findTrack(*currentSequence(), m_selectedTrackId)) {
            m_selectedTrackId.clear();
            emit selectionChanged();
        }
        m_tracks->refresh();
        m_sequence->refresh();
        emit projectChanged();
        emit titleChanged();
    });
    connect(m_document.get(), &Document::historyChanged, this, [this] {
        m_history->refresh();
        emit historyChanged();
    });
    connect(m_document.get(), &Document::dirtyChanged, this, [this] {
        emit dirtyChanged();
        emit titleChanged();
    });
    connect(m_document.get(), &Document::filePathChanged, this, &ProjectController::projectChanged);

    m_tracks->setSources([this] { return currentSequence(); }, [this] { return m_selectedTrackId; });
    m_history->setDocument(m_document.get());
    m_sequence->refresh();
    emit projectChanged();
    emit titleChanged();
    emit dirtyChanged();
    emit historyChanged();
    emit selectionChanged();
}

void ProjectController::newProject(const QString& preset) {
    Project project = makeDefaultProject();
    if (preset == QLatin1String("vertical")) {
        project.sequences.front().format.width = 1080;
        project.sequences.front().format.height = 1920;
    }
    setDocument(std::make_unique<Document>(std::move(project)));
    clearStatus();
}

bool ProjectController::openProject(const QUrl& file) {
    const QString path = localPath(file);
    auto opened = Document::open(path);
    if (!opened.ok()) {
        emit errorOccurred(tr("Couldn't open project"), opened.error().message);
        return false;
    }
    setDocument(std::move(opened).value());
    m_recent.add(path);
    clearStatus();
    return true;
}

bool ProjectController::save() {
    if (m_document->filePath().isEmpty()) {
        emit saveAsRequested();
        return false;
    }
    if (Status s = m_document->save(); !s) {
        emit errorOccurred(tr("Couldn't save project"), s.message);
        return false;
    }
    m_recent.add(m_document->filePath());
    return true;
}

bool ProjectController::saveAs(const QUrl& file) {
    QString path = localPath(file);
    if (!path.endsWith(QLatin1String(".cutline"), Qt::CaseInsensitive))
        path += QStringLiteral(".cutline");
    if (Status s = m_document->saveAs(path); !s) {
        emit errorOccurred(tr("Couldn't save project"), s.message);
        return false;
    }
    m_recent.add(path);
    emit projectChanged();
    return true;
}

void ProjectController::undo() { m_document->undo(); }
void ProjectController::redo() { m_document->redo(); }
void ProjectController::jumpToHistory(int row) { m_document->jumpTo(row); }

void ProjectController::setStatusMessage(const QString& message) {
    if (message == m_statusMessage)
        return;
    m_statusMessage = message;
    emit statusMessageChanged();
}

void ProjectController::clearStatus() { setStatusMessage({}); }

void ProjectController::setSelectedTrackId(const QString& id) {
    if (id == m_selectedTrackId)
        return;
    m_selectedTrackId = id;
    m_tracks->refresh();
    emit selectionChanged();
}

bool ProjectController::run(std::unique_ptr<Command> command) {
    const Status status = m_document->execute(std::move(command));
    if (!status) {
        setStatusMessage(status.message);
        return false;
    }
    return true;
}

static QString sequenceId(const ProjectController& p) {
    const Sequence* s = p.currentSequence();
    return s ? s->id : QString();
}

bool ProjectController::setProjectName(const QString& name) {
    if (name.trimmed() == projectName())
        return true;
    return run(makeRenameProject(name));
}

QString ProjectController::addTrack(const QString& kind) {
    const QString id = newId();
    const TrackKind k = kind == QLatin1String("audio") ? TrackKind::Audio : TrackKind::Video;
    return run(makeAddTrack(sequenceId(*this), k, id)) ? id : QString();
}

bool ProjectController::removeTrack(const QString& id) { return run(makeRemoveTrack(sequenceId(*this), id)); }

bool ProjectController::setTrackName(const QString& id, const QString& name) {
    const Sequence* s = currentSequence();
    const Track* t = s ? findTrack(*s, id) : nullptr;
    if (t && t->name == name.trimmed())
        return true;
    return run(makeSetTrackName(sequenceId(*this), id, name));
}

bool ProjectController::setTrackLocked(const QString& id, bool locked) {
    return run(makeSetTrackLocked(sequenceId(*this), id, locked));
}
bool ProjectController::setTrackEnabled(const QString& id, bool enabled) {
    return run(makeSetTrackEnabled(sequenceId(*this), id, enabled));
}
bool ProjectController::setTrackSolo(const QString& id, bool solo) {
    return run(makeSetTrackSolo(sequenceId(*this), id, solo));
}

QVariant ProjectController::trackField(const QString& id, const QString& field) const {
    const Sequence* s = currentSequence();
    const Track* t = s ? findTrack(*s, id) : nullptr;
    if (!t)
        return {};
    if (field == QLatin1String("name"))
        return t->name;
    if (field == QLatin1String("code"))
        return trackCode(*s, id);
    if (field == QLatin1String("kind"))
        return t->kind == TrackKind::Video ? QStringLiteral("video") : QStringLiteral("audio");
    if (field == QLatin1String("locked"))
        return t->locked;
    if (field == QLatin1String("enabled"))
        return t->enabled;
    if (field == QLatin1String("solo"))
        return t->solo;
    return {};
}

bool ProjectController::setSequenceSettings(const SequenceSettings& settings) {
    return run(makeSetSequenceSettings(sequenceId(*this), settings));
}

} // namespace cutline
