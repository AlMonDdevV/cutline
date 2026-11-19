import QtQuick
import QtQuick.Controls
import Cutline

ComboBox {
    id: control

    font.family: Theme.uiFont
    font.pixelSize: Theme.fontMd
    implicitHeight: Theme.rowHeight

    contentItem: Text {
        leftPadding: Theme.space2
        rightPadding: control.indicator.width + Theme.space2
        text: control.displayText
        font: control.font
        color: control.enabled ? Theme.text : Theme.textDisabled
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    indicator: Icon {
        x: control.width - width - Theme.space2
        y: (control.height - height) / 2
        name: "chevron-down"
        color: Theme.textMuted
        size: Theme.fontMd
    }

    background: Rectangle {
        radius: Theme.radius
        color: control.down ? Theme.pressed : (control.hovered ? Theme.hover : Theme.field)
        border.width: 1
        border.color: control.visualFocus ? Theme.focusRing : Theme.divider
    }

    delegate: ItemDelegate {
        id: row
        required property int index
        required property var modelData
        width: control.width
        height: Theme.rowHeight + Theme.space1
        highlighted: control.highlightedIndex === index
        contentItem: Text {
            text: row.modelData
            font: control.font
            color: Theme.text
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle { color: row.highlighted ? Theme.hover : "transparent" }
    }

    popup: Popup {
        y: control.height + 2
        width: control.width
        implicitHeight: Math.min(contentItem.implicitHeight + 2, 320)
        padding: 1
        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
        }
        background: Rectangle { color: Theme.panelRaised; border.color: Theme.divider; radius: Theme.radius }
    }
}
