/****************************************************************************
**
** Copyright (c) 2011-2014 Microsoft Mobile and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 1.1
import QtWebKit 1.0
import SocialConnect 1.0

Item {
    id: screen; width: 300; height: 480

    property variant retrievedMessages: []

    state: "deauthenticated"

    Component.onCompleted: {
        facebookConnection.restoreCredentials();
        if (!facebookConnection.authenticated) {
            facebookConnection.authenticate();
        }
        else {
            proceed();
        }
    }

    function proceed() {
        state = "authenticated";
    }

    FacebookConnection {
        id: facebookConnection
        webInterface: webInterface
        permissions: ["publish_stream", "read_stream", "friends_status"]
        clientId: "399096860123557"
        onNameChanged: textArea.text += "\nWelcome " + name + "!";

        onAuthenticateCompleted: {
            facebookConnection.storeCredentials();
            proceed();
        }

        onDeauthenticateCompleted: Qt.quit();

        onRetrieveMessagesCompleted: {
            retrievedMessages = messages;
            console.debug("lngth:" + messages.length);
            for (var i = 0; i < messages.length; i++) {
                textArea.text += "\n" + messages[i]["text"];
            }
        }

        onRetrieveMessageCountCompleted: {
            textArea.text += "\nMsg count:" + count;
        }

        onPostMessageCompleted: {
            if (success) {
                textArea.text += "\nMessage posted";
            }
            else {
                textArea.text += "\nMessage posting failed";
            }
        }
    }

    WebInterface {
        id: webInterface
        onUrlChanged: webView.url = url;
    }

    WebView {
        id: webView
        anchors.fill: parent
        preferredHeight: height
        preferredWidth: width
        onUrlChanged: webInterface.url = url;

        Behavior on opacity { NumberAnimation { duration: 300 } }
    }

    Rectangle {
        id: background
        anchors.fill: parent; color: "#343434";
        opacity: 0

        Flickable {
            z: 2
            height: parent.height
            width: parent.width
            anchors.top: buttonRow2.bottom
            anchors.bottom: parent.bottom
            contentHeight: textArea.height
            Text {
                anchors.fill: parent
                id: textArea
                color: "white"
            }
        }

        Behavior on opacity { NumberAnimation { duration: 300 } }

        Image {
            source: "images/stripes.png";
            fillMode: Image.Tile;
            anchors.fill: parent;
            opacity: 0.3
        }

        MouseArea {
            anchors.fill: parent
            onClicked: screen.focus = false;
        }

        Item {
            id: views
            x: 2; width: parent.width - 4
            y:60
            height: parent.height - 100

        }

        Row {
            z: 3
            id: buttonRow
            anchors.top: parent.top

            Button {
                id: button1
                width: 100; height: 32
                focus:true
                text: "Post image"
                onClicked: {
                    facebookConnection.postMessage({"text" : "Hello from socialexample!",
                                                    "url" : "file:///socialconnectexample80.png"});
                }
            }
            Button {
                id: button2
                width: 100; height: 32
                text: "Post msg"
                onClicked: {
                    facebookConnection.postMessage({"text" : "Hello from socialexample!"});
                }
            }

            Button {
                id: button3
                width: 100; height: 32
                text: "Retrieve msgs"
                onClicked: {
                    facebookConnection.retrieveMessages("", "", 10);
                }
            }
        }
        Row {
            z: 3
            id: buttonRow2
            anchors.top: buttonRow.bottom

            Button {
                id: button4
                width: 100; height: 32
                focus:true
                text: "Retr. msg cnt"
                onClicked: {
                    facebookConnection.retrieveMessageCount();
                }
            }
            Button {
                id: button5
                width: 100; height: 32
                focus:true
                text: "Deauthenticate"
                onClicked: {
                    facebookConnection.deauthenticate();
                }
            }
//            Button {
//                id: button6
//                width: 100; height: 32
//                text: "Retrieve 5 more"
//                onClicked: {
//                    facebookConnection.retrieveMessages("", "", 10);
//                }
//            }
        }


    }
    states: [
        State {
            name: "authenticated"
            PropertyChanges { target: webView; opacity: 0 }
            PropertyChanges { target: background; opacity: 1 }
        },
        State {
            name: "deauthenticated"
            PropertyChanges { target: webView; opacity: 1 }
            PropertyChanges { target: background; opacity: 0 }
        }
    ]
}

