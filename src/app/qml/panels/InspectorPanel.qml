import QtQuick
import QtQuick.Controls
import Cutline

PanelFrame {
    id: panel
    panelId: "inspector"
    cutlinePanelContext: 8 // Inspector

    readonly property string trackId: Project.selectedTrackId
    readonly property var seq: Project.sequence

    Flickable {
        anchors.fill: parent
        contentHeight: column.implicitHeight + Theme.space4
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}

        Column {
            id: column
            width: parent.width

            // ---- Track selected ----
            Column {
                id: trackSection
                width: parent.width
                visible: panel.trackId.length > 0
                readonly property int rev: Project.revision
                function field(name) {
                    return trackSection.rev >= 0 && panel.trackId.length > 0 ? Project.trackField(panel.trackId, name) : undefined
                }
                readonly property string kind: field("kind") || ""

                SectionHeader { text: qsTr("Track %1").arg(trackSection.field("code") || "") }
                InspectorRow {
                    label: qsTr("Name")
                    CTextField {
                        objectName: "inspector.trackName"
                        anchors.fill: parent
                        value: trackSection.field("name") || ""
                        onCommit: (v) => Project.setTrackName(panel.trackId, v)
                    }
                }
                InspectorRow {
                    label: qsTr("Kind")
                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: trackSection.kind === "video" ? qsTr("Video") : qsTr("Audio")
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fontMd
                        color: Theme.text
                    }
                }
                InspectorRow {
                    label: qsTr("Locked")
                    CheckBox {
                        anchors.verticalCenter: parent.verticalCenter
                        checked: trackSection.field("locked") === true
                        onToggled: Project.setTrackLocked(panel.trackId, checked)
                    }
                }
                InspectorRow {
                    label: trackSection.kind === "video" ? qsTr("Visible") : qsTr("Audible")
                    CheckBox {
                        anchors.verticalCenter: parent.verticalCenter
                        checked: trackSection.field("enabled") === true
                        onToggled: Project.setTrackEnabled(panel.trackId, checked)
                    }
                }
                InspectorRow {
                    visible: trackSection.kind === "audio"
                    label: qsTr("Solo")
                    CheckBox {
                        anchors.verticalCenter: parent.verticalCenter
                        checked: trackSection.field("solo") === true
                        onToggled: Project.setTrackSolo(panel.trackId, checked)
                    }
                }
            }

            // ---- Nothing selected: project and sequence ----
            Column {
                width: parent.width
                visible: panel.trackId.length === 0

                SectionHeader { text: qsTr("Project") }
                InspectorRow {
                    label: qsTr("Name")
                    CTextField {
                        objectName: "inspector.projectName"
                        anchors.fill: parent
                        value: Project.projectName
                        onCommit: (v) => Project.setProjectName(v)
                    }
                }

                SectionHeader { text: qsTr("Sequence") }
                InspectorRow {
                    label: qsTr("Name")
                    CTextField {
                        objectName: "inspector.sequenceName"
                        anchors.fill: parent
                        value: panel.seq.name
                        onCommit: (v) => panel.seq.name = v
                    }
                }
                InspectorRow {
                    id: resolutionRow
                    // "Custom" only reveals the size fields; the size changes when you type one.
                    property bool customMode: false
                    readonly property int customIndex: panel.seq.resolutionOptions.length - 1
                    label: qsTr("Resolution")
                    CComboBox {
                        objectName: "inspector.resolution"
                        anchors.fill: parent
                        model: panel.seq.resolutionOptions
                        currentIndex: resolutionRow.customMode ? resolutionRow.customIndex : panel.seq.resolutionIndex
                        onActivated: (i) => {
                            resolutionRow.customMode = i === resolutionRow.customIndex
                            if (!resolutionRow.customMode)
                                panel.seq.resolutionIndex = i
                        }
                    }
                }
                InspectorRow {
                    objectName: "inspector.customSize"
                    visible: resolutionRow.customMode || panel.seq.resolutionIndex === resolutionRow.customIndex
                    label: qsTr("Width × height")
                    Row {
                        anchors.fill: parent
                        spacing: Theme.space2
                        CTextField {
                            width: (parent.width - Theme.space2) / 2
                            value: String(panel.seq.width)
                            validator: IntValidator { bottom: 16; top: 8192 }
                            onCommit: (v) => panel.seq.width = parseInt(v)
                        }
                        CTextField {
                            width: (parent.width - Theme.space2) / 2
                            value: String(panel.seq.height)
                            validator: IntValidator { bottom: 16; top: 8192 }
                            onCommit: (v) => panel.seq.height = parseInt(v)
                        }
                    }
                }
                InspectorRow {
                    label: qsTr("Frame rate")
                    CComboBox {
                        anchors.fill: parent
                        model: panel.seq.frameRateOptions
                        currentIndex: panel.seq.frameRateIndex
                        displayText: currentIndex >= 0 ? currentText + " fps" : qsTr("Custom")
                        onActivated: (i) => panel.seq.frameRateIndex = i
                    }
                }
                InspectorRow {
                    label: qsTr("Sample rate")
                    CComboBox {
                        anchors.fill: parent
                        model: panel.seq.sampleRateOptions
                        currentIndex: panel.seq.sampleRateIndex
                        onActivated: (i) => panel.seq.sampleRateIndex = i
                    }
                }
                InspectorRow {
                    label: qsTr("Timecode start")
                    CComboBox {
                        anchors.fill: parent
                        model: ["00:00:00:00", "01:00:00:00"]
                        currentIndex: panel.seq.timecodeStartHour
                        onActivated: (i) => panel.seq.timecodeStartHour = i
                    }
                }
            }
        }
    }
}
