// asynctask.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-01-30 00:06:29 +0000
// Version: $Id$
// 

#ifndef _ASYNCTASK_H_
#define _ASYNCTASK_H_

#include <QtCore>

class Karia2;

class AsyncTask : public QThread
{
    Q_OBJECT;
public:
    explicit AsyncTask(Karia2 *pwin);
    virtual ~AsyncTask();

    virtual void run();

signals:
    void canFirstShow();
    void canAsyncFirstShow();
private:
    Karia2 *mpwin;

};

#endif /* _ASYNCTASK_H_ */
