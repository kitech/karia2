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

namespace ng {
    namespace stat2 {
        enum {
            task_id=1, gid, total_length, completed_length, completed_percent, download_speed, upload_speed,
            bitfield, hex_bitfield, piece_length, num_pieces, num_connections, eta, str_eta,
            num_active, num_waiting, num_stopped,
            error_code, error_string, status, belongs_to
        };
    };
};

/*
  这一层定义用户动作调用接口
  响应用户动作的接口标准
 */
class Aria2Manager : public QThread
{
    Q_OBJECT;
public:
    enum BackendType {
        BT_UNDEFINED = 0, BT_EMBEDED = 1, BT_LIBARIA2,
        BT_XMLRPC_HTTP, 
        BT_JSONRPC_HTTP, BT_JSONRPC_WS, BT_JSONRPC_WSS,
        BT_JSONRPC_CWS, BT_JSONRPC_CWSS,

    };
    explicit Aria2Manager();
    virtual ~Aria2Manager();
    
    virtual void run();
    virtual void cleanup();
    
signals:
    /*virtual */void taskStatChanged(int tid, QMap<int, QVariant> stats);
    /*virtual */void taskServerStatChanged(int tid, QList<QMap<QString, QString> > stats);
    /*virtual */void taskFinished(int tid, int code);
    /*virtual */void globalStatChanged(QMap<int, QVariant> stats);
    
public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to) = 0;
    // virtual int removeTask(int task_id)  = 0;
    // virtual int startTask(int task_id)  = 0;
    // virtual int stopTask(int task_id)  = 0;
    virtual int pauseTask(int task_id)  = 0;
    /////
    virtual bool onAllStatArrived(int stkey) = 0;
    virtual bool setSpeedLimit(int downloadSpeed, int uploadSpeed) {return true;};
                              
protected slots:
    //// from backend
    // virtual void onBackendFinished() = 0;

protected:
    uint64_t tid2gid(int tid);
    QString  tid2hex(int tid);
    
protected:
    // > 0 是从下载后端传递过来的事件
    // < 0 是从上端UI传递过来的事件
    // = 0 保留未用
    static QAtomicInt m_counter;
protected:
    QHash<int, void*> m_tasks; // tid => worker/QProcess/something
    QHash<void*, int> m_rtasks; // worker/QProcess/something => tid
    KBiHash<int, void*> m_tasks2; // tid => worker/QProcess/something

    // statqueue member
    QQueue<QPair<int, Aria2StatCollector*> > stkeys;
    QMutex stkLocker;
    Aria2StatCollector *lastStat;
    static QAtomicInt doneCounter; // 小于0的counter
};


#endif /* _ARIA2MANAGER_H_ */
