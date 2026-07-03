import QtQuick

// Minimal vector silhouette of an AirPod (stem + speaker housing), not a
// copy of Apple's product renders - tilted left/right to fan out like the
// two-earbud layout in the reference mockup.
Item {
    id: root
    property bool mirrored: false
    implicitWidth: 40
    implicitHeight: 74
    rotation: mirrored ? 16 : -16

    Rectangle { // stem
        width: 11
        height: 40
        radius: 5.5
        color: "#ffffff"
        border.color: "#dcdce0"
        border.width: 1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

        Rectangle { // mic hole
            width: 4
            height: 4
            radius: 2
            color: "#c7c7cc"
            anchors.horizontalCenter: parent.horizontalCenter
            y: 8
        }
    }

    Rectangle { // speaker housing
        width: 28
        height: 34
        radius: 14
        color: "#ffffff"
        border.color: "#dcdce0"
        border.width: 1
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top

        Rectangle { // speaker mesh dot
            width: 6
            height: 6
            radius: 3
            color: "#c7c7cc"
            x: 9
            y: 12
        }
    }
}
