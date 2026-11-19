import QtQuick
import Cutline

CDialog {
    id: dialog
    objectName: "dialog.about"
    title: "CUTLINE " + AppInfo.version

    Text {
        width: parent.width - Theme.space4 * 2
        text: qsTr("Video editing without the paywall. Developer preview: the editing engine arrives in later previews.")
        wrapMode: Text.Wrap
        font.family: Theme.uiFont
        font.pixelSize: Theme.fontMd
        color: Theme.text
    }
    Column {
        spacing: Theme.space1
        Repeater {
            model: [ AppInfo.licenseName,
                     "Qt " + AppInfo.qtVersion + " (LGPLv3)",
                     "FFmpeg " + AppInfo.ffmpegVersion + " (" + AppInfo.ffmpegLicense + ")",
                     qsTr("Fonts: Inter and JetBrains Mono (SIL Open Font License)") ]
            Text {
                required property string modelData
                text: modelData
                font.family: Theme.uiFont
                font.pixelSize: Theme.fontSm
                color: Theme.textMuted
            }
        }
    }
    footer: [ CButton { text: qsTr("Close"); emphasized: true; onClicked: dialog.close() } ]
}
