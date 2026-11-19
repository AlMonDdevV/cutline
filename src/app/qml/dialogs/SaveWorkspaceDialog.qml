import QtQuick
import Cutline

CDialog {
    id: dialog
    objectName: "dialog.saveWorkspace"
    title: qsTr("Save workspace as")
    property string error: ""

    onOpened: { error = ""; nameField.text = ""; nameField.forceActiveFocus() }

    function save() {
        if (Workspaces.saveAs(nameField.text))
            close()
        else
            error = Workspaces.lastError()
    }

    CTextField {
        id: nameField
        width: parent.width - Theme.space4 * 2
        placeholderText: qsTr("Workspace name")
        onAccepted: dialog.save()
    }
    Text {
        visible: dialog.error.length > 0
        text: dialog.error
        font.family: Theme.uiFont
        font.pixelSize: Theme.fontSm
        color: Theme.danger
    }
    footer: [
        CButton { text: qsTr("Save"); emphasized: true; onClicked: dialog.save() },
        CButton { text: qsTr("Cancel"); onClicked: dialog.close() }
    ]
}
