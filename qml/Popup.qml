import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects

// Frameless, transparent, always-on-top window that mimics the iOS "AirPods
// connected" sheet: parked below the screen, slides up when the model says
// popupVisible, slides back down and disappears when it doesn't.
Window {
    id: popup
    width: 380
    height: 300
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    visible: y < Screen.height // stays "visible" through the slide-down animation

    // Bottom-center, like the real iOS AirPods sheet. Single ease-in-ease-out
    // curve both directions (iOS's default UIView animation curve) reads as
    // smooth/springy rather than the sharp Expo snap.
    readonly property int screenMargin: 24
    readonly property int finalY: Screen.height - height - screenMargin
    readonly property int hiddenY: Screen.height + 20

    x: Screen.width / 2 - width / 2
    y: hiddenY

    NumberAnimation {
        id: slideAnim
        target: popup
        property: "y"
        duration: 450
        easing.type: Easing.InOutCubic
    }

    function slideIn() {
        slideAnim.stop()
        slideAnim.from = popup.y
        slideAnim.to = popup.finalY
        slideAnim.start()
    }

    function slideOut() {
        slideAnim.stop()
        slideAnim.from = popup.y
        slideAnim.to = popup.hiddenY
        slideAnim.start()
    }

    Rectangle {
        id: card
        anchors.fill: parent
        anchors.margins: 12
        radius: 22
        color: "#ffffff"
        border.color: "#e5e5ea"
        border.width: 1

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: "#40000000"
            shadowBlur: 0.8
            shadowVerticalOffset: 6
        }

        Text {
            id: title
            text: airpodsModel.connected ? airpodsModel.modelName : "Searching for AirPods…"
            anchors.top: parent.top
            anchors.topMargin: 24
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 19
            color: "#3a3a3c"
        }

        // AirPods only broadcast their battery/status data in short bursts -
        // when the case opens/closes or a pod goes in/out of the ear - not
        // continuously while just sitting connected. If we haven't caught one
        // of those bursts yet, say so instead of showing an empty battery row.
        Text {
            visible: !airpodsModel.connected
            anchors.centerIn: parent
            width: parent.width - 60
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: "Open the case, or take a pod out and back in, near this PC to refresh."
            font.pixelSize: 14
            color: "#8e8e93"
        }

        RowLayout {
            id: iconsRow
            visible: airpodsModel.connected
            anchors.top: title.bottom
            anchors.topMargin: 26
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 46

            ColumnLayout {
                id: leftPodColumn
                spacing: 12
                AirPodIcon { id: leftPod; Layout.alignment: Qt.AlignHCenter; mirrored: false }
                BatteryGlyph {
                    Layout.alignment: Qt.AlignHCenter
                    level: airpodsModel.batteryLeft
                    charging: airpodsModel.chargingLeft
                }
            }
            ColumnLayout {
                id: rightPodColumn
                spacing: 12
                AirPodIcon { id: rightPod; Layout.alignment: Qt.AlignHCenter; mirrored: true }
                BatteryGlyph {
                    Layout.alignment: Qt.AlignHCenter
                    level: airpodsModel.batteryRight
                    charging: airpodsModel.chargingRight
                }
            }
            ColumnLayout {
                id: caseColumn
                spacing: 12
                CaseIcon { id: caseIcon; Layout.alignment: Qt.AlignHCenter }
                BatteryGlyph {
                    Layout.alignment: Qt.AlignHCenter
                    level: airpodsModel.batteryCase
                    charging: airpodsModel.chargingCase
                }
            }
        }

        // Staggered "assemble in" reveal: each icon scales/fades up in turn
        // (left pod, then right pod, then case) whenever the popup opens -
        // a 2D stand-in for the case's real 3D spin-in animation.
        function playRevealAnimation() {
            revealLeft.restart()
            revealRight.restart()
            revealCase.restart()
        }

        ScaleAnimator { id: revealLeft; target: leftPodColumn; from: 0.5; to: 1.0; duration: 260; easing.type: Easing.OutBack }
        SequentialAnimation {
            id: revealRight
            PauseAnimation { duration: 90 }
            ScaleAnimator { target: rightPodColumn; from: 0.5; to: 1.0; duration: 260; easing.type: Easing.OutBack }
        }
        SequentialAnimation {
            id: revealCase
            PauseAnimation { duration: 180 }
            ScaleAnimator { target: caseColumn; from: 0.5; to: 1.0; duration: 260; easing.type: Easing.OutBack }
        }
    }

    Connections {
        target: airpodsModel
        function onPopupVisibleChanged() {
            if (airpodsModel.popupVisible) {
                popup.slideIn()
                card.playRevealAnimation()
            } else {
                popup.slideOut()
            }
        }
    }
}
