# karia2.pro --- 
# 
# Author: liuguangzhao
# Copyright (C) 2007-2013 liuguangzhao@users.sf.net
# URL: 
# Created: 2010-04-03 19:19:40 +0800
# Version: $Id$
# 
######################################################################
# Automatically generated by qmake (2.01a) ??? ?? 11 17:28:45 2006
######################################################################

TEMPLATE = app
TARGET = karia2
DEPENDPATH += . GeneratedFiles
INCLUDEPATH += .
QT = core gui xml network sql widgets

win32 {
      CONFIG += release
} else {
      CONFIG += debug
    #QMAKE_CC = clang       # clang compiled karia2 crash
    #QMAKE_CXX = clang++
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_LFLAGS += -g -O2 -std=c++11
}

DESTDIR = ./bin
UI_HEADERS_DIR = GeneratedFiles
MOC_DIR = tmp
UI_DIR = tmp
OBJECTS_DIR = tmp
#CONFIG += qtestlib

VERSION = 0.1.62

#########################
INCLUDEPATH += ./libmaia/ ./qtsingleapplication ./skynet

# karia2 realy need this?
unix{
    LIBS += -lssl -lX11
}
win32 {
    INCLUDEPATH +=Z:/cross/boost152/include 
	win32-g++ {
		# -mwindows can drop the dos window when app run. got this knowleage at dev-cpp's makefile generate function
        #LIBS += -LD:/msys/1.0/local/ssl/lib/ -mwindows -lssl -lcrypto -lwsock32 -lgdi32
  	    #LIBS += -LD:/msys/1.0/local/ssl/lib/ -mwindows -lssl -lcrypto -lws2_32 -lgdi32
  	    LIBS += -LD:/librarys/mw-ssl/lib/ -mwindows -lssl -lcrypto -lws2_32 -lgdi32 -lpsapi
        INCLUDEPATH += D:/librarys/mw-ssl/include 	
        QMAKE_LFLAGS_WINDOWS = 
	} else {
      # LIBS += wsock32.lib  E:/library/openssl/lib/libeay32.lib E:/library/openssl/lib/ssleay32.lib 
      # LIBS += ws2_32.lib  E:/library/openssl/lib/libeay32.lib E:/library/openssl/lib/ssleay32.lib 

        ## check cl.exe, x64 or x86
        CLARCH=$$system(path)
        VAMD64=$$find(CLARCH,amd64)

        isEmpty(VAMD64) {
            # from qt 4.7, use QMAKE_LIBDIR instead of LIBPATH
            LIBPATH += Z:/librarys/vc-ssl-x86/lib Z:/librarys/vc-zlib/static32   # depcreated
            QMAKE_LIBDIR += Z:/librarys/vc-ssl-x86/lib Z:/librarys/vc-zlib/static32
            INCLUDEPATH += Z:/librarys/vc-ssl-x86/include/
        } else {
             LIBPATH += Z:/librarys/vc-ssl-x64/lib Z:/librarys/vc-zlib/staticx64 # depcreated
             QMAKE_LIBDIR += Z:/librarys/vc-ssl-x64/lib Z:/librarys/vc-zlib/staticx64
            INCLUDEPATH += Z:/librarys/vc-ssl-x64/include/
        }

      #      LIBPATH += Z:/librarys/vc-ssl/lib Z:/librarys/vc-zlib/lib
      # INCLUDEPATH += Z:/librarys/vc-ssl/include/
      LIBS += -lqtmain -lzlibstat -llibeay32 -lssleay32 -ladvapi32 -luser32 -lpsapi -lws2_32
      RESOURCES = karia2.rc
      # QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:console
    }
} else {
    ARIA2_ROOT = ./aria2-1.18.0
    INCLUDEPATH += $$ARIA2_ROOT/src/ $$ARIA2_ROOT  $$ARIA2_ROOT/lib $$ARIA2_ROOT/src/includes
    LIBS += -L$$ARIA2_ROOT/src/ -laria2 -L$$ARIA2_ROOT/deps/wslay/lib/.libs/ -lwslay
    LIBS += -lgcrypt  -lrt -lnettle -lgmp -lz   -lcares   -lgnutls   -lsqlite3 -lxml2 -lz -lm
}

DEFINES += "KARIA2_VERSION=\\\"$$VERSION\\\""
include(./install.pri)
DEFINES += NXDATADIR=\"\\\"$$DATADIR\\\"\"
DEFINES += NXPKGDATADIR=\"\\\"$$PKGDATADIR\\\"\"
DEFINES += THREADSAFE=1   # for sqlite thread-safe feature
DEFINES += HAVE_CONFIG_H # for aria2c source code.

#添加这一块可以使得在windows上编译出来的程序有图标
win32 {
   RC_FILE	= karia2.rc
}    

## libmaia
MAIA_HEADERS = libmaia/maiaObject.h libmaia/maiaFault.h libmaia/maiaXmlRpcClient.h \
             libmaia/maiaXmlRpcServer.h libmaia/maiaXmlRpcServerConnection.h
MAIA_SOURCES = libmaia/maiaObject.cpp libmaia/maiaFault.cpp libmaia/maiaXmlRpcClient.cpp \
             libmaia/maiaXmlRpcServer.cpp libmaia/maiaXmlRpcServerConnection.cpp

QTSAPP_HEADERS = qtsingleapplication/qtsingleapplication.h qtsingleapplication/qtlocalpeer.h
QTSAPP_SOURCES = qtsingleapplication/qtsingleapplication.cpp qtsingleapplication/qtlocalpeer.cpp

## libskynet
# include(./skynet/libskynet.pri)

## qtxmlrpc library
# include(./qxmlrpc/client/client.pri)

## qjson library
include (./qjson/qjson.pri)

