import Qt 4.7
import QtWebKit 1.0
import SocialConnect 1.0

Item {
    width: 320
    height: 480
    
    Component.onCompleted: smokeConnection.authenticate();

    SmokeConnection {
        id: smokeConnection
        webInterface: webInterface
        clientId: "399096860123557"
        
        onAuthenticatedChanged: console.debug("authenticated = " + authenticated);
        onBusyChanged: console.debug("busy = " + busy);
        onAccessTokenChanged: console.debug("accessToken = " + accessToken);

        onAuthenticateCompleted: {
            console.debug("onAuthenticateCompleted success = " + success);
            retrieveMessages("", "", 8);
        }

        onRetrieveMessagesCompleted: {
            console.debug("onRetrieveMessagesCompleted success = " + success +
                          " messages.length = " + messages.length);
        }
    }

    WebInterface {
        id: webInterface

        onActiveChanged: {
            console.debug("active = " + active);

            if (active) {
                webView.sourceComponent = webViewComponent;
            } else {
                webView.sourceComponent = undefined;
            }
        }

        onUrlChanged: {
            webView.item.url = url;
        }
    }
    
    Loader {
        id: webView
        anchors.fill: parent
    }

    Component {
        id: webViewComponent

        WebView {
            preferredHeight: height
            preferredWidth: width
            onUrlChanged: webInterface.url = url;
        }
    }
}
