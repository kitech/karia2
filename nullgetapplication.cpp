// nullgetapplication.cpp --- 
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

#include "nullgetapplication.h"

NullGetApplication::NullGetApplication(int & argc, char ** argv)
	: QtSingleApplication(argc,argv)
{

}

NullGetApplication::~NullGetApplication()
{

}

#if defined(Q_OS_WIN32)
bool NullGetApplication::winEventFilter ( MSG * msg, long * result )
{
	//qDebug()<<__FUNCTION__<<__LINE__<<rand();

	//qDebug()<<msg->message ;

	return QApplication::winEventFilter(msg,result);

}
#elif defined(Q_OS_MAC)
bool NullGetApplication::macEventFilter(EventHandlerCallRef caller, EventRef event )
{
    return QApplication::macEventFilter(caller, event);
}
#else
bool NullGetApplication::x11EventFilter ( XEvent * event )
{
	return QApplication::x11EventFilter(event);
}
#endif

void NullGetApplication::handleMessage(const QString &msg)
{
    qDebug()<<"I am running, you say:"<<msg;
}
