/**
 * Copyright (c) 2012 Nokia Corporation.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1

PageStackWindow {
    id: window
    initialPage: mainPage
    showStatusBar: true
    showToolBar: true

    MainPage {
        id: mainPage
    }
}
