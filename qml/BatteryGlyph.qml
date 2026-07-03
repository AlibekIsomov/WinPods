import QtQuick
import QtQuick.Layouts

// iOS-style pill battery icon (body + nub + fill) with a lightning-bolt
// overlay while charging, and a percentage label underneath.
ColumnLayout {
    id: root
    property int level: -1 // 0-100, -1 = unknown/disconnected
    property bool charging: false
    spacing: 4

    readonly property color fillColor: level < 0 ? "#c7c7cc" : level <= 20 ? "#ff3b30" : "#34c759"

    Item {
        Layout.alignment: Qt.AlignHCenter
        implicitWidth: 30
        implicitHeight: 15

        Rectangle { // body
            id: body
            width: 26
            height: 14
            radius: 3
            color: "transparent"
            border.color: "#8e8e93"
            border.width: 1.4
            anchors.left: parent.left

            Rectangle { // charge fill
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 2
                height: parent.height - 4
                width: Math.max(0, (parent.width - 4) * Math.max(0, root.level) / 100)
                radius: 1.5
                color: root.fillColor

                Behavior on width {
                    NumberAnimation { duration: 350; easing.type: Easing.OutCubic }
                }
            }
        }

        Rectangle { // nub
            width: 3
            height: 6
            radius: 1
            color: "#8e8e93"
            anchors.left: body.right
            anchors.verticalCenter: body.verticalCenter
        }

        Text {
            visible: root.charging
            text: "⚡"
            font.pixelSize: 11
            color: "#ffffff"
            anchors.centerIn: body
        }
    }

    Text {
        Layout.alignment: Qt.AlignHCenter
        text: root.level < 0 ? "--" : root.level + "%"
        font.pixelSize: 13
        color: "#3a3a3c"
    }
}
