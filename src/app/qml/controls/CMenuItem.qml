import QtQuick
import QtQuick.Controls
import Cutline

// A menu item driven by a registered action: title, enabled/checked state and shortcut text all come
// from the ActionRegistry. It never sets a Qt shortcut; the ShortcutRouter owns every key.
MenuItem {
    id: item

    property string actionId: ""
    property string titleOverride: ""
    property bool showsCheck: false
    property bool isChecked: false
    readonly property var act: actionId.length > 0 ? Actions.action(actionId) : null
    readonly property string shortcutText: act ? act.shortcutText : ""

    text: titleOverride.length > 0 ? titleOverride : (act ? act.title : "")
    enabled: act ? act.enabled : true
    objectName: actionId.length > 0 ? "menuitem." + actionId : ""
    implicitHeight: Theme.rowHeight + Theme.space1
    implicitWidth: Math.max(220, row.implicitWidth + leftPadding + rightPadding)
    leftPadding: Theme.space2
    rightPadding: Theme.space3
    font.family: Theme.uiFont
    font.pixelSize: Theme.fontMd

    onTriggered: if (act) act.trigger()

    contentItem: Item {
        implicitWidth: row.implicitWidth
        implicitHeight: row.implicitHeight
        Row {
            id: row
            anchors.verticalCenter: parent.verticalCenter
            spacing: Theme.space2
            Item {
                width: Theme.fontLg
                height: Theme.fontLg
                anchors.verticalCenter: parent.verticalCenter
                Rectangle {
                    anchors.centerIn: parent
                    width: Theme.space2
                    height: Theme.space2
                    radius: width / 2
                    color: Theme.accent
                    visible: (item.act && item.act.checkable && item.act.checked) || (item.showsCheck && item.isChecked)
                }
            }
            Text {
                text: item.text.replace("&", "")
                font: item.font
                color: item.enabled ? Theme.text : Theme.textDisabled
                anchors.verticalCenter: parent.verticalCenter
            }
            Item { width: Theme.space6; height: 1 }
        }
        Text {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: item.shortcutText
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontSm
            color: Theme.textMuted
            visible: text.length > 0
        }
    }

    background: Rectangle {
        radius: Theme.radius
        color: item.highlighted ? Theme.hover : "transparent"
    }
}
