import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Effects
import QtMultimedia

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

        // Spinning product render (pods on the left half, case on the right),
        // picked to match the detected model; generations we have no clip for
        // fall back to the 2nd-gen one as the closest generic look.
        Column {
            id: mediaBlock
            visible: airpodsModel.connected
            anchors.top: title.bottom
            anchors.topMargin: 14
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.round(parent.width * 0.68) // leave the card some air
            spacing: 16

            readonly property url videoSource: {
                const m = airpodsModel.modelName
                if (m.indexOf("1st") !== -1) return "qrc:/videos/AirPods_1.avi"
                if (m.indexOf("3rd") !== -1) return "qrc:/videos/AirPods_3.avi"
                return "qrc:/videos/AirPods_2.avi"
            }

            Item {
                width: parent.width
                height: width / 2 // the clips are 2:1

                MediaPlayer {
                    id: player
                    // Don't touch the media stack until there's something to
                    // show - loading at startup spins up decoders while the
                    // popup is still parked off-screen.
                    source: airpodsModel.connected ? mediaBlock.videoSource : ""
                    videoOutput: videoOut
                    loops: MediaPlayer.Infinite
                    // A model change mid-display swaps the source, which stops
                    // playback; kick it off again if we're still on screen.
                    onSourceChanged: if (airpodsModel.popupVisible) play()
                }
                VideoOutput {
                    id: videoOut
                    anchors.fill: parent
                }
            }

            Row {
                // Wider than the clip so the readouts spread outward, sitting
                // under the pods on the left and the case on the right rather
                // than bunching up in the middle.
                width: Math.round(card.width * 0.85)
                anchors.horizontalCenter: parent.horizontalCenter

                Item { // battery readouts sit under the halves they describe
                    width: parent.width / 2
                    height: 22
                    Row {
                        anchors.centerIn: parent
                        spacing: 24
                        BatteryGlyph {
                            level: airpodsModel.batteryLeft
                            charging: airpodsModel.chargingLeft
                        }
                        BatteryGlyph {
                            level: airpodsModel.batteryRight
                            charging: airpodsModel.chargingRight
                        }
                    }
                }
                Item {
                    width: parent.width / 2
                    height: 22
                    BatteryGlyph {
                        anchors.centerIn: parent
                        level: airpodsModel.batteryCase
                        charging: airpodsModel.chargingCase
                    }
                }
            }
        }

        // Reveal: the whole media block scales/fades up when the popup opens,
        // and the spin clip restarts from its first frame.
        function playRevealAnimation() {
            player.position = 0
            player.play()
            revealAnim.restart()
        }

        ParallelAnimation {
            id: revealAnim
            ScaleAnimator { target: mediaBlock; from: 0.85; to: 1.0; duration: 260; easing.type: Easing.OutBack }
            OpacityAnimator { target: mediaBlock; from: 0.0; to: 1.0; duration: 220 }
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
                player.pause() // no point spinning while parked off-screen
            }
        }
    }
}
