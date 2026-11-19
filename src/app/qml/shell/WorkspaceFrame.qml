import QtQuick
import QtQuick.Controls
import Cutline

// The five-area frame: [Left | Viewer | Right] over [Timeline | Side], all resizable.
SplitView {
    id: vertical
    orientation: Qt.Vertical

    handle: Rectangle {
        implicitWidth: 3
        implicitHeight: 3
        color: SplitHandle.pressed ? Theme.accent : (SplitHandle.hovered ? Theme.textDisabled : Theme.window)
    }

    function fractionsOf(row, groups, key) {
        const old = Workspaces.sizes(key)
        let visibleTotal = 0, visibleOld = 0
        for (let i = 0; i < groups.length; ++i)
            if (groups[i].visible) { visibleTotal += (row.orientation === Qt.Horizontal ? groups[i].width : groups[i].height); visibleOld += old[i] }
        const out = []
        for (let i = 0; i < groups.length; ++i) {
            const size = row.orientation === Qt.Horizontal ? groups[i].width : groups[i].height
            out.push(groups[i].visible && visibleTotal > 0 ? size / visibleTotal * visibleOld : old[i])
        }
        return out
    }

    function applySizes() {
        const top = Workspaces.sizes("top"), bottom = Workspaces.sizes("bottom"), vert = Workspaces.sizes("vertical")
        const tg = [left, viewer, right], bg = [timeline, side]
        let topVisible = 0, bottomVisible = 0
        for (let i = 0; i < 3; ++i) if (tg[i].visible) topVisible += top[i]
        for (let i = 0; i < 2; ++i) if (bg[i].visible) bottomVisible += bottom[i]
        for (let i = 0; i < 3; ++i) tg[i].SplitView.preferredWidth = topRow.width * top[i] / Math.max(topVisible, 0.0001)
        for (let i = 0; i < 2; ++i) bg[i].SplitView.preferredWidth = bottomRow.width * bottom[i] / Math.max(bottomVisible, 0.0001)
        if (topRow.visible && bottomRow.visible)
            topRow.SplitView.preferredHeight = vertical.height * vert[0]
    }

    Component.onCompleted: Qt.callLater(applySizes)
    onWidthChanged: Qt.callLater(applySizes)
    onHeightChanged: Qt.callLater(applySizes)
    Connections {
        target: Workspaces
        function onStructureRevisionChanged() { Qt.callLater(vertical.applySizes) }
        function onCurrentChanged() { Qt.callLater(vertical.applySizes) }
    }
    onResizingChanged: if (!resizing && topRow.visible && bottomRow.visible)
        Workspaces.setSizes("vertical", [topRow.height / height, bottomRow.height / height])

    SplitView {
        id: topRow
        orientation: Qt.Horizontal
        visible: left.visible || viewer.visible || right.visible
        SplitView.minimumHeight: Math.max(left.minimumSize.height, viewer.minimumSize.height, right.minimumSize.height)
        SplitView.fillHeight: !bottomRow.visible
        handle: vertical.handle
        onResizingChanged: if (!resizing) Workspaces.setSizes("top", vertical.fractionsOf(topRow, [left, viewer, right], "top"))

        PanelGroup { id: left; area: 0; visible: panelIds.length > 0; SplitView.minimumWidth: minimumSize.width }
        PanelGroup { id: viewer; area: 1; visible: panelIds.length > 0; SplitView.minimumWidth: minimumSize.width; SplitView.fillWidth: true }
        PanelGroup { id: right; area: 2; visible: panelIds.length > 0; SplitView.minimumWidth: minimumSize.width; SplitView.fillWidth: !viewer.visible }
    }

    SplitView {
        id: bottomRow
        orientation: Qt.Horizontal
        visible: timeline.visible || side.visible
        SplitView.minimumHeight: Math.max(timeline.minimumSize.height, side.minimumSize.height)
        SplitView.fillHeight: true
        handle: vertical.handle
        onResizingChanged: if (!resizing) Workspaces.setSizes("bottom", vertical.fractionsOf(bottomRow, [timeline, side], "bottom"))

        PanelGroup { id: timeline; area: 3; visible: panelIds.length > 0; SplitView.minimumWidth: minimumSize.width; SplitView.fillWidth: true }
        PanelGroup { id: side; area: 4; visible: panelIds.length > 0; SplitView.minimumWidth: minimumSize.width }
    }
}
