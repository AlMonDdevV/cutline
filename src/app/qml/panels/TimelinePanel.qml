import QtQuick
import QtQuick.Controls
import Cutline

PanelFrame {
    id: panel
    panelId: "timeline"
    cutlinePanelContext: 1 // Timeline

    readonly property var seq: Project.sequence
    readonly property int headerWidth: Math.round(220 * Theme.scale)
    readonly property real pixelsPerSecond: 22 * Theme.scale

    // Top row: big timecode over the headers, ruler over the lanes.
    Rectangle {
        id: top
        anchors.left: parent.left
        anchors.right: parent.right
        height: Theme.headerHeight + Theme.space2
        color: Theme.panelRaised

        Item {
            width: panel.headerWidth
            height: parent.height
            TimecodeLabel {
                anchors.left: parent.left
                anchors.leftMargin: Theme.space3
                anchors.verticalCenter: parent.verticalCenter
                pixelSize: Theme.fontXl
                timecode: Timecode.format(0, panel.seq.frameRateNum, panel.seq.frameRateDen, false, panel.seq.timecodeStartHour)
            }
            Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Theme.divider }
        }

        Item {
            id: ruler
            x: panel.headerWidth
            width: parent.width - panel.headerWidth
            height: parent.height
            clip: true

            Repeater {
                model: Math.max(0, Math.ceil(ruler.width / panel.pixelsPerSecond) + 1)
                Rectangle {
                    required property int index
                    x: index * panel.pixelsPerSecond
                    anchors.bottom: parent.bottom
                    width: 1
                    height: index % 5 === 0 ? Theme.space3 : Theme.space1 + 1
                    color: Theme.textDisabled
                }
            }
            Repeater {
                model: Math.max(0, Math.ceil(ruler.width / (panel.pixelsPerSecond * 5)) + 1)
                Text {
                    required property int index
                    x: index * panel.pixelsPerSecond * 5 + Theme.space1
                    y: Theme.space1
                    text: Timecode.format(index * 5 * Math.round(panel.seq.fps > 0 ? panel.seq.fps : 30),
                                          panel.seq.frameRateNum, panel.seq.frameRateDen, false, panel.seq.timecodeStartHour)
                    font.family: Theme.monoFont
                    font.pixelSize: Theme.fontXs - 1
                    color: Theme.textMuted
                }
            }
            // Playhead head
            Canvas {
                id: playheadHead
                width: 12
                height: 8
                x: -6
                anchors.bottom: parent.bottom
                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    ctx.fillStyle = Theme.accent
                    ctx.beginPath(); ctx.moveTo(0, 0); ctx.lineTo(12, 0); ctx.lineTo(6, 8); ctx.closePath(); ctx.fill()
                }
                Connections { target: Theme; function onPaletteChanged() { playheadHead.requestPaint() } }
            }
        }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }
    }

    // Header column background, so the space below the last track still reads as the header column.
    Rectangle {
        anchors.top: top.bottom
        anchors.bottom: parent.bottom
        width: panel.headerWidth
        color: Theme.panelRaised
    }

    ListView {
        id: tracks
        anchors.top: top.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        clip: true
        model: Project.tracks
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}

        delegate: Item {
            id: row
            required property int index
            required property string id
            required property string code
            required property string name
            required property string kind
            required property bool locked
            required property bool enabled
            required property bool solo
            required property bool selected
            width: tracks.width
            height: trackHeader.implicitHeight

            TrackHeader {
                id: trackHeader
                width: panel.headerWidth
                height: parent.height
                trackId: row.id
                code: row.code
                name: row.name
                kind: row.kind
                locked: row.locked
                enabled: row.enabled
                solo: row.solo
                selected: row.selected
                onContextMenuRequested: (x, y) => trackMenu.popup(trackHeader, x, y)
            }
            Rectangle {
                x: panel.headerWidth
                width: parent.width - panel.headerWidth
                height: parent.height
                color: row.index % 2 === 0 ? Theme.panel : Qt.darker(Theme.panel, Theme.dark ? 1.08 : 1.02)
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }
            }
            // A heavier line between the video and audio sections.
            Rectangle {
                visible: row.kind === "video" && row.code === "V1"
                anchors.bottom: parent.bottom
                width: parent.width
                height: 2
                color: Theme.textDisabled
            }
        }
    }

    // Playhead line through all tracks (at 0 until media arrives).
    Rectangle {
        x: panel.headerWidth
        anchors.top: top.bottom
        anchors.bottom: parent.bottom
        width: 1
        color: Theme.accent
    }

    CMenu {
        id: trackMenu
        CMenuItem { actionId: "sequence.addVideoTrack" }
        CMenuItem { actionId: "sequence.addAudioTrack" }
        MenuSeparator {}
        CMenuItem { actionId: "sequence.removeTrack" }
    }
}
