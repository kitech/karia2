// karia2application.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-17 23:14:21 +0800
// Version: $Id$
// 

#include <QtCore>

#ifdef Q_OS_WIN32
#include <windows.h>
#else
#endif

#include "karia2application.h"

Karia2Application::Karia2Application(int & argc, char ** argv)
	: QtSingleApplication(argc, argv)
{

}

Karia2Application::~Karia2Application()
{

}

#if defined(Q_OS_WIN)
bool Karia2Application::eventHandled=false;
long Karia2Application::eventResult=0;

bool Karia2Application::winEventFilter ( MSG * msg, long * result )
{
	//qDebug()<<__FUNCTION__<<__LINE__<<rand();

	//qDebug()<<msg->message ;
    eventHandled=false;
    emit winMessage( msg );
    if ( eventHandled )
      *result = eventResult;
    return eventHandled;


    //	return QApplication::winEventFilter(msg,result);

}
#elif defined(Q_OS_MAC)
bool Karia2Application::macEventFilter(EventHandlerCallRef caller, EventRef event )
{
    return QApplication::macEventFilter(caller, event);
}
#else
#include <X11/Xlib.h>
#include "xmessages.h"
bool Karia2Application::x11EventFilter(XEvent *event)
{
    switch(event->type) {
    case ClientMessage:
        XMessages::processXMessages(event);
        break;
    }
    // return FALSE;
	return QApplication::x11EventFilter(event);
}
#endif

void Karia2Application::handleMessage(const QString &msg)
{
    // qDebug()<<"I am running, you say:"<<msg;
}
