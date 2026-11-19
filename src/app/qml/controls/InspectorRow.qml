import QtQuick
import Cutline

// Label on the left, editor on the right, fixed row height: the inspector's grid.
Item {
    id: row
    property string label: ""
    default property alias editor: slot.data

    width: parent ? parent.width : 300
    implicitHeight: Theme.rowHeight + Theme.space1

    Text {
        id: caption
        x: Theme.space3
        width: Math.round(row.width * 0.36)
        anchors.verticalCenter: parent.verticalCenter
        text: row.label
        font.family: Theme.uiFont
        font.pixelSize: Theme.fontMd
        color: Theme.textMuted
        elide: Text.ElideRight
    }
    Item {
        id: slot
        anchors.left: caption.right
        anchors.right: parent.right
        anchors.rightMargin: Theme.space3
        anchors.verticalCenter: parent.verticalCenter
        height: Theme.rowHeight
    }
}
