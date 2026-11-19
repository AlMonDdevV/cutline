#pragma once

// Exposes AppServices' objects to QML as singletons without putting QML macros in the classes themselves.
#include "AppServices.h"
#include "actions/Action.h"
#include "actions/ActionRegistry.h"
#include "actions/ShortcutRouter.h"
#include "controllers/AppInfo.h"
#include "controllers/AppSettings.h"
#include "controllers/HistoryModel.h"
#include "controllers/ProjectController.h"
#include "controllers/RecentProjects.h"
#include "controllers/SequenceInspector.h"
#include "controllers/TimecodeHelper.h"
#include "controllers/TracksModel.h"
#include "layout/WorkspaceManager.h"
#include "theme/ThemeController.h"

#include <QJSEngine>
#include <QQmlEngine>
#include <QtQml/qqmlregistration.h>

namespace cutline::qml {

template <class T>
T* owned(T& object) {
    QJSEngine::setObjectOwnership(&object, QJSEngine::CppOwnership);
    return &object;
}

struct ActionsSingleton {
    Q_GADGET
    QML_FOREIGN(cutline::ActionRegistry)
    QML_NAMED_ELEMENT(Actions)
    QML_SINGLETON
public:
    static cutline::ActionRegistry* create(QQmlEngine*, QJSEngine*) { return owned(AppServices::instance()->actions()); }
};

struct ShortcutsSingleton {
    Q_GADGET
    QML_FOREIGN(cutline::ShortcutRouter)
    QML_NAMED_ELEMENT(Shortcuts)
    QML_SINGLETON
public:
    static cutline::ShortcutRouter* create(QQmlEngine*, QJSEngine*) { return owned(AppServices::instance()->router()); }
};

struct WorkspacesSingleton {
    Q_GADGET
    QML_FOREIGN(cutline::WorkspaceManager)
    QML_NAMED_ELEMENT(Workspaces)
    QML_SINGLETON
public:
    static cutline::WorkspaceManager* create(QQmlEngine*, QJSEngine*) { return owned(AppServices::instance()->workspaces()); }
};

struct ThemeSingleton {
    Q_GADGET
    QML_FOREIGN(cutline::ThemeController)
    QML_NAMED_ELEMENT(Theme)
    QML_SINGLETON
public:
    static cutline::ThemeController* create(QQmlEngine*, QJSEngine*) { return owned(AppServices::instance()->theme()); }
};

struct ProjectSingleton {
    Q_GADGET
    QML_FOREIGN(cutline::ProjectController)
    QML_NAMED_ELEMENT(Project)
    QML_SINGLETON
public:
    static cutline::ProjectController* create(QQmlEngine*, QJSEngine*) { return owned(AppServices::instance()->project()); }
};

struct SettingsSingleton {
    Q_GADGET
    QML_FOREIGN(cutline::AppSettings)
    QML_NAMED_ELEMENT(Settings)
    QML_SINGLETON
public:
    static cutline::AppSettings* create(QQmlEngine*, QJSEngine*) { return owned(AppServices::instance()->settings()); }
};

struct RecentSingleton {
    Q_GADGET
    QML_FOREIGN(cutline::RecentProjects)
    QML_NAMED_ELEMENT(Recent)
    QML_SINGLETON
public:
    static cutline::RecentProjects* create(QQmlEngine*, QJSEngine*) { return owned(AppServices::instance()->recent()); }
};

struct TimecodeSingleton {
    Q_GADGET
    QML_FOREIGN(cutline::TimecodeHelper)
    QML_NAMED_ELEMENT(Timecode)
    QML_SINGLETON
public:
    static cutline::TimecodeHelper* create(QQmlEngine*, QJSEngine*) { return owned(AppServices::instance()->timecode()); }
};

struct AppInfoSingleton {
    Q_GADGET
    QML_FOREIGN(cutline::AppInfo)
    QML_NAMED_ELEMENT(AppInfo)
    QML_SINGLETON
public:
    static cutline::AppInfo* create(QQmlEngine*, QJSEngine*) { return owned(AppServices::instance()->info()); }
};

// Types QML sees as property values (not creatable).
struct ActionForeign {
    Q_GADGET
    QML_FOREIGN(cutline::Action)
    QML_NAMED_ELEMENT(CutlineAction)
    QML_UNCREATABLE("Actions come from the ActionRegistry")
};
struct TracksModelForeign {
    Q_GADGET
    QML_FOREIGN(cutline::TracksModel)
    QML_ANONYMOUS
};
struct HistoryModelForeign {
    Q_GADGET
    QML_FOREIGN(cutline::HistoryModel)
    QML_ANONYMOUS
};
struct SequenceInspectorForeign {
    Q_GADGET
    QML_FOREIGN(cutline::SequenceInspector)
    QML_ANONYMOUS
};

} // namespace cutline::qml
