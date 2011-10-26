// nullgetapplication.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-09 00:16:45 +0000
// Version: $Id: nullgetapplication.h 143 2010-06-29 15:23:14Z drswinghead $
// 

#ifndef SKYSERVAPPLICATION_H
#define SKYSERVAPPLICATION_H

#include <QApplication>
#include <QtSingleApplication>  // add include_once in this file

class SkyServApplication : public QtSingleApplication
{
	Q_OBJECT;
public:
    SkyServApplication(int & argc, char ** argv);
    virtual ~SkyServApplication();

#if defined(Q_OS_WIN)
	virtual bool winEventFilter ( MSG * msg, long * result );
#elif defined(Q_OS_MAC)
    virtual bool macEventFilter(EventHandlerCallRef caller, EventRef event ) ;
#else
	virtual bool x11EventFilter ( XEvent * event ) ;
#endif

public slots:
    void handleMessage(const QString &msg);

private:
    
};

#endif // SKYSERVAPPLICATION_H


