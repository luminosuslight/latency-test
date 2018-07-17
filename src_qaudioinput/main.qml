import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Latency Test")

    Column {
        x: 20
        y: 20
        spacing: 20
        Text {
            font.pixelSize: 20
            text: "Input: " + audioInput.deviceName()
        }
        Text {
            font.pixelSize: 20
            text: "Chosen Buffer Size: " + audioInput.bufferSize() + " Samples"
        }
        Text {
            font.pixelSize: 20
            text: "Actual Chunk Size: " + audioInput.lastChunkSize + " Samples"
        }
    }

    Rectangle {
        id: volumeLevel
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 60
        width: 20
        height: 200
        color: "#888"

        Rectangle {
            width: parent.width
            height: parent.height * audioInput.lastPeakValue
            anchors.bottom: parent.bottom
            color: "blue"
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: volumeLevel.bottom
        anchors.topMargin: 20
        font.pixelSize: 18
        text: "Peak Level"
    }
}
