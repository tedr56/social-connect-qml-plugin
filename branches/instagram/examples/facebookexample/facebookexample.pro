# Copyright (c) 2012-2014 Microsoft Mobile.

QT += declarative network
CONFIG += qt-components

TARGET = FacebookExample
TEMPLATE = app
VERSION = 1.0

include (../../plugin/plugin.pri)

SOURCES += main.cpp
OTHER_FILES = qml/*
RESOURCES += rsc/resources.qrc

symbian {
    TARGET.CAPABILITY += NetworkServices
    TARGET.UID3 = 0xED6AB23A
}
