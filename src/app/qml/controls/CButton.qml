import QtQuick
import QtQuick.Controls
import Cutline

Button {
    id: control

    property string iconName: ""
    property bool emphasized: false
    property bool iconAfterText: false

    font.family: Theme.uiFont
    font.pixelSize: Theme.fontMd
    font.weight: emphasized ? Font.DemiBold : Font.Normal
    implicitHeight: Theme.rowHeight + Theme.space1
    leftPadding: Theme.space3
    rightPadding: Theme.space3
    focusPolicy: Qt.StrongFocus

    contentItem: Row {
        spacing: Theme.space2
        layoutDirection: control.iconAfterText ? Qt.RightToLeft : Qt.LeftToRight
        Icon {
            visible: control.iconName !== ""
            name: control.iconName
            color: control.enabled ? Theme.text : Theme.textDisabled
            size: Theme.fontLg
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            text: control.text
            font: control.font
            color: control.enabled ? Theme.text : Theme.textDisabled
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    background: Rectangle {
        radius: Theme.radius
        color: control.down ? Theme.pressed : (control.hovered ? Theme.hover : Theme.field)
        border.width: control.visualFocus || control.emphasized ? 1 : 1
        border.color: control.visualFocus ? Theme.focusRing : (control.emphasized ? Theme.textMuted : Theme.divider)
    }
}
