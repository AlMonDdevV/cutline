import QtQuick
import QtQuick.Controls
import Cutline

Rectangle {
    id: bar
    objectName: "header.bar"
    signal saveWorkspaceRequested()

    implicitHeight: Theme.headerHeight + Theme.space2
    implicitWidth: leftGroup.implicitWidth + picker.implicitWidth + toggles.implicitWidth + Theme.space4 * 4
    color: Theme.panelRaised

    Row {
        id: leftGroup
        anchors.left: parent.left
        anchors.leftMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space2

        Image {
            source: "image://icons/cutline-mark?color=e5484d"
            sourceSize: Qt.size(Math.round(20 * Theme.scale), Math.round(20 * Theme.scale))
            anchors.verticalCenter: parent.verticalCenter
            Accessible.name: "CUTLINE"
        }
        Text {
            text: Project.projectName + (Project.dirty ? " *" : "")
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontMd
            font.weight: Font.DemiBold
            color: Theme.text
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: Project.sequence.name
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontSm
            color: Theme.textMuted
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    CButton {
        id: picker
        objectName: "header.workspacePicker"
        anchors.centerIn: parent
        text: qsTr("Workspace: %1").arg(Workspaces.current)
        iconName: "chevron-down"
        iconAfterText: true
        onClicked: workspaceMenu.popup(picker, 0, picker.height + 2)
        Accessible.name: qsTr("Workspace: %1").arg(Workspaces.current)

        CMenu {
            id: workspaceMenu
            Repeater {
                model: Workspaces.names
                CMenuItem {
                    required property string modelData
                    objectName: "workspaceitem." + modelData
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
    }

    Row {
        id: toggles
        anchors.right: parent.right
        anchors.rightMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space1

        Repeater {
            model: [ { id: "inspector", icon: "panel-inspector", action: "panel.inspector" },
                     { id: "history", icon: "panel-history", action: "panel.history" } ]
            CToolButton {
                required property var modelData
                objectName: "header.toggle." + modelData.id
                text: Workspaces.panelTitle(modelData.id)
                iconName: modelData.icon
                checked: Workspaces.revision >= 0 && Workspaces.isPanelVisible(modelData.id)
                tip: checked ? qsTr("Hide %1").arg(text) : (Actions.action(modelData.action).title
                     + (Actions.action(modelData.action).shortcutText ? " (" + Actions.action(modelData.action).shortcutText + ")" : ""))
                onClicked: checked ? Workspaces.hidePanel(modelData.id) : Actions.action(modelData.action).trigger()
            }
        }
    }

    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }
}
