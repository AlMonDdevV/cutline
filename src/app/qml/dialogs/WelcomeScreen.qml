import QtQuick
import QtQuick.Controls
import Cutline

// First thing people see: quick starts for the two things they make most, plus recent projects.
CDialog {
    id: dialog
    objectName: "dialog.welcome"
    width: Math.min(640, (parent ? parent.width : 640) - Theme.space6 * 2)
    signal openRequested()
    signal recentRequested(string path)

    function start(preset) {
        Project.newProject(String(preset))
        close()
    }

    Text {
        text: qsTr("Start something new")
        font.family: Theme.uiFont
        font.pixelSize: Theme.fontXl
        font.weight: Font.DemiBold
        color: Theme.text
    }

    Row {
        spacing: Theme.space3
        Repeater {
            model: [ { preset: "youtube", title: qsTr("YouTube video"), subtitle: "16:9 · 1920 × 1080", w: 64, h: 36 },
                     { preset: "vertical", title: qsTr("Short, TikTok or Reel"), subtitle: "9:16 · 1080 × 1920", w: 22, h: 40 } ]
            Button {
                id: card
                required property var modelData
                objectName: "welcome." + modelData.preset
                width: (dialog.width - Theme.space4 * 2 - Theme.space3) / 2
                height: Math.round(132 * Theme.scale)
                onClicked: dialog.start(modelData.preset)
                Accessible.name: modelData.title
                contentItem: Column {
                    spacing: Theme.space2
                    Rectangle {
                        width: Math.round(card.modelData.w * Theme.scale)
                        height: Math.round(card.modelData.h * Theme.scale)
                        radius: 2
                        color: "transparent"
                        border.width: 1.5
                        border.color: card.hovered ? Theme.accent : Theme.textMuted
                    }
                    Text {
                        text: card.modelData.title
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fontLg
                        font.weight: Font.Medium
                        color: Theme.text
                    }
                    Text {
                        text: card.modelData.subtitle
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fontSm
                        color: Theme.textMuted
                    }
                }
                background: Rectangle {
                    radius: Theme.radius * 2
                    color: card.down ? Theme.pressed : (card.hovered ? Theme.hover : Theme.field)
                    border.width: 1
                    border.color: card.visualFocus ? Theme.focusRing : Theme.divider
                }
            }
        }
    }

    CButton {
        text: qsTr("Open Project…")
        iconName: "chevron-right"
        onClicked: { dialog.close(); dialog.openRequested() }
    }

    Column {
        visible: Recent.paths.length > 0
        spacing: Theme.space1
        SectionHeader { leftPadding: 0; text: qsTr("Recent") }
        Repeater {
            model: Recent.paths.slice(0, 5)
            ItemDelegate {
                required property string modelData
                width: dialog.width - Theme.space4 * 2
                height: Theme.rowHeight + Theme.space1
                onClicked: { dialog.close(); dialog.recentRequested(modelData) }
                contentItem: Text {
                    text: Recent.displayName(parent.modelData)
                    font.family: Theme.uiFont
                    font.pixelSize: Theme.fontMd
                    color: Theme.text
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle { radius: Theme.radius; color: parent.hovered ? Theme.hover : "transparent" }
            }
        }
    }

    CheckBox {
        id: showAtStartup
        text: qsTr("Show this at startup")
        checked: Settings.showWelcome
        onToggled: Settings.showWelcome = checked
        contentItem: Text {
            leftPadding: showAtStartup.indicator.width + Theme.space2
            text: showAtStartup.text
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontSm
            color: Theme.textMuted
            verticalAlignment: Text.AlignVCenter
        }
    }
}
