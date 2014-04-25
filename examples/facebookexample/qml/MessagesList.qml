/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1

Page {
    property alias model: listView.model
    tools: toolBarLayout

    ToolBarLayout {
        id: toolBarLayout
        ToolButton {
            flat: true
            iconSource: "toolbar-back"
            onClicked: pageStack.pop();
        }
    }

    ListView {
        id: listView
        width: parent.width
        height: 500
        delegate: listDelegate
        model: listModel
        Component {
            id: listDelegate

            ListItem {
                id: listItem

                Column {
                    anchors.fill: listItem.paddingItem

                    ListItemText {
                        elide: Text.ElideRight
                        id: titleText
                        width: parent.width
                        role: "Title"
                        text: message
                    }
                    ListItemText {
                        elide: Text.ElideRight
                        id: subtitleText
                        width: parent.width
                        role: "Subtitle"
                        text: time
                        color: "grey"
                    }
                }
            }
        }
    }
}
