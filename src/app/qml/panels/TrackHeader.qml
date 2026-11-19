import QtQuick
import QtQuick.Controls
import Cutline

// One track's header: colour bar, code, name (double-click to rename), lock, visibility/mute and solo.
Rectangle {
    id: header

    required property string trackId
    required property string code
    required property string name
    required property string kind
    required property bool locked
    required property bool enabled
    required property bool solo
    required property bool selected
    signal contextMenuRequested(real x, real y)

    readonly property bool isVideo: kind === "video"
    property bool editing: false

    objectName: "track." + code
    implicitHeight: Math.round(Theme.rowHeight * 2)
    color: selected ? Theme.selection : Theme.panelRaised

    Accessible.role: Accessible.ListItem
    Accessible.name: code + " " + name

    function startEditing() {
        editing = true
        Qt.callLater(function() { if (editorLoader.item) { editorLoader.item.forceActiveFocus(); editorLoader.item.selectAll() } })
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: Project.selectedTrackId = header.trackId
        onDoubleTapped: header.startEditing()
    }
    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: (point) => {
            Project.selectedTrackId = header.trackId
            header.contextMenuRequested(point.position.x, point.position.y)
        }
    }

    Rectangle {
        width: 3
        height: parent.height
        color: header.isVideo ? Theme.clipVideo : Theme.clipAudio
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: Theme.space3
        anchors.right: controls.left
        anchors.rightMargin: Theme.space2
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space2

        Text {
            width: Theme.space6
            text: header.code
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontSm
            font.weight: Font.DemiBold
            color: Theme.text
            anchors.verticalCenter: parent.verticalCenter
        }

        Item {
            id: nameSlot
            objectName: "track." + header.code + ".name"
            width: parent.width - Theme.space6 - Theme.space2
            height: Theme.rowHeight
            anchors.verticalCenter: parent.verticalCenter

            function startEditing() { header.startEditing() }

            Text {
                anchors.fill: parent
                visible: !header.editing
                text: header.name
                font.family: Theme.uiFont
                font.pixelSize: Theme.fontMd
                color: Theme.textMuted
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            Loader {
                id: editorLoader
                anchors.fill: parent
                active: header.editing
                // Commits on Enter or when you click away; Escape reverts (like the Inspector's fields).
                sourceComponent: CTextField {
                    objectName: "track." + header.code + ".editor"
                    value: header.name
                    onCommit: (v) => Project.setTrackName(header.trackId, v)
                    onEditingDone: Qt.callLater(function() { header.editing = false })
                }
            }
        }
    }

    Row {
        id: controls
        anchors.right: parent.right
        anchors.rightMargin: Theme.space2
        anchors.verticalCenter: parent.verticalCenter
        spacing: Theme.space1

        CToolButton {
            iconName: header.locked ? "lock" : "unlock"
            iconColor: header.locked ? Theme.text : Theme.textMuted
            tip: header.locked ? qsTr("Unlock track") : qsTr("Lock track")
            onClicked: Project.setTrackLocked(header.trackId, !header.locked)
        }
        CToolButton {
            iconName: header.isVideo ? (header.enabled ? "eye" : "eye-off") : (header.enabled ? "speaker" : "speaker-off")
            iconColor: header.enabled ? Theme.textMuted : Theme.text
            tip: header.isVideo ? (header.enabled ? qsTr("Hide track") : qsTr("Show track"))
                                : (header.enabled ? qsTr("Mute track") : qsTr("Unmute track"))
            onClicked: Project.setTrackEnabled(header.trackId, !header.enabled)
        }
        CToolButton {
            visible: !header.isVideo
            text: "S"
            checkable: false
            checked: header.solo
            tip: header.solo ? qsTr("Unsolo track") : qsTr("Solo track")
            onClicked: Project.setTrackSolo(header.trackId, !header.solo)
        }
        Item { visible: header.isVideo; width: Theme.rowHeight; height: 1 }
    }

    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }
}
