// nullgetapplication.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-09 00:16:45 +0000
// Version: $Id$
// 

#ifndef NULLGETAPPLICATION_H
#define NULLGETAPPLICATION_H

#include <QApplication>
#include <QtSingleApplication>  // add include_once in this file

class NullGetApplication : public QtSingleApplication
{
	Q_OBJECT;

public:
    NullGetApplication(int & argc, char ** argv);
    ~NullGetApplication();

#if defined(Q_OS_WIN32)
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

#endif // NULLGETAPPLICATION_H


