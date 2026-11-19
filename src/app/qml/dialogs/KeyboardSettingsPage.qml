import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import Cutline

Item {
    id: page

    ShortcutsModel { id: shortcuts }
    property string recordingId: ""
    property string notice: ""

    function apply(actionId, portable) {
        recordingId = ""
        const clashes = shortcuts.conflictsFor(actionId, portable)
        if (clashes.length === 0) {
            if (!shortcuts.assign(actionId, portable, false))
                notice = shortcuts.lastError()
            return
        }
        const c = clashes[0]
        conflict.ask(qsTr("%1 is already used by “%2” (%3).").arg(shortcuts.nativeText(portable)).arg(c.title).arg(c.contextsText),
                     function() { shortcuts.assign(actionId, portable, true) })
    }

    Column {
        id: top
        width: parent.width
        spacing: Theme.space2

        CTextField {
            id: search
            objectName: "keyboard.search"
            width: parent.width
            placeholderText: qsTr("Search actions or keys")
            onTextChanged: shortcuts.filter = text
        }
        Row {
            spacing: Theme.space2
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: qsTr("Profile: %1").arg(shortcuts.profileName)
                font.family: Theme.uiFont
                font.pixelSize: Theme.fontMd
                color: Theme.textMuted
            }
            CButton { text: qsTr("Import…"); onClicked: importDialog.open() }
            CButton { text: qsTr("Export…"); onClicked: exportDialog.open() }
            CButton { objectName: "keyboard.restoreAll"; text: qsTr("Restore All Defaults"); onClicked: shortcuts.resetAll() }
        }
        Text {
            visible: page.notice.length > 0
            text: page.notice
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontSm
            color: Theme.danger
        }
    }

    Row {
        anchors.top: top.bottom
        anchors.topMargin: Theme.space2
        anchors.bottom: parent.bottom
        width: parent.width
        spacing: Theme.space3

        ListView {
            id: categories
            width: Math.round(130 * Theme.scale)
            height: parent.height
            model: [qsTr("All")].concat(shortcuts.categories)
            delegate: ItemDelegate {
                required property string modelData
                required property int index
                width: categories.width
                height: Theme.rowHeight + Theme.space1
                onClicked: { categories.currentIndex = index; shortcuts.category = index === 0 ? "" : modelData }
                contentItem: Text {
                    text: parent.modelData
                    font.family: Theme.uiFont
                    font.pixelSize: Theme.fontMd
                    color: categories.currentIndex === parent.index ? Theme.text : Theme.textMuted
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    radius: Theme.radius
                    color: categories.currentIndex === parent.index ? Theme.pressed : (parent.hovered ? Theme.hover : "transparent")
                }
            }
        }

        ListView {
            id: list
            objectName: "keyboard.list"
            width: parent.width - categories.width - Theme.space3
            height: parent.height
            clip: true
            model: shortcuts
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar {}

            delegate: Rectangle {
                id: row
                required property string actionId
                required property string title
                required property string description
                required property string shortcutsText
                required property string contextsText
                required property bool overridden
                required property bool fixed
                readonly property bool recording: page.recordingId === actionId

                objectName: "keyboard.row." + actionId
                width: list.width
                height: Theme.rowHeight + Theme.space3
                color: recording ? Theme.selection : "transparent"
                ToolTip.visible: hover.hovered && description.length > 0
                ToolTip.text: description
                HoverHandler { id: hover }

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.space2
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width * 0.45
                    Text {
                        width: parent.width
                        text: row.title
                        elide: Text.ElideRight
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fontMd
                        color: Theme.text
                    }
                    Text {
                        text: row.contextsText
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fontXs
                        color: Theme.textMuted
                    }
                }

                Row {
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.space2
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: Theme.space1

                    Rectangle {
                        width: Math.max(Math.round(150 * Theme.scale), keys.implicitWidth + Theme.space3 * 2)
                        height: Theme.rowHeight
                        radius: Theme.radius
                        color: Theme.field
                        border.width: 1
                        border.color: row.recording ? Theme.focusRing : Theme.divider

                        Text {
                            id: keys
                            anchors.centerIn: parent
                            text: row.recording ? qsTr("Press a shortcut…") : (row.shortcutsText.length > 0 ? row.shortcutsText : qsTr("None"))
                            font.family: Theme.uiFont
                            font.pixelSize: Theme.fontSm
                            color: row.recording || row.shortcutsText.length > 0 ? Theme.text : Theme.textDisabled
                        }
                        ShortcutRecorder {
                            id: recorder
                            objectName: "keyboard.recorder." + row.actionId
                            anchors.fill: parent
                            recording: row.recording
                            onRecordingChanged: if (recording) forceActiveFocus()
                            onCaptured: (portable) => page.apply(row.actionId, portable)
                            onCancelled: page.recordingId = ""
                            onActiveFocusChanged: if (!activeFocus && row.recording) page.recordingId = ""
                        }
                    }
                    CToolButton {
                        objectName: "keyboard.record." + row.actionId
                        visible: !row.fixed
                        iconName: "record"
                        tip: qsTr("Record a new shortcut")
                        onClicked: { page.notice = ""; page.recordingId = row.actionId }
                    }
                    CToolButton {
                        visible: !row.fixed && row.overridden
                        iconName: "reset"
                        tip: qsTr("Restore the default")
                        onClicked: shortcuts.resetAction(row.actionId)
                    }
                    CToolButton {
                        visible: row.fixed
                        iconName: "lock"
                        tip: qsTr("This shortcut can't be changed.")
                    }
                }
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }
            }
        }
    }

    ShortcutConflictDialog { id: conflict }

    FileDialog {
        id: importDialog
        title: qsTr("Import Shortcuts")
        fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("CUTLINE shortcuts (*.json)")]
        onAccepted: if (!shortcuts.importProfile(selectedFile)) page.notice = shortcuts.lastError()
    }
    FileDialog {
        id: exportDialog
        title: qsTr("Export Shortcuts")
        fileMode: FileDialog.SaveFile
        defaultSuffix: "json"
        nameFilters: [qsTr("CUTLINE shortcuts (*.json)")]
        onAccepted: if (!shortcuts.exportProfile(selectedFile)) page.notice = shortcuts.lastError()
    }
}
