#pragma once

namespace cutline {

class ActionRegistry;
class ProjectController;
class ThemeController;
class WorkspaceManager;

// Registers every action that works in this build (Milestone 2). Nothing is registered before it works.
void registerDefaultActions(ActionRegistry& actions, ProjectController& project, WorkspaceManager& workspaces,
                            ThemeController& theme);

} // namespace cutline
