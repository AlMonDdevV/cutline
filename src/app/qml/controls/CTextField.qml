import QtQuick
import QtQuick.Controls
import Cutline

// Text field that commits on Enter or when focus leaves, and reverts on Escape (the shortcut router
// calls cancelEditing()). Bind `value`; handle `commit(value)`.
TextField {
    id: field

    property string value: ""
    property string committedText: ""
    signal commit(string value)
    // Emitted after Enter, Escape or leaving the field (after any commit).
    signal editingDone()

    font.family: Theme.uiFont
    font.pixelSize: Theme.fontMd
    color: Theme.text
    selectionColor: Theme.selection
    selectedTextColor: Theme.text
    placeholderTextColor: Theme.textDisabled
    implicitHeight: Theme.rowHeight
    leftPadding: Theme.space2
    rightPadding: Theme.space2
    topPadding: 0
    bottomPadding: 0
    verticalAlignment: TextInput.AlignVCenter
    selectByMouse: true

    background: Rectangle {
        radius: Theme.radius
        color: Theme.field
        border.width: 1
        border.color: field.activeFocus ? Theme.focusRing : Theme.divider
    }

    Component.onCompleted: text = value
    onValueChanged: if (!activeFocus) text = value
    onActiveFocusChanged: {
        if (activeFocus) {
            committedText = text
        } else {
            Qt.callLater(function() { if (!field.activeFocus) field.text = field.value })
        }
    }
    onEditingFinished: {
        if (text !== committedText) {
            committedText = text
            field.commit(text)
        }
        field.editingDone()
    }

    function cancelEditing() { text = committedText }
}
