#pragma once

#include <QString>
#include <QStringList>

#include <memory>

namespace cutline {

class ActionRegistry;
class AppInfo;
class AppSettings;
class PanelRegistry;
class ProjectController;
class RecentProjects;
class ShortcutRouter;
class ShortcutStore;
class ThemeController;
class ThemeLibrary;
class TimecodeHelper;
class WorkspaceManager;

// Owns every app-wide service. Created once in main() (and in UI tests) before QML loads; the QML
// singletons hand out these instances.
class AppServices {
public:
    explicit AppServices(const QString& configDir);
    ~AppServices();
    AppServices(const AppServices&) = delete;
    AppServices& operator=(const AppServices&) = delete;

    static AppServices* instance();

    AppSettings& settings();
    RecentProjects& recent();
    ThemeLibrary& themeLibrary();
    ThemeController& theme();
    ShortcutStore& shortcutStore();
    ActionRegistry& actions();
    ShortcutRouter& router();
    PanelRegistry& panels();
    WorkspaceManager& workspaces();
    ProjectController& project();
    TimecodeHelper& timecode();
    AppInfo& info();

    QStringList startupWarnings() const;
    // Saves the current shortcut profile (after a change on the Keyboard Shortcuts page).
    void saveShortcuts();

private:
    struct Impl;
    std::unique_ptr<Impl> d;
};

} // namespace cutline
