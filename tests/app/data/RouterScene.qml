import QtQuick
import QtQuick.Controls
import Cutline

Window {
    id: root
    width: 640
    height: 400
    visible: true

    property int itemKeyCount: 0

    Row {
        FocusScope {
            objectName: "timelinePanel"
            property int cutlinePanelContext: 1 // Timeline
            width: 200
            height: 300

            Item {
                objectName: "timelineItem"
                anchors.fill: parent
                focus: true
                Keys.onPressed: (event) => root.itemKeyCount++
            }
        }

        FocusScope {
            objectName: "inspectorPanel"
            property int cutlinePanelContext: 8 // Inspector
            width: 300
            height: 300

            Column {
                Item {
                    objectName: "inspectorItem"
                    width: 100
                    height: 20
                    focus: true
                }
                TextField { objectName: "fieldA"; width: 200 }
                TextField { objectName: "fieldB"; width: 200 }
            }
        }

        ShortcutRecorder {
            objectName: "recorder"
            width: 60
            height: 20
        }
    }

    Popup {
        objectName: "testPopup"
        modal: true
        focus: true
        width: 120
        height: 80
        contentItem: Item {}
    }
}
