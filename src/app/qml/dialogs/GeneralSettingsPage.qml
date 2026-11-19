import QtQuick
import QtQuick.Controls
import Cutline

Column {
    id: page
    spacing: Theme.space2
    signal importThemeRequested()
    signal exportThemeRequested()

    readonly property var modes: ["dark", "light", "system"]

    SectionHeader { leftPadding: 0; text: qsTr("Appearance") }

    InspectorRow {
        label: qsTr("Theme")
        CComboBox {
            objectName: "settings.theme"
            anchors.fill: parent
            model: Theme.themeNames
            currentIndex: {
                const i = page.modes.indexOf(Theme.mode)
                return i >= 0 ? i : Theme.themeNames.indexOf(Theme.mode.replace("custom:", ""))
            }
            onActivated: (i) => Theme.setMode(i < 3 ? page.modes[i] : "custom:" + Theme.themeNames[i])
        }
    }

    Row {
        x: Theme.space3
        spacing: Theme.space2
        CButton { text: qsTr("Import Theme…"); onClicked: page.importThemeRequested() }
        CButton { text: qsTr("Export Current Theme…"); onClicked: page.exportThemeRequested() }
        CButton {
            text: qsTr("Open Themes Folder")
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Edit a theme file here and CUTLINE updates as you save.")
            onClicked: Theme.openThemesFolder()
        }
    }
    Text {
        x: Theme.space3
        text: qsTr("Community theme library coming later.")
        font.family: Theme.uiFont
        font.pixelSize: Theme.fontSm
        color: Theme.textMuted
    }

    InspectorRow {
        label: qsTr("Interface scale")
        Row {
            anchors.fill: parent
            spacing: Theme.space2
            Slider {
                id: scaleSlider
                objectName: "settings.scale"
                width: parent.width - scaleText.width - Theme.space2
                anchors.verticalCenter: parent.verticalCenter
                from: 0.9
                to: 1.5
                stepSize: 0.1
                snapMode: Slider.SnapAlways
                value: Theme.scale
                onMoved: Theme.scale = Math.round(value * 10) / 10
            }
            Text {
                id: scaleText
                anchors.verticalCenter: parent.verticalCenter
                text: Math.round(Theme.scale * 100) + "%"
                font.family: Theme.uiFont
                font.pixelSize: Theme.fontMd
                color: Theme.text
            }
        }
    }

    SectionHeader { leftPadding: 0; text: qsTr("Startup") }
    CheckBox {
        id: welcome
        x: Theme.space2
        text: qsTr("Show the welcome screen at startup")
        checked: Settings.showWelcome
        onToggled: Settings.showWelcome = checked
        contentItem: Text {
            leftPadding: welcome.indicator.width + Theme.space2
            text: welcome.text
            font.family: Theme.uiFont
            font.pixelSize: Theme.fontMd
            color: Theme.text
            verticalAlignment: Text.AlignVCenter
        }
    }
}
