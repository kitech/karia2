// emaria2c.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-10-23 06:54:26 -0700
// Version: $Id: 631840f7a796aed45a01e69179cd07f9274ec520 $
// 

#ifndef _EMARIA2C_H_
#define _EMARIA2C_H_

#include "common.h"

#include <vector>

// #include "SharedHandle.h"
#include "DownloadResult.h"

#include <QtCore>

class TaskOption;
class EAria2Worker;
class EAria2Man;
class Karia2StatCalc;
class Aria2StatCollector;

namespace aria2 {
    class MultiUrlRequestInfo;
}

namespace ng {
    namespace stat {
        enum {
            task_id=1, gid, total_length, completed_length, completed_percent, download_speed, upload_speed,
            bitfield, hex_bitfield, piece_length, num_pieces, num_connections, eta, str_eta,
            num_active, num_waiting, num_stopped,
            error_code, error_string, status, belongs_to
        };
    };
};

class EAria2Man : public QThread
{
    Q_OBJECT;
protected:
    static EAria2Man *m_instance;
    static QMutex m_inst_mutex;
    EAria2Man(QObject *parent = 0);
public:
    static EAria2Man *instance();
    virtual ~EAria2Man();

public slots:
    // int addTask(QString url);
    int addUri(int task_id, const QString &url, TaskOption *to);
    int pauseTask(int task_id);

    //// from backend
    void onWorkerFinished();

private:

signals:
//    void progressStat(int tid, quint32 gid, quint64 total_length,
//                   quint64 curr_length, quint32 down_speed, quint32 up_speed,
//                   quint32 num_conns, quint32 eta);
    void progressStat(Aria2StatCollector *stats);
    void taskFinished(int tid, int code);

/**
 * 实现下载状态信息的暂存
 * 实现下载状态状态的合并
 * 实现下载状态拆分发出
 * 实现三角通信的一个节点，另两个是GUI和aria2实例
 */
public:
    virtual void run();
    bool checkAndDispatchStat(Aria2StatCollector *sclt);
    bool checkAndDispatchServerStat(Aria2StatCollector *sclt);
    bool confirmBackendFinished(int tid, EAria2Worker *eaw);
public slots:
    bool onAllStatArrived(int stkey);

signals:
    void sessionStatFinished();
    void globalStatFinished();
    // -1表示没有修改
    // void taskStatChanged(int tid, int totalLengh = -1, int completedLength = -1, 
    //                      int completeDPercent = -1,
    //                      int downloadSpeed = -1, int uploadSpeed = -1);
    void taskStatChanged(int tid, QMap<int, QVariant> stats);
    void taskServerStatChanged(int tid, QList<QMap<QString, QString> > stats);
    void segmentStatFinished();

protected:
    QHash<int, EAria2Worker*> m_tasks; // tid => w
    QHash<EAria2Worker*, int> m_rtasks; // w => tid

    // statqueue member
    QQueue<QPair<int, Aria2StatCollector*> > stkeys;
    Aria2StatCollector *lastStat;
    static QAtomicInt doneCounter; // 小于0的counter
};

class EAria2Worker : public QThread
{
    Q_OBJECT;
public:
    EAria2Worker(QObject *parent = 0);
    virtual ~EAria2Worker();

    void run();

protected:
    friend class EAria2Man;
    std::vector<std::string> args;

    int m_tid;
    std::shared_ptr<aria2::MultiUrlRequestInfo> muri;
    std::vector<std::shared_ptr<aria2::RequestGroup> > requestGroups_;

    std::shared_ptr<aria2::Option> option_;
    // std::shared_ptr<aria2::StatCalc> statCalc_;
    // std::shared_ptr<aria2::OutputFile> summaryOut_;
    std::unique_ptr<Karia2StatCalc> statCalc_;
    aria2::DownloadEngine *e;
    int exit_status;

};

#endif /* _EMARIA2C_H_ */
