import QtQuick
import QtQuick.Controls
import Cutline

Menu {
    id: menu
    font.family: Theme.uiFont
    font.pixelSize: Theme.fontMd
    padding: Theme.space1
    implicitWidth: Math.max(240, contentItem.implicitWidth + leftPadding + rightPadding)

    delegate: CMenuItem {}

    background: Rectangle {
        implicitWidth: 240
        color: Theme.panelRaised
        border.color: Theme.divider
        radius: Theme.radius
    }

    onOpened: Shortcuts.pushModal(menu)
    onClosed: Shortcuts.popModal(menu)
}
