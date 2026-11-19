import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Cutline

// One area of the workspace frame: a tab bar (or a title strip for a single panel) over its panels.
Rectangle {
    id: group

    required property int area
    readonly property int structure: Workspaces.structureRevision
    readonly property var panelIds: structure >= 0 ? Workspaces.panelsIn(area) : []
    readonly property string currentId: Workspaces.revision >= 0 ? Workspaces.currentIn(area) : ""
    readonly property size minimumSize: {
        let w = 0, h = 0
        for (const id of panelIds) {
            const s = Workspaces.panelMinimumSize(id)
            w = Math.max(w, s.width)
            h = Math.max(h, s.height)
        }
        return Qt.size(w, h + strip.height)
    }

    objectName: "area." + area
    color: Theme.panel

    Rectangle {
        id: strip
        width: parent.width
        height: Theme.headerHeight - Theme.space1
        color: Theme.panelRaised

        Row {
            height: parent.height
            Repeater {
                model: group.panelIds
                Item {
                    id: tab
                    required property string modelData
                    readonly property bool current: modelData === group.currentId
                    width: label.implicitWidth + Theme.space4 * 2
                    height: strip.height

                    Text {
                        id: label
                        anchors.centerIn: parent
                        text: Workspaces.panelTitle(tab.modelData)
                        font.family: Theme.uiFont
                        font.pixelSize: Theme.fontMd
                        font.weight: tab.current ? Font.Medium : Font.Normal
                        color: tab.current ? Theme.text : Theme.textMuted
                    }
                    Rectangle {
                        visible: tab.current && group.panelIds.length > 1
                        anchors.bottom: parent.bottom
                        width: parent.width
                        height: 2
                        color: Theme.accent
                    }
                    TapHandler { onTapped: Workspaces.setCurrentIn(group.area, tab.modelData) }
                    Accessible.role: Accessible.PageTab
                    Accessible.name: label.text
                }
            }
        }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.divider }
    }

    StackLayout {
        anchors.top: strip.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        currentIndex: Math.max(0, group.panelIds.indexOf(group.currentId))

        Repeater {
            model: group.panelIds
            Loader {
                required property string modelData
                sourceComponent: Qt.createComponent("Cutline", Workspaces.panelComponent(modelData))
            }
        }
    }
}
