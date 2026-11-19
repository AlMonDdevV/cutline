import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import Cutline

ApplicationWindow {
    id: root

    width: 1600
    height: 900
    minimumWidth: 1024
    minimumHeight: 640
    visible: true
    title: Project.windowTitle
    color: Theme.window
    font.family: Theme.uiFont
    font.pixelSize: Theme.fontMd

    property bool screenshotMode: false
    property string startupProject: ""
    property bool forceQuit: false
    property var afterSave: null
    property var panelItems: ({})

    // ---- Panels and focus ----
    function registerPanel(id, item) { panelItems[id] = item }
    function unregisterPanel(id, item) { if (panelItems[id] === item) delete panelItems[id] }
    function currentPanelId() {
        for (const id in panelItems)
            if (panelItems[id] && panelItems[id].activeFocus)
                return id
        return ""
    }
    function focusPanel(id) {
        if (!id)
            return
        if (!Workspaces.isPanelVisible(id))
            Workspaces.showPanel(id)
        else
            Workspaces.setCurrentIn(Workspaces.areaOf(id), id)
        Qt.callLater(function() {
            const p = root.panelItems[id]
            if (!p)
                return
            p.forceActiveFocus()
            const fi = root.activeFocusItem
            if (fi && fi !== p && fi.cursorPosition !== undefined) {
                fi.focus = false
                p.forceActiveFocus()
            }
        })
    }
    function cyclePanel(step) { focusPanel(Workspaces.nextPanel(currentPanelId(), step)) }

    // ---- Unsaved changes ----
    function confirmDiscard(next) {
        if (!Project.dirty) {
            next()
            return
        }
        saveChanges.ask(function(choice) {
            if (choice === "discard") {
                next()
            } else if (choice === "save") {
                if (Project.filePath.length === 0) {
                    root.afterSave = next
                    saveDialog.open()
                } else if (Project.save()) {
                    next()
                }
            }
        })
    }

    // Every UI action in the ActionRegistry has a handler here (tests check this).
    readonly property var uiHandlers: ({
        "file.new": () => root.confirmDiscard(() => Project.newProject()),
        "file.open": () => root.confirmDiscard(() => openDialog.open()),
        "file.save": () => { if (Project.filePath.length === 0) saveDialog.open(); else Project.save() },
        "file.saveAs": () => saveDialog.open(),
        "app.quit": () => root.close(),
        "app.settings": () => settingsDialog.openAt("general"),
        "panel.next": () => root.cyclePanel(1),
        "panel.previous": () => root.cyclePanel(-1),
        "panel.nextFromCanvas": () => root.cyclePanel(1),
        "view.fullscreen": () => { if (root.visibility === Window.FullScreen) root.showNormal(); else root.showFullScreen() },
        "workspace.saveAs": () => saveWorkspace.open(),
        "help.shortcuts": () => settingsDialog.openAt("keyboard"),
        "help.about": () => about.open()
    })
    readonly property list<string> uiHandlerIds: Object.keys(uiHandlers)

    Connections {
        target: Actions
        function onUiRequested(id) {
            if (id.startsWith("focus:"))
                root.focusPanel(id.substring(6))
            else if (root.uiHandlers[id])
                root.uiHandlers[id]()
        }
    }
    Connections {
        target: Project
        function onErrorOccurred(heading, message) { errorDialog.show(heading, message) }
        function onSaveAsRequested() { saveDialog.open() }
    }

    onClosing: (close) => {
        if (Project.dirty && !root.forceQuit && !root.screenshotMode) {
            close.accepted = false
            root.confirmDiscard(() => { root.forceQuit = true; root.close() })
        }
    }

    Component.onCompleted: {
        if (startupProject.length > 0)
            Project.openPath(startupProject)
        else if (!screenshotMode && Settings.showWelcome)
            welcome.open()
    }

    menuBar: AppMenuBar {
        onOpenRecent: (path) => root.confirmDiscard(() => Project.openPath(path))
    }
    header: HeaderBar {}
    footer: StatusBar {}

    WorkspaceFrame { anchors.fill: parent }

    SaveChangesDialog { id: saveChanges }
    ErrorDialog { id: errorDialog }
    AboutDialog { id: about }
    SaveWorkspaceDialog { id: saveWorkspace }
    SettingsDialog {
        id: settingsDialog
        onImportThemeRequested: themeImportDialog.open()
        onExportThemeRequested: themeExportDialog.open()
    }
    WelcomeScreen {
        id: welcome
        onOpenRequested: openDialog.open()
        onRecentRequested: (path) => Project.openPath(path)
    }

    FileDialog {
        id: openDialog
        title: qsTr("Open Project")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("CUTLINE projects (*.cutline)")]
        onAccepted: Project.openProject(selectedFile)
    }
    FileDialog {
        id: saveDialog
        title: qsTr("Save Project As")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "cutline"
        nameFilters: [qsTr("CUTLINE projects (*.cutline)")]
        onAccepted: {
            if (Project.saveAs(selectedFile) && root.afterSave) {
                const next = root.afterSave
                root.afterSave = null
                next()
            }
        }
        onRejected: root.afterSave = null
    }
    FileDialog {
        id: themeImportDialog
        title: qsTr("Import Theme")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("CUTLINE themes (*.json)")]
        onAccepted: {
            const result = Theme.importTheme(selectedFile)
            if (!result.ok)
                errorDialog.show(qsTr("Couldn't import theme"), result.error)
            else if (result.warnings.length > 0)
                errorDialog.show(qsTr("Theme imported"), result.warnings.join("\n"))
        }
    }
    FileDialog {
        id: themeExportDialog
        title: qsTr("Export Theme")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        nameFilters: [qsTr("CUTLINE themes (*.json)")]
        onAccepted: {
            const error = Theme.exportTheme(selectedFile)
            if (error.length > 0)
                errorDialog.show(qsTr("Couldn't export theme"), error)
        }
    }
}
