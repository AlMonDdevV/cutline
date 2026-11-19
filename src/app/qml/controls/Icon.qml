import QtQuick

// A CUTLINE stroke icon, recoloured on the fly: Icon { name: "lock"; color: Theme.text; size: 16 }
Image {
    id: icon

    property string name
    property color color: "#E6E6E6"
    property int size: 16

    width: size
    height: size
    sourceSize: Qt.size(size, size)
    fillMode: Image.PreserveAspectFit
    smooth: true
    asynchronous: false
    source: name.length > 0 ? "image://icons/" + name + "?color=" + color.toString().substring(1) : ""
    Accessible.ignored: true
}
