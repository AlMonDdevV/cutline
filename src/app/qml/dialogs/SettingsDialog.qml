import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutline

CDialog {
    id: dialog
    objectName: "dialog.settings"
    title: qsTr("Settings")
    width: Math.min(820, (parent ? parent.width : 820) - Theme.space6 * 2)

    property string page: "general"
    signal importThemeRequested()
    signal exportThemeRequested()

    function openAt(which) {
        page = which
        open()
    }

    Row {
        width: dialog.width - Theme.space4 * 2
        spacing: Theme.space4

        Column {
            id: nav
            width: Math.round(160 * Theme.scale)
            spacing: Theme.space1
            Repeater {
                model: [ { id: "general", title: qsTr("General") }, { id: "keyboard", title: qsTr("Keyboard Shortcuts") } ]
                ItemDelegate {
                    required property var modelData
                    objectName: "settings.nav." + modelData.id
                    width: nav.width
                    height: Theme.rowHeight + Theme.space2
                    onClicked: dialog.page = modelData.id
                    contentItem: Text {
                        text: parent.modelData.title
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fontMd
                        font.weight: dialog.page === parent.modelData.id ? Font.Medium : Font.Normal
                        color: dialog.page === parent.modelData.id ? Theme.text : Theme.textMuted
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: Theme.radius
                        color: dialog.page === parent.modelData.id ? Theme.pressed : (parent.hovered ? Theme.hover : "transparent")
                    }
                }
            }
        }

        Loader {
            width: parent.width - nav.width - Theme.space4
            height: Math.round(440 * Theme.scale)
            sourceComponent: dialog.page === "keyboard" ? keyboardPage : generalPage
        }
    }

    Component {
        id: generalPage
        GeneralSettingsPage {
            onImportThemeRequested: dialog.importThemeRequested()
            onExportThemeRequested: dialog.exportThemeRequested()
        }
    }
    Component {
        id: keyboardPage
        KeyboardSettingsPage {}
    }

    footer: [ CButton { text: qsTr("Done"); emphasized: true; onClicked: dialog.close() } ]
}
