import QtQuick 1.1
import QtWebKit 1.0
import SocialConnect 1.0

Item {
    width: 360
    height: 640
    
    Component.onCompleted: {
        twitterConnection.restoreCredentials();
        console.log("QML: RestoreCredentials called.\n" +
                    "        isAuthenticated: " + twitterConnection.authenticated + "\n" +
                    "        userName: " + twitterConnection.name + "\n" +
                    "        access_token: " + twitterConnection.accessToken + "\n" +
                    "        access_token_secret: " + twitterConnection.accessTokenSecret);
        twitterConnection.authenticate();
    }

    // Enable for testing deauthentication.
//    Component.onDestruction: {
//        twitterConnection.deauthenticate();
//        twitterConnection.removeCredentials();
//    }

    TwitterConnection {
        id: twitterConnection

        webInterface: webInterface
        consumerKey: "8X8HXL5PuNIPpuieYICVQ"
        consumerSecret: "GCJgN1mh4rtBjB8ZjDaOEoZ83hqbOyjufJThw50I"
        callbackUrl: "http://projects.developer.nokia.com/socialconnect"

        onAuthenticatedChanged: {
            console.log("QML: Twitter authenticated = " + authenticated);
        }

        onAuthenticateCompleted: {
            webViewLoader.sourceComponent = msgView;
            webViewLoader.item.text = "Twitter onAuthenticateCompleted success = " + success;
            console.log("QML: Twitter onAuthenticateCompleted success = " + success);
        }

        onDeauthenticateCompleted: {
            console.log("QML: DeAuthenticate completed: " + success);
        }

        onPostMessageCompleted: {
            console.log("QML: Posting message completed: " + success);
        }

        onRetrieveMessageCountCompleted: {
            console.log("QML: Retrieving completed: " + success
                        + ". Message count: " + count);
        }

        onRetrieveMessagesCompleted: {
            console.log("QML: onRetrieveMessages completed: " + success
                        + ". JSON Stringified: " + JSON.stringify(messages));
            for (var i = 0; i < messages.length; i++ ) {
                var message = messages[i];
                console.log("QML: MSG ID: " + message.id + " TXT: " + message.text);
            }
        }
    }

    Loader {
        id: webViewLoader
        anchors.fill: parent
        width: parent.width
        height: parent.height
        sourceComponent: webView
    }

    WebInterface {
        id: webInterface
        onUrlChanged: webViewLoader.item.url = url;
    }

    // WebView to show the OAuth login, if needed.
    Component {
        id: webView

        WebView {
            onUrlChanged: webInterface.url = url;

            Component.onDestruction: {
                console.log("QML: Storing credentials, was authenticated: "
                            + twitterConnection.authenticated)
                twitterConnection.storeCredentials();
            }
        }
    }

    // Component, that has buttons to demonstrate different SocialConnect API's
    // TwitterConnection Plugin functionalities.
    Component {
        id: msgView

        Item {
            id: root

            property alias text: txt.text
            anchors.fill: parent

            Text {
                id: txt
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Grid {
                spacing: 20
                anchors.top: parent.top
                anchors.topMargin: 50
                anchors.horizontalCenter: parent.horizontalCenter

                rows: 3
                columns: 2

                // Button demonstrating sending a status update to Twitter
                // with the QML TwitterConnection plugin.
                TwitterButton {
                    text: "Send Status!"
                    onClicked: twitterConnection.postMessage(
                                   {"text": "Date in ms: " + Date.now()});
                }

                // Button demonstrating sending a status update with image
                // to Twitter with the TwitterConnection.
                TwitterButton {
                    text: "Send Image!"
                    onClicked: twitterConnection.postMessage(
                                   {"text": "PicUpload Date in ms: " + Date.now(),
                                       "url": "mediabrowservideo1.png"});
                }

                // Button demonstrating getting user's Twitter message count
                // with the TwitterConnection.
                TwitterButton {
                    text: "Get msg count!"
                    onClicked: twitterConnection.retrieveMessageCount()
                }

                // Button demonstrating fetching user's own Twitter timeline
                // with the TwitterConnection.
                TwitterButton {
                    text: "Get messages!"

                    // Use this to get the 30 most recent Tweets.
//                    onClicked: twitterConnection.retrieveMessages("", "", 30);

                    // Use this to get 20 tweets, older than the Tweet identified
                    // as the "207044923478904830". Uses Twitter API's "max_id".
//                    onClicked: twitterConnection.retrieveMessages(
//                                   "", "202309940713496580", 20);

                    // Use this to get 20 tweets, that are more recent than the Tweet
                    // identified as the "207837441565011970". If there's less than
                    // the given amount, it'll return only those. Uses Twitter API's
                    // "since_id".
//                    onClicked: twitterConnection.retrieveMessages(
//                                   "207837441565011970", "", 20);

                    // Use this to get 15 tweets, older than the Tweet identified
                    // as the "207044923478904830", but newer than the "already shown"
                    // Tweet id: "204845668941053950". I.e. example using the
                    // "since_id" & "max_id" both correctly with Twitter.
                    onClicked: twitterConnection.retrieveMessages(
                                   "204845668941053950", "208101028405575680", 15);

                }

                // Button for cancelling whatever request might be ongoing.
                TwitterButton {
                    text: "Abort! Abort!"
                    onClicked: twitterConnection.cancel()
                }

                TwitterButton {
                    text: "Authenticate!"
                    onClicked: {
                        webViewLoader.sourceComponent = webView;
                        twitterConnection.authenticate();
                    }
                }
            }
        }
    }
}
