# Copyright (c) 2012 Nokia Corporation.

QT += declarative network script
CONFIG += qt plugin

HEADERS += \
    $$PWD/src/socialconnectplugin.h \
    $$PWD/src/socialconnection.h \
    $$PWD/src/webinterface.h

SOURCES += \
    $$PWD/src/socialconnectplugin.cpp \
    $$PWD/src/socialconnection.cpp \
    $$PWD/src/webinterface.cpp

INCLUDEPATH += $$PWD/src

# Smoke (base debugging implementation)
DEFINES += ENABLE_SMOKE_CONNECTION
HEADERS += $$PWD/src/smoke/smokeconnection.h
SOURCES += $$PWD/src/smoke/smokeconnection.cpp

# Facebook
HEADERS += \
    $$PWD/src/facebook/facebookconnection.h \
    $$PWD/src/facebook/facebook.h \
    $$PWD/src/facebook/facebookrequest.h \
    $$PWD/src/facebook/facebookreply.h \
    $$PWD/src/facebook/facebookdatamanager.h

SOURCES += \
    $$PWD/src/facebook/facebookconnection.cpp \
    $$PWD/src/facebook/facebook.cpp \
    $$PWD/src/facebook/facebookrequest.cpp \
    $$PWD/src/facebook/facebookreply.cpp \
    $$PWD/src/facebook/facebookdatamanager.cpp

# Twitter
HEADERS += \
    $$PWD/src/twitter/twitterconnection.h \
    $$PWD/src/twitter/twitterrequest.h \
    $$PWD/src/twitter/twitterconstants.h

SOURCES += \
    $$PWD/src/twitter/twitterconnection.cpp \
    $$PWD/src/twitter/twitterrequest.cpp
