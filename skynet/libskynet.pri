QT *= core gui

SKYNET_HEADERS = $$PWD/xmessages.h $$PWD/skypecommon.h $$PWD/skypecommand.h \
               $$PWD/skype.h
SKYNET_SOURCES = $$PWD/xmessages.cpp $$PWD/skypecommon_X11.cpp $$PWD/skypecommand.cpp \
               $$PWD/skype.cpp

HEADERS += $$SKYNET_HEADERS
SOURCES += $$SKYNET_SOURCES