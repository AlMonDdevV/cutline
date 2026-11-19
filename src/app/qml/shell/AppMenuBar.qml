import QtQuick
import QtQuick.Controls
import Cutline

MenuBar {
    id: menuBar
    signal openRecent(string path)

    font.family: Theme.uiFont
    font.pixelSize: Theme.fontMd

    delegate: MenuBarItem {
        id: entry
        implicitHeight: Theme.rowHeight + Theme.space1
        leftPadding: Theme.space3
        rightPadding: Theme.space3
        contentItem: Text {
            text: entry.text.replace("&", "")
            font: entry.font
            color: Theme.text
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: Theme.radius
            color: entry.highlighted ? Theme.hover : "transparent"
        }
    }

    background: Rectangle {
        color: Theme.panelRaised
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }
    }

    CMenu {
        title: qsTr("&File")
        CMenuItem { actionId: "file.new" }
        CMenuItem { actionId: "file.open" }
        CMenu {
            id: recentMenu
            title: qsTr("Open Recent")
            Repeater {
                model: Recent.paths
                CMenuItem {
                    required property string modelData
                    titleOverride: Recent.displayName(modelData)
                    onTriggered: menuBar.openRecent(modelData)
                    ToolTip.visible: hovered
                    ToolTip.text: modelData
                }
            }
            CMenuItem {
                titleOverride: qsTr("No recent projects")
                enabled: false
                visible: Recent.paths.length === 0
                height: visible ? implicitHeight : 0
            }
        }
        MenuSeparator {}
        CMenuItem { actionId: "file.save" }
        CMenuItem { actionId: "file.saveAs" }
        MenuSeparator {}
        CMenuItem { actionId: "app.quit" }
    }
    CMenu {
        title: qsTr("&Edit")
        CMenuItem {
            actionId: "edit.undo"
            titleOverride: Project.canUndo ? qsTr("Undo %1").arg(Project.undoText) : qsTr("Undo")
        }
        CMenuItem {
            actionId: "edit.redo"
            titleOverride: Project.canRedo ? qsTr("Redo %1").arg(Project.redoText) : qsTr("Redo")
        }
        CMenuItem { actionId: "edit.deselect" }
        MenuSeparator {}
        CMenuItem { actionId: "app.settings" }
    }
    CMenu {
        title: qsTr("&Sequence")
        CMenuItem { actionId: "sequence.addVideoTrack" }
        CMenuItem { actionId: "sequence.addAudioTrack" }
        CMenuItem { actionId: "sequence.removeTrack" }
    }
    CMenu {
        title: qsTr("&View")
        CMenuItem { actionId: "panel.inspector" }
        CMenuItem { actionId: "panel.history" }
        MenuSeparator {}
        CMenuItem { actionId: "panel.next" }
        CMenuItem { actionId: "panel.previous" }
        MenuSeparator {}
        CMenu {
            title: qsTr("Theme")
            CMenuItem { actionId: "view.themeDark" }
            CMenuItem { actionId: "view.themeLight" }
            CMenuItem { actionId: "view.themeSystem" }
        }
        CMenuItem { actionId: "view.fullscreen" }
    }
    CMenu {
        title: qsTr("&Workspace")
        Repeater {
            model: Workspaces.names
            CMenuItem {
                required property string modelData
                titleOverride: modelData
                showsCheck: true
                isChecked: modelData === Workspaces.current
                onTriggered: Workspaces.switchTo(modelData)
            }
        }
        MenuSeparator {}
        CMenuItem { actionId: "workspace.saveAs" }
        CMenuItem { actionId: "workspace.reset" }
    }
    CMenu {
        title: qsTr("&Help")
        CMenuItem { actionId: "help.shortcuts" }
        CMenuItem { actionId: "help.about" }
    }
}
