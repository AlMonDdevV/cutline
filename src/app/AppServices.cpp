#include "app/AppServices.h"

#include "app/actions/ActionRegistry.h"
#include "app/actions/DefaultActions.h"
#include "app/actions/ShortcutProfile.h"
#include "app/actions/ShortcutRouter.h"
#include "app/controllers/AppInfo.h"
#include "app/controllers/AppSettings.h"
#include "app/controllers/ProjectController.h"
#include "app/controllers/RecentProjects.h"
#include "app/controllers/TimecodeHelper.h"
#include "app/layout/PanelRegistry.h"
#include "app/layout/WorkspaceManager.h"
#include "app/theme/ThemeController.h"
#include "app/theme/ThemeLibrary.h"

#include <QDir>

namespace cutline {
namespace {
AppServices* g_instance = nullptr;
}

struct AppServices::Impl {
    explicit Impl(const QString& configDir)
        : settings(configDir), recent(settings), themeLibrary(QDir(configDir).filePath(QStringLiteral("themes"))),
          theme(settings.theme(), settings.uiScale(), &themeLibrary), shortcutStore(configDir), router(actions),
          project(settings, recent) {
        panels.add({QStringLiteral("viewer"), QStringLiteral("Viewer"), Area::Viewer, Context::Viewer,
                    QStringLiteral("ViewerPanel"), {320, 180}});
        panels.add({QStringLiteral("inspector"), QStringLiteral("Inspector"), Area::Right, Context::Inspector,
                    QStringLiteral("InspectorPanel"), {240, 200}});
        panels.add({QStringLiteral("history"), QStringLiteral("History"), Area::Right, Context::History,
                    QStringLiteral("HistoryPanel"), {200, 120}});
        panels.add({QStringLiteral("timeline"), QStringLiteral("Timeline"), Area::Timeline, Context::Timeline,
                    QStringLiteral("TimelinePanel"), {360, 160}});
        workspaces = std::make_unique<WorkspaceManager>(panels, settings.raw());

        registerDefaultActions(actions, project, *workspaces, theme);
        QString shortcutWarning;
        actions.applyProfile(shortcutStore.loadActive(&shortcutWarning));
        if (!shortcutWarning.isEmpty())
            warnings << shortcutWarning;
        if (!workspaces->loadWarning().isEmpty())
            warnings << workspaces->loadWarning();
        if (!theme.warning().isEmpty())
            warnings << theme.warning();

        QObject::connect(&theme, &ThemeController::modeChanged, &settings, [this] { settings.setTheme(theme.mode()); });
        QObject::connect(&theme, &ThemeController::scaleChanged, &settings, [this] { settings.setUiScale(theme.scale()); });
        info = std::make_unique<AppInfo>(warnings);
    }

    AppSettings settings;
    RecentProjects recent;
    ThemeLibrary themeLibrary;
    ThemeController theme;
    ShortcutStore shortcutStore;
    ActionRegistry actions;
    ShortcutRouter router;
    PanelRegistry panels;
    std::unique_ptr<WorkspaceManager> workspaces;
    ProjectController project;
    TimecodeHelper timecode;
    QStringList warnings;
    std::unique_ptr<AppInfo> info;
};

AppServices::AppServices(const QString& configDir) {
    Q_ASSERT_X(!g_instance, "AppServices", "only one AppServices may exist");
    d = std::make_unique<Impl>(configDir);
    g_instance = this;
}

AppServices::~AppServices() {
    if (g_instance == this)
        g_instance = nullptr;
}

AppServices* AppServices::instance() { return g_instance; }

AppSettings& AppServices::settings() { return d->settings; }
RecentProjects& AppServices::recent() { return d->recent; }
ThemeLibrary& AppServices::themeLibrary() { return d->themeLibrary; }
ThemeController& AppServices::theme() { return d->theme; }
ShortcutStore& AppServices::shortcutStore() { return d->shortcutStore; }
ActionRegistry& AppServices::actions() { return d->actions; }
ShortcutRouter& AppServices::router() { return d->router; }
PanelRegistry& AppServices::panels() { return d->panels; }
WorkspaceManager& AppServices::workspaces() { return *d->workspaces; }
ProjectController& AppServices::project() { return d->project; }
TimecodeHelper& AppServices::timecode() { return d->timecode; }
AppInfo& AppServices::info() { return *d->info; }
QStringList AppServices::startupWarnings() const { return d->warnings; }
void AppServices::saveShortcuts() { d->shortcutStore.saveActive(d->actions.profile()); }

} // namespace cutline
