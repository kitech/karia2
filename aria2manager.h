// aria2manager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-20 22:17:48 +0000
// Version: $Id$
// 
#ifndef _ARIA2MANAGER_H_
#define _ARIA2MANAGER_H_


#include <QtCore>


class TaskOption;

class Aria2Manager : public QThread
{
    Q_OBJECT;
public:
    enum class BackendType {
        BT_UNDEFINED = 0, BT_EMBEDED = 1, BT_XML_ON_HTTP, BT_XML_ON_WEBSOCKET,
        BT_JSON_ON_HTTP, BT_JSON_ON_WEBSOCKET
    };
    Aria2Manager();
    virtual ~Aria2Manager();
    
    virtual void run();
    
signals:
    void taskStatChanged(int tid, QMap<int, QVariant> stats);
    void taskFinished(int tid, int code);
    
public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to) = 0;
    virtual int removeTask(int task_id)  = 0;
    virtual int startTask(int task_id)  = 0;
    virtual int stopTask(int task_id)  = 0;
    virtual int pauseTask(int task_id)  = 0;
                              
protected slots:
    //// from backend
    virtual void onBackendFinished() = 0;
    
protected:
    // > 0 是从下载后端传递过来的事件
    // < 0 是从上端UI传递过来的事件
    // = 0 保留未用
    static QAtomicInt m_counter;
};


#endif /* _ARIA2MANAGER_H_ */
