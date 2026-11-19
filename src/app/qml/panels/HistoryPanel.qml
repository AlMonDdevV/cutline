import QtQuick
import QtQuick.Controls
import Cutline

PanelFrame {
    id: panel
    panelId: "history"
    cutlinePanelContext: 64 // History

    ListView {
        id: list
        anchors.fill: parent
        anchors.topMargin: Theme.space1
        clip: true
        model: Project.history
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}

        delegate: ItemDelegate {
            id: entry
            required property int index
            required property string title
            required property bool isCurrent
            required property bool isUndone
            width: list.width
            height: Theme.rowHeight + Theme.space1
            focusPolicy: Qt.NoFocus
            onClicked: Project.jumpToHistory(index)
            Accessible.name: title

            contentItem: Row {
                spacing: Theme.space2
                Rectangle {
                    width: 3
                    height: Theme.fontLg
                    anchors.verticalCenter: parent.verticalCenter
                    color: entry.isCurrent ? Theme.accent : "transparent"
                }
                Text {
                    text: entry.title
                    font.family: Theme.uiFont
                    font.pixelSize: Theme.fontMd
                    font.weight: entry.isCurrent ? Font.DemiBold : Font.Normal
                    color: entry.isUndone ? Theme.textDisabled : Theme.text
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            background: Rectangle { color: entry.hovered ? Theme.hover : "transparent" }
        }
    }
}
