import QtQuick
import QtQuick.Controls
import Cutline

Rectangle {
    id: bar
    objectName: "header.bar"
    signal saveWorkspaceRequested()

    implicitHeight: Theme.headerHeight + Theme.space2
    // Names elide, so only the logo counts towards the minimum width.
    implicitWidth: logo.width + picker.implicitWidth + toggles.implicitWidth + Theme.space4 * 5
    color: Theme.panelRaised

    // Logo, project name and sequence name share the space left of the workspace picker; long names elide.
    Item {
        id: leftGroup
        anchors.left: parent.left
        anchors.leftMargin: Theme.space3
        anchors.right: picker.left
        anchors.rightMargin: Theme.space4
        anchors.verticalCenter: parent.verticalCenter
        height: parent.height

        Image {
            id: logo
            source: "image://icons/cutline-mark?color=e5484d"
            sourceSize: Qt.size(Math.round(20 * Theme.scale), Math.round(20 * Theme.scale))
            width: sourceSize.width
            height: sourceSize.height
            anchors.verticalCenter: parent.verticalCenter
            Accessible.name: "CUTLINE"
        }
        Text {
            id: projectName
            objectName: "header.projectName"
            x: logo.width + Theme.space2
            width: Math.max(0, Math.min(implicitWidth, leftGroup.width - x))
            anchors.verticalCenter: parent.verticalCenter
            text: Project.projectName + (Project.dirty ? " *" : "")
            elide: Text.ElideRight
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontMd
            font.weight: Font.DemiBold
            color: Theme.text
        }
        Text {
            x: projectName.x + projectName.width + Theme.space2
            width: Math.max(0, Math.min(implicitWidth, leftGroup.width - x))
            visible: width >= Theme.space6 * 2
            anchors.verticalCenter: parent.verticalCenter
            text: Project.sequence.name
            elide: Text.ElideRight
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontSm
            color: Theme.textMuted
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
