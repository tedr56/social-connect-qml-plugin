# Copyright (c) 2012 Nokia Corporation.

TEMPLATE = lib
QT += declarative network script
TARGET = socialconnect
CONFIG += qt plugin

HEADERS += \
    src/socialconnectplugin.h \
    src/socialconnection.h \
    src/webinterface.h

SOURCES += \
    src/socialconnectplugin.cpp \
    src/socialconnection.cpp \
    src/webinterface.cpp

INCLUDEPATH += src

# Smoke (base debugging implementation)
DEFINES += ENABLE_SMOKE_CONNECTION
HEADERS += src/smoke/smokeconnection.h
SOURCES += src/smoke/smokeconnection.cpp

# Facebook
HEADERS += \
    src/facebook/facebookconnection.h \
    src/facebook/facebook.h \
    src/facebook/facebookrequest.h \
    src/facebook/facebookreply.h \
    src/facebook/facebookdatamanager.h

SOURCES += \
    src/facebook/facebookconnection.cpp \
    src/facebook/facebook.cpp \
    src/facebook/facebookrequest.cpp \
    src/facebook/facebookreply.cpp \
    src/facebook/facebookdatamanager.cpp

# Twitter
HEADERS += \
    src/twitter/twitterconnection.h \
    src/twitter/twitterrequest.h \
    src/twitter/twitterconstants.h

SOURCES += \
    src/twitter/twitterconnection.cpp \
    src/twitter/twitterrequest.cpp

# Platform specific files and configuration
symbian {
    VERSION = 1.0.0
    TARGET.CAPABILITY = ALL -TCB
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.UID3 = 0xED6AB47E
    deployment_files.sources = socialconnect.dll qmldir
    deployment_files.path = $$QT_IMPORTS_BASE_DIR/SocialConnect
    DEPLOYMENT += deployment_files
} else {
    qmldir.files = qmldir

    unix {
        qmldir.path = ../install/SocialConnect
        target.path = ../install/SocialConnect
    } else {
        qmldir.path = $$[QT_INSTALL_IMPORTS]/SocialConnect
        target.path = $$[QT_INSTALL_IMPORTS]/SocialConnect
    }

    INSTALLS += target qmldir
}
