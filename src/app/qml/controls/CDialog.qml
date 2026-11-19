import QtQuick
import QtQuick.Controls
import Cutline

// Base for every CUTLINE dialog: modal, themed, and registered with the shortcut router so Escape closes it
// and its keys never reach the editor behind it.
Popup {
    id: dialog

    property string title: ""
    default property alias content: body.data
    property alias footer: footerRow.data

    modal: true
    focus: true
    anchors.centerIn: Overlay.overlay
    padding: 0
    closePolicy: Popup.CloseOnEscape
    width: Math.min(480, (parent ? parent.width : 480) - Theme.space6 * 2)

    Overlay.modal: Rectangle { color: "#99000000" }

    background: Rectangle {
        color: Theme.panelRaised
        border.color: Theme.divider
        radius: Theme.radius * 2
    }

    contentItem: Column {
        spacing: 0
        Text {
            width: parent.width
            visible: dialog.title.length > 0
            text: dialog.title
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontLg
            font.weight: Font.DemiBold
            color: Theme.text
            leftPadding: Theme.space4
            rightPadding: Theme.space4
            topPadding: Theme.space4
            bottomPadding: Theme.space2
            wrapMode: Text.Wrap
        }
        Column {
            id: body
            width: parent.width
            padding: Theme.space4
            topPadding: dialog.title.length > 0 ? 0 : Theme.space4
            spacing: Theme.space3
        }
        Row {
            id: footerRow
            anchors.right: parent.right
            rightPadding: Theme.space4
            bottomPadding: Theme.space4
            spacing: Theme.space2
            layoutDirection: Qt.RightToLeft
        }
    }

    onOpened: Shortcuts.pushModal(dialog)
    onClosed: Shortcuts.popModal(dialog)
}
