# skyserv.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2010 liuguangzhao@users.sf.net
# URL: 
# Created: 2010-07-03 14:03:38 +0800
# Version: $Id$
# 

QT       += core gui network
TARGET = skyserv
TEMPLATE = app
CONFIG += debug
UI_HEADERS_DIR = GeneratedFiles
MOC_DIR = tmp
UI_DIR = tmp
OBJECTS_DIR = tmp

VERSION = 0.0.97

#########################
INCLUDEPATH += /usr/include/postgresql/              # for ubuntu, the include header not in standard /usr/include
INCLUDEPATH += ../qtsingleapplication ../skynet ..

QTSAPP_HEADERS = ../qtsingleapplication/qtsingleapplication.h ../qtsingleapplication/qtlocalpeer.h
QTSAPP_SOURCES = ../qtsingleapplication/qtsingleapplication.cpp ../qtsingleapplication/qtlocalpeer.cpp

SOURCES += main.cpp\
        skyserv.cpp \
        skyservapplication.cpp \
        database.cpp \
        ../metauri.cpp

HEADERS  += skyserv.h \
         skyservapplication.h \
         database.h \
          ../metauri.h

FORMS    += skyserv.ui

SOURCES += $$QTSAPP_SOURCES
HEADERS += $$QTSAPP_HEADERS

#libskynet 
include(../skynet/libskynet.pri)

LIBS += -lpq