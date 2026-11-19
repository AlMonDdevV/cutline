import QtQuick
import Cutline

// "Save changes to “X” before closing?" — used by New, Open, Quit and window close.
CDialog {
    id: dialog
    objectName: "dialog.saveChanges"
    title: qsTr("Save changes to “%1” before closing?").arg(Project.projectName)

    // Called with "save" | "discard" | "cancel".
    property var onChoice: null

    function ask(callback) {
        onChoice = callback
        open()
    }
    function choose(choice) {
        const cb = onChoice
        onChoice = null
        close()
        if (cb)
            cb(String(choice))
    }

    Text {
        width: parent.width - Theme.space4 * 2
        text: qsTr("If you don't save, your changes since the last save will be lost.")
        wrapMode: Text.Wrap
        font.family: Theme.uiFont
        font.pixelSize: Theme.fontMd
        color: Theme.textMuted
    }

    footer: [
        CButton { text: qsTr("Save"); emphasized: true; onClicked: dialog.choose("save") },
        CButton { text: qsTr("Don't Save"); onClicked: dialog.choose("discard") },
        CButton { text: qsTr("Cancel"); onClicked: dialog.choose("cancel") }
    ]
    onClosed: if (onChoice) { const cb = onChoice; onChoice = null; cb("cancel") }
}
