import QtQuick
import Cutline

CDialog {
    id: dialog
    objectName: "dialog.error"
    property string message: ""

    function show(heading, text) {
        title = heading
        message = text
        open()
    }

    Row {
        spacing: Theme.space3
        width: parent.width - Theme.space4 * 2
        Icon { name: "warning"; color: Theme.warningColor; size: Theme.fontXl }
        Text {
            width: parent.width - Theme.fontXl - Theme.space3
            text: dialog.message
            wrapMode: Text.Wrap
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontMd
            color: Theme.text
        }
    }
    footer: [ CButton { text: qsTr("OK"); emphasized: true; onClicked: dialog.close() } ]
}
