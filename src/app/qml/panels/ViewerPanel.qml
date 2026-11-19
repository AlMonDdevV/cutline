import QtQuick
import Cutline

PanelFrame {
    id: panel
    panelId: "viewer"
    cutlinePanelContext: 2 // Viewer

    readonly property var seq: Project.sequence

    Rectangle {
        id: bar
        anchors.left: parent.left
        anchors.right: parent.right
        height: Theme.headerHeight - Theme.space1
        color: Theme.panelRaised
        Text {
            anchors.left: parent.left
            anchors.leftMargin: Theme.space3
            anchors.verticalCenter: parent.verticalCenter
            text: panel.seq.name
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontMd
            font.weight: Font.Medium
            color: Theme.text
        }
        Text {
            anchors.right: parent.right
            anchors.rightMargin: Theme.space3
            anchors.verticalCenter: parent.verticalCenter
            text: panel.seq.summary
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontSm
            color: Theme.textMuted
        }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }
    }

    Rectangle {
        id: surround
        anchors.top: bar.bottom
        anchors.bottom: transport.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: Theme.viewerSurround

        Rectangle {
            id: canvas
            readonly property real aspect: panel.seq.height > 0 ? panel.seq.width / panel.seq.height : 16 / 9
            readonly property real maxW: surround.width - Theme.space4 * 2
            readonly property real maxH: surround.height - Theme.space4 * 2
            width: Math.max(0, Math.min(maxW, maxH * aspect))
            height: Math.max(0, width / aspect)
            anchors.centerIn: parent
            color: "#000000"

            Text {
                anchors.centerIn: parent
                width: parent.width - Theme.space4
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                text: qsTr("No media in this project yet.")
                font.family: Theme.uiFont
                font.pixelSize: Theme.fontMd
                color: "#A0A0A0"
            }
        }
    }

    Rectangle {
        id: transport
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: Theme.headerHeight + Theme.space3
        color: Theme.panel
        Rectangle { width: parent.width; height: 1; color: Theme.divider }
        TimecodeLabel {
            anchors.centerIn: parent
            timecode: Timecode.format(0, panel.seq.frameRateNum, panel.seq.frameRateDen, false, panel.seq.timecodeStartHour)
        }
    }
}
