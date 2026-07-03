import QtQuick

// Minimal vector silhouette of the charging case (rounded shell + lid seam).
Item {
    implicitWidth: 58
    implicitHeight: 64

    Rectangle {
        anchors.fill: parent
        radius: 18
        color: "#ffffff"
        border.color: "#dcdce0"
        border.width: 1

        Rectangle { // lid seam
            width: parent.width * 0.46
            height: 3
            radius: 1.5
            color: "#dcdce0"
            anchors.horizontalCenter: parent.horizontalCenter
            y: parent.height * 0.3
        }
    }
}