## qjsonrpc library
include (./qjsonrpc/src/qjsonrpc.pri)

# Input
HEADERS += aboutdialog.h \
           abstractstorage.h \
           batchjobmandlg.h \
           catmandlg.h \
           catpropdlg.h \
           dlrules.h \
           dropzone.h \
           dmstatusbar.h \
           dircompletermodel.h	\
           hash.h \
           instantspeedhistogramwnd.h \
           labspace.h \
           nullcontroller.h \
           karia2.h \
           karia2application.h \
           asynctask.h \
           radarscanner.h \
           resource.h \
           rulesmandlg.h \
           segmentlogmodel.h \
           serverswitcher.h \
           # skypeclientwnd.h \
           sqlitecategorymodel.h \
           sqlitesegmentmodel.h \
           sqlitestorage.h \
           sqlitetaskmodel.h \
           storageexception.h \
           storagefactory.h \
           taskballmapwidget.h \
           taskinfodisgestwnd.h \
           taskinfodlg.h \
           taskqueue.h \
           utility.h \
           walksitewnd.h \
           walksitewndex.h \
           webpagelinkdlg.h \
           xmlstorage.h \   
           optionmanager.h \
           preferencesdialog.h	\        
           ariaman.h      \
           libng/html-parse.h \
           libng/md5.h \
           torrentpeermodel.h \
           taskservermodel.h \
           seedfilemodel.h \
           seedfilesdialog.h \  
           taskitemdelegate.h \
#           skypetracer.h \
#           skypetunnel.h \
           simplelog.h \
           asyncdatabase.h \
           databaseworker.h \
           emaria2c.h  \
           karia2statcalc.h \
           aria2manager.h \
           aria2embedmanager.h \
           aria2libaria2manager.h \
           aria2rpcserver.h \
           aria2rpcmanager.h \
           aria2jsonmanager.h \
           aria2xmlmanager.h \
           aria2managerfactory.h

win32 {
    HEADERS += DiskInfo.h	
}      

SOURCES += aboutdialog.cpp \
           abstractstorage.cpp \
           batchjobmandlg.cpp \
           catmandlg.cpp \
           catpropdlg.cpp \
           dlrules.cpp \
           dropzone.cpp \
           dmstatusbar.cpp \
           dircompletermodel.cpp	\
           hash.cpp \
           instantspeedhistogramwnd.cpp \
           labspace.cpp \
           main.cpp \
           nullcontroller.cpp \
           karia2.cpp \
           karia2application.cpp \
           karia2_embed_aria2.cpp \
           karia2_standalone_aria2.cpp \
           asynctask.cpp \
           radarscanner.cpp \
           rulesmandlg.cpp \
           segmentlogmodel.cpp \
           serverswitcher.cpp \
#           skypeclientwnd.cpp \
           sqlitecategorymodel.cpp \
           sqlitesegmentmodel.cpp \
           sqlitestorage.cpp \
           sqlitetaskmodel.cpp \
           storageexception.cpp \
           storagefactory.cpp \
           taskballmapwidget.cpp \
           taskinfodisgestwnd.cpp \
           taskinfodlg.cpp \
           taskqueue.cpp \
           utility.cpp \
           walksitewnd.cpp \
           walksitewndex.cpp \
           webpagelinkdlg.cpp \
           xmlstorage.cpp \      
           optionmanager.cpp \
           preferencesdialog.cpp	\     
           ariaman.cpp \
           libng/html-parse.c \
           libng/md5.c  \
           libng/qtmd5.cpp \
           torrentpeermodel.cpp \
           taskservermodel.cpp \
           seedfilemodel.cpp \
           seedfilesdialog.cpp \
           taskitemdelegate.cpp \
           mimetypeshash.cpp \
#           skypetracer.cpp \
           metauri.cpp \
#           skypetunnel.cpp \
           simplelog.cpp \
           asyncdatabase.cpp \
           databaseworker.cpp \
           emaria2c.cpp  \
           karia2statcalc.cpp \ # aria2-1.13.0/src/option_processing.cc \
           aria2manager.cpp  \
           aria2embedmanager.cpp \
           aria2libaria2manager.cpp \
           aria2rpcserver.cpp \
           aria2rpcmanager.cpp \
           aria2jsonmanager.cpp \
           aria2xmlmanager.cpp \
           aria2managerfactory.cpp

win32 {
    SOURCES += DiskInfo.cpp
}  

FORMS += aboutdialog.ui \
         batchjobmandlg.ui \
         catmandlg.ui \
         catpropdlg.ui \
         columnsmandlg.ui \
         dlrules.ui \
           dmstatusbar.ui \
         karia2.ui \
         rulesmandlg.ui \
         # skypeclientwnd.ui \
         taskinfodisgestwnd.ui \
         taskinfodlg.ui \
         walksitewnd.ui \
         walksitewndex.ui \
         webpagehostselectdlg.ui \
         webpagelinkdlg.ui \
         webpageurlinputdlg.ui	\
         labspace.ui				\
         preferencesdialog.ui  \
         seedfilesdialog.ui \
         proxyinfodialog.ui
         # skypetracer.ui

HEADERS += $$MAIA_HEADERS 
SOURCES += $$MAIA_SOURCES 

HEADERS += $$QTSAPP_HEADERS
SOURCES += $$QTSAPP_SOURCES

# HEADERS += $$SKYNET_HEADERS
# SOURCES += $$SKYNET_SOURCES

TRANSLATIONS += translations/karia2_en_US.ts \
                translations/karia2_zh_CN.ts \
                translations/karia2_zh_TW.ts

RESOURCES = karia2.qrc

documentation.path = /
documentation.files = release/*.exe
documentation.extra = cp release/*.exe Z:/temp

INSTALLS += documentation
