/**
 * Copyright (c) 2012 Nokia Corporation.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import SocialConnect 1.0
import QtWebKit 1.0

Page {
    id: mainPage

    ToolBarLayout {
        id: toolBarLayout
        ToolButton {
            flat: true
            iconSource: "toolbar-back"
            onClicked: {
                facebookConnection.removeCredentials();
                Qt.quit();
            }
        }
    }

    tools: toolBarLayout

    Component.onCompleted: {
        // Try restoring credentials. If access token not found or
        // it has been expired, authenticate.
        facebookConnection.restoreCredentials()
        if (!facebookConnection.authenticated) {
            facebookConnection.authenticate();
        }
    }

    FacebookConnection {
        id: facebookConnection
        webInterface: webInterface
        permissions: ["publish_stream", "read_stream", "friends_status"]
        clientId: "399096860123557" // Replace with your own application id.
        onNameChanged: helloText.text += "\nWelcome " + name +
                       "! This is a FacebookConnection example program.";
        property string lastId: ""

        onAuthenticateCompleted: {
            if (success) {
                facebookConnection.storeCredentials();
            }

            webView.destroy(); // Destroy WebView after authentication.
            container.opacity = 1;
        }

        onDeauthenticateCompleted: Qt.quit();

        // Show busy indicator when sending requests to the network.
        onTransmittingChanged: {
            busyIndicator.visible = transmitting;
        }

        onRetrieveMessagesCompleted: {
            lastId = messages[messages.length - 1].time;
            for (var i = 0; i < messages.length; i++) {
                if (messages[i].text !== "") {
                    // Convert Unix timestamp to human readable time.
                    var time = new Date(messages[i].time * 1000);
                    var timeStr = time.getMonth() + "/" +
                            time.getDate() + "/" +
                            time.getFullYear() + " " +
                            time.getHours() + ":" +
                            time.getMinutes() + "." +
                            time.getSeconds();

                    listModel.append({"message" : messages[i].text, "time" : timeStr});
                }
            }
            pageStack.push("qrc:/MessagesList.qml", {model: listModel});
        }
    }

    WebView {
        id: webView
        anchors.fill: parent
        preferredHeight: height
        preferredWidth: width
        onUrlChanged: webInterface.url = url;
    }

    WebInterface {
        id: webInterface
        onUrlChanged: webView.url = url;
    }

    Column {
        id: container
        width: parent.width
        spacing: 20
        opacity: 0

        Label {
            id: helloText
            wrapMode: Text.WordWrap
            width: parent.width
            z: 2
        }

        Button {
            width: parent.width
            text: "Post a test message"
            onClicked: facebookConnection.postMessage({"text" : "Hello!"});
            z: 2
        }
        Button {
            width: parent.width
            text: "Retrieve messages"
            onClicked: {
                facebookConnection.retrieveMessages("", facebookConnection.lastId, 20);
            }
            z: 2
        }

        ListModel {
            id: listModel
        }
        Behavior on opacity { NumberAnimation { duration: 300 } }
    }

    BusyIndicator {
        id: busyIndicator
        width: 100
        height: 100
        anchors.centerIn: parent
        running: visible ? true : false;
        visible: false
        Behavior on opacity { NumberAnimation { duration: 300 } }
    }
}
