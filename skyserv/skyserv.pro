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
        ../metauri.cpp \
        streamd.cpp \
        wav_switcher.cpp sua_switcher.cpp PjCallback.cpp sip_entry.cpp

HEADERS  += skyserv.h \
         skyservapplication.h \
         database.h \
          ../metauri.h \
          streamd.h \
          wav_switcher.h sua_switcher.h PjCallback.h sip_entry.h

FORMS    += skyserv.ui

SOURCES += $$QTSAPP_SOURCES
HEADERS += $$QTSAPP_HEADERS

#libskynet 
include(../skynet/libskynet.pri)

LIBS += -lpq

INCLUDEPATH += /serv/stow/pjsip/include
LIBS += -L/serv/stow/pjsip/lib

# LIBS += -lpjsua-x86_64-unknown-linux-gnu \
#     -lpjsip-ua-x86_64-unknown-linux-gnu \
#     -lpjsip-simple-x86_64-unknown-linux-gnu \
#     -lpjsip-x86_64-unknown-linux-gnu \
#     -lpjmedia-codec-x86_64-unknown-linux-gnu \
#     -lpjmedia-x86_64-unknown-linux-gnu \
#     -lpjmedia-codec-x86_64-unknown-linux-gnu \
#     -lpjmedia-audiodev-x86_64-unknown-linux-gnu \
#     -lpjnath-x86_64-unknown-linux-gnu \
#     -lpjlib-util-x86_64-unknown-linux-gnu \
#     -lpj-x86_64-unknown-linux-gnu \
#     -lportaudio-x86_64-unknown-linux-gnu \
#     -lgsmcodec-x86_64-unknown-linux-gnu \
#     -lilbccodec-x86_64-unknown-linux-gnu \
#     -lspeex-x86_64-unknown-linux-gnu \
#     -lresample-x86_64-unknown-linux-gnu \
#     -lmilenage-x86_64-unknown-linux-gnu \
#     -lsrtp-x86_64-unknown-linux-gnu \

LIBS += -lm \
    -lpthread \
    -lssl \
    -lasound

PJSIP_LIBS=$$system("cat /serv/stow/pjsip/lib/pkgconfig/libpjproject.pc | grep Libs | awk -F\"}\" '{print $2}'")
# message($$PJSIP_LIBS)

LIBS += $$PJSIP_LIBS