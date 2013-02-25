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

#include "libng/qbihash.h"


class TaskOption;
class Aria2StatCollector;

/*
  这一层定义用户动作调用接口
  响应用户动作的接口标准
 */
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
    virtual void taskStatChanged(int tid, QMap<int, QVariant> stats);
    virtual void taskServerStatChanged(int tid, QList<QMap<QString, QString> > stats);
    virtual void taskFinished(int tid, int code);
    
public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to) = 0;
    virtual int removeTask(int task_id)  = 0;
    virtual int startTask(int task_id)  = 0;
    virtual int stopTask(int task_id)  = 0;
    virtual int pauseTask(int task_id)  = 0;
    /////
    virtual bool onAllStatArrived(int stkey) = 0;
                              
protected slots:
    //// from backend
    virtual void onBackendFinished() = 0;
    
protected:
    // > 0 是从下载后端传递过来的事件
    // < 0 是从上端UI传递过来的事件
    // = 0 保留未用
    static QAtomicInt m_counter;
protected:
    QHash<int, void*> m_tasks; // tid => worker/QProcess/something
    QHash<void*, int> m_rtasks; // worker/QProcess/something => tid
    KBiHash<int, void*> m_tasks2; // tid => worker/QProcess/something
    int m_argc;
    char m_argv[32][256];

    // statqueue member
    QQueue<QPair<int, Aria2StatCollector*> > stkeys;
    Aria2StatCollector *lastStat;
    static QAtomicInt doneCounter; // 小于0的counter
};


#endif /* _ARIA2MANAGER_H_ */
