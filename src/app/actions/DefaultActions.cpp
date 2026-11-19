#include "app/actions/DefaultActions.h"

#include "app/actions/ActionRegistry.h"
#include "app/controllers/ProjectController.h"
#include "app/layout/WorkspaceManager.h"
#include "app/theme/ThemeController.h"

namespace cutline {
namespace {

ActionSpec make(const char* id, const char* category, const char* title, QList<QKeySequence> keys = {},
                bool textEntry = false, Contexts contexts = {}) {
    ActionSpec s;
    s.id = QString::fromLatin1(id);
    s.category = QString::fromLatin1(category);
    s.title = QString::fromUtf8(title);
    s.defaultShortcuts = std::move(keys);
    s.worksInTextEntry = textEntry;
    s.contexts = contexts;
    return s;
}

QKeySequence key(const char* portable) { return QKeySequence::fromString(QString::fromLatin1(portable), QKeySequence::PortableText); }

} // namespace

void registerDefaultActions(ActionRegistry& a, ProjectController& project, WorkspaceManager& workspaces,
                            ThemeController& theme) {
    // File
    a.addUi(make("file.new", "File", "New Project", {key("Ctrl+N")}, true));
    a.addUi(make("file.open", "File", "Open Project…", {key("Ctrl+O")}, true));
    a.addUi(make("file.save", "File", "Save Project", {key("Ctrl+S")}, true));
    a.addUi(make("file.saveAs", "File", "Save Project As…", {key("Ctrl+Shift+S")}, true));
    a.addUi(make("app.quit", "File", "Quit CUTLINE", {key("Ctrl+Q")}, true));

    // Edit
    Action* undo = a.add(make("edit.undo", "Edit", "Undo", {key("Ctrl+Z")}), [&project] { project.undo(); });
    undo->setEnabledWhen([&project] { return project.canUndo(); });
    Action* redo = a.add(make("edit.redo", "Edit", "Redo", {key("Ctrl+Y"), key("Ctrl+Shift+Z")}), [&project] { project.redo(); });
    redo->setEnabledWhen([&project] { return project.canRedo(); });
    ActionSpec deselect = make("edit.deselect", "Edit", "Deselect", {QKeySequence(Qt::Key_Escape)});
    deselect.fixedShortcut = true;
    deselect.description = QStringLiteral("Clears the selection. Escape also closes dialogs and leaves text fields first.");
    a.add(deselect, [&project] { project.setSelectedTrackId({}); });
    a.addUi(make("app.settings", "Edit", "Settings…", {key("Ctrl+,")}, true));

    // Sequence
    a.add(make("sequence.addVideoTrack", "Sequence", "Add Video Track"), [&project] { project.addTrack(QStringLiteral("video")); });
    a.add(make("sequence.addAudioTrack", "Sequence", "Add Audio Track"), [&project] { project.addTrack(QStringLiteral("audio")); });
    Action* removeTrack = a.add(make("sequence.removeTrack", "Sequence", "Remove Selected Track"),
                                [&project] { project.removeTrack(project.selectedTrackId()); });
    removeTrack->setEnabledWhen([&project] { return !project.selectedTrackId().isEmpty(); });

    // View: panels
    auto showPanel = [&a, &workspaces](const QString& id) {
        workspaces.showPanel(id);
        emit a.uiRequested(QStringLiteral("focus:") + id);
    };
    a.add(make("panel.inspector", "View", "Show Inspector", {key("Shift+5")}), [showPanel] { showPanel(QStringLiteral("inspector")); });
    a.add(make("panel.history", "View", "Show History"), [showPanel] { showPanel(QStringLiteral("history")); });
    a.addUi(make("panel.next", "View", "Next Panel", {key("F6"), key("Ctrl+Tab")}, true));
    a.addUi(make("panel.previous", "View", "Previous Panel", {key("Shift+F6"), key("Ctrl+Shift+Tab")}, true));
    a.addUi(make("panel.nextFromCanvas", "View", "Next Panel (from Timeline or Viewer)", {QKeySequence(Qt::Key_Tab)}, false,
                 Contexts(Context::Timeline) | Context::Viewer));
#if defined(Q_OS_MACOS)
    a.addUi(make("view.fullscreen", "View", "Full Screen", {key("Ctrl+Meta+F")}, true));
#else
    a.addUi(make("view.fullscreen", "View", "Full Screen", {key("F11")}, true));
#endif

    // View: themes
    struct ThemeEntry {
        const char* id;
        const char* title;
        const char* mode;
    };
    for (const ThemeEntry& e : {ThemeEntry{"view.themeDark", "Dark Theme", "dark"}, ThemeEntry{"view.themeLight", "Light Theme", "light"},
                                ThemeEntry{"view.themeSystem", "Match System", "system"}}) {
        ActionSpec spec = make(e.id, "View", e.title);
        spec.checkable = true;
        const QString mode = QString::fromLatin1(e.mode);
        Action* action = a.add(spec, [&theme, mode] { theme.setMode(mode); });
        action->setCheckedWhen([&theme, mode] { return theme.mode() == mode; });
        QObject::connect(&theme, &ThemeController::modeChanged, action, &Action::refresh);
    }

    // Workspace
    a.addUi(make("workspace.saveAs", "Workspace", "Save Workspace As…"));
    a.add(make("workspace.reset", "Workspace", "Reset Workspace"), [&workspaces] { workspaces.resetCurrent(); });

    // Help
    a.addUi(make("help.shortcuts", "Help", "Keyboard Shortcuts…"));
    a.addUi(make("help.about", "Help", "About CUTLINE"));

    QObject::connect(&project, &ProjectController::historyChanged, &a, &ActionRegistry::refreshAll);
    QObject::connect(&project, &ProjectController::selectionChanged, &a, &ActionRegistry::refreshAll);
    a.refreshAll();
}

} // namespace cutline
