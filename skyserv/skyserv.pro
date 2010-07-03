#-------------------------------------------------
#
# Project created by QtCreator 2010-07-03T10:44:47
#
#-------------------------------------------------

QT       += core gui

TARGET = skyserv
TEMPLATE = app

UI_HEADERS_DIR = GeneratedFiles
MOC_DIR = tmp
UI_DIR = tmp
OBJECTS_DIR = tmp

VERSION = 0.0.97

#########################
INCLUDEPATH += ../qtsingleapplication ../skynet

SOURCES += main.cpp\
        skyserv.cpp \
        ../metauri.cpp

HEADERS  += skyserv.h \
          ../metauri.h

FORMS    += skyserv.ui

#libskynet
include(../skynet/libskynet.pri)

