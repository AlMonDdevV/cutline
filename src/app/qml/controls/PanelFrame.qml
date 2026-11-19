import QtQuick
import QtQuick.Controls
import Cutline

// Root of every panel: a focus scope the shortcut router can identify (cutlinePanelContext), with a 1px
// accent line along the top while it has focus.
FocusScope {
    id: frame

    property string panelId: ""
    property int cutlinePanelContext: 0
    default property alias content: inner.data

    objectName: "panel." + panelId
    activeFocusOnTab: false
    clip: true

    Accessible.role: Accessible.Pane
    Accessible.name: panelId

    Rectangle { anchors.fill: parent; color: Theme.panel }

    TapHandler {
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressedChanged: if (pressed && !frame.activeFocus) frame.forceActiveFocus()
    }

    Item {
        id: inner
        anchors.fill: parent
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: Theme.focusRing
        visible: frame.activeFocus
        z: 100
    }

    Component.onCompleted: {
        const w = ApplicationWindow.window
        if (w && w.registerPanel)
            w.registerPanel(panelId, frame)
    }
    Component.onDestruction: {
        const w = ApplicationWindow.window
        if (w && w.unregisterPanel)
            w.unregisterPanel(panelId, frame)
    }
}
