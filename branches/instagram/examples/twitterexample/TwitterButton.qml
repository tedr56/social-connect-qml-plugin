import QtQuick 1.1

Rectangle {
    id: button

    property alias text: txt.text
    signal clicked()

    width: 90
    height: 64
    color: ma.pressed ? "blue" : "red"

    Text {
        id: txt
        anchors.centerIn: parent
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        onClicked: button.clicked()
    }
}
