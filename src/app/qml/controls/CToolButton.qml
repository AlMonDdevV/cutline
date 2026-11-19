import QtQuick
import QtQuick.Controls
import Cutline

// Compact icon (+ optional label) button for headers and track controls.
ToolButton {
    id: control

    property string iconName: ""
    property color iconColor: checked ? Theme.text : Theme.icon
    property string tip: ""

    font.family: Theme.uiFont
    font.pixelSize: Theme.fontSm
    implicitHeight: Theme.rowHeight
    implicitWidth: text.length > 0 ? implicitContentWidth + leftPadding + rightPadding : Theme.rowHeight
    padding: Theme.space1
    leftPadding: text.length > 0 ? Theme.space2 : Theme.space1
    rightPadding: text.length > 0 ? Theme.space2 : Theme.space1
    focusPolicy: Qt.TabFocus

    ToolTip.visible: tip.length > 0 && hovered
    ToolTip.delay: 600
    ToolTip.text: tip
    Accessible.name: tip.length > 0 ? tip : text

    contentItem: Row {
        spacing: Theme.space2
        Icon {
            visible: control.iconName !== ""
            name: control.iconName
            color: control.enabled ? control.iconColor : Theme.textDisabled
            size: Math.round(Theme.fontMd * 1.15)
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            visible: control.text.length > 0
            text: control.text
            font: control.font
            color: control.enabled ? Theme.text : Theme.textDisabled
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    background: Rectangle {
        radius: Theme.radius
        color: control.down ? Theme.pressed : (control.checked ? Theme.pressed : (control.hovered ? Theme.hover : "transparent"))
        border.width: control.visualFocus ? 1 : 0
        border.color: Theme.focusRing
    }
}
