import QtQuick
import Cutline

CDialog {
    id: dialog
    objectName: "dialog.shortcutConflict"
    title: qsTr("Shortcut already in use")

    property string message: ""
    property var onReassign: null

    function ask(text, reassign) {
        message = text
        onReassign = reassign
        open()
    }

    Text {
        objectName: "dialog.shortcutConflict.text"
        width: parent.width - Theme.space4 * 2
        text: dialog.message
        wrapMode: Text.Wrap
        font.family: Theme.uiFont
        font.pixelSize: Theme.fontMd
        color: Theme.text
    }
    footer: [
        CButton {
            objectName: "dialog.shortcutConflict.reassign"
            text: qsTr("Reassign")
            emphasized: true
            onClicked: { const cb = dialog.onReassign; dialog.onReassign = null; dialog.close(); if (cb) cb() }
        },
        CButton { text: qsTr("Cancel"); onClicked: { dialog.onReassign = null; dialog.close() } }
    ]
}
