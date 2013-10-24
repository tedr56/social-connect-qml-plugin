// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1
import QtWebKit 1.0
import SocialConnect 1.0

Rectangle {
    width: 800
    height: 500
    id: main

    MouseArea {
        anchors.fill: parent
        onClicked: Qt.quit();
    }

    Component.onCompleted: {
        facebookConnection.restoreCredentials();
        if (!facebookConnection.authenticated) {
            facebookConnection.authenticate();
        }
    }

    FacebookConnection {
        id: facebookConnection
        webInterface: webInterface
        permissions: ["publish_stream", "read_stream", "friends_status"]
        clientId: "399096860123557"
        onAuthenticateCompleted: {
            console.debug("Auth: " + success);
            if (success) {
                facebookConnection.storeCredentials();
                proceed();
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
    }

    function proceed() {
        if (!facebookConnection.busy) {
            webView.opacity = 0;
            facebookConnection.postMessage({"text" : "Hello world!"});
        }
    }
}
