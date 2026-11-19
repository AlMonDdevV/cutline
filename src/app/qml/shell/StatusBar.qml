import QtQuick
import Cutline

Rectangle {
    id: bar
    implicitHeight: Theme.rowHeight
    color: Theme.panelRaised

    property var pendingWarnings: AppInfo.startupWarnings.slice()
    property string message: Project.statusMessage.length > 0 ? Project.statusMessage : warning
    property string warning: ""

    function nextWarning() {
        if (pendingWarnings.length > 0) {
            warning = pendingWarnings.shift()
            warningTimer.restart()
        } else {
            warning = ""
        }
    }
    Component.onCompleted: nextWarning()

    Timer { id: warningTimer; interval: 6000; onTriggered: bar.nextWarning() }
    Timer {
        id: clearTimer
        interval: 5000
        onTriggered: Project.clearStatus()
    }
    Connections {
        target: Project
        function onStatusMessageChanged() { if (Project.statusMessage.length > 0) clearTimer.restart() }
    }

    Rectangle { width: parent.width; height: 1; color: Theme.divider }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space2
        Icon {
            name: Project.dirty ? "record" : "info"
            color: Theme.textMuted
            size: Theme.fontSm
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            objectName: "status.saved"
            text: Project.dirty ? qsTr("Unsaved changes") : qsTr("Saved")
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontSm
            color: Theme.textMuted
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Row {
        anchors.centerIn: parent
        spacing: Theme.space2
        visible: bar.message.length > 0
        Icon {
            name: "warning"
            color: Theme.warningColor
            size: Theme.fontMd
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            objectName: "status.message"
            text: bar.message
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontSm
            color: Theme.text
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    Text {
        anchors.right: parent.right
        anchors.rightMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        text: "CUTLINE " + AppInfo.version
        font.family: Theme.uiFont
        font.pixelSize: Theme.fontSm
        color: Theme.textDisabled
    }
}
