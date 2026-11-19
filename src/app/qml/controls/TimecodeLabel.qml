import QtQuick
import Cutline

// Fixed-width timecode with the leading zeros dimmed, so the digits that change stand out.
Row {
    id: label
    property string timecode: "00:00:00:00"
    property int pixelSize: Theme.fontTimecode
    readonly property int split: Timecode.significantIndex(timecode)

    Accessible.role: Accessible.StaticText
    Accessible.name: timecode

    Text {
        text: label.timecode.substring(0, label.split)
        font.family: Theme.monoFont
        font.pixelSize: label.pixelSize
        font.weight: Font.Medium
        color: Theme.timecodeDim
    }
    Text {
        text: label.timecode.substring(label.split)
        font.family: Theme.monoFont
        font.pixelSize: label.pixelSize
        font.weight: Font.Medium
        color: Theme.text
    }
}
