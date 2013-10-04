// aria2embedmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-25 21:55:29 +0000
// Version: $Id$
// 
#ifndef _ARIA2EMBEDMANAGER_H_
#define _ARIA2EMBEDMANAGER_H_

#include <QtCore>

// #include "SharedHandle.h"
#include "DownloadResult.h"

#include "aria2manager.h"

class TaskOption;
class Aria2EmbedWorker;
class Karia2StatCalc;
class Aria2StatCollector;

namespace aria2 {
    class MultiUrlRequestInfo;
}


class Aria2EmbedManager : public Aria2Manager
{
    Q_OBJECT;
public:
    Aria2EmbedManager();
    virtual ~Aria2EmbedManager();

    virtual void run();
    
public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to);
    // virtual int removeTask(int task_id);
    // virtual int startTask(int task_id);
    // virtual int stopTask(int task_id);
    virtual int pauseTask(int task_id);
    /////
    virtual bool onAllStatArrived(int stkey);

public slots: // from worker thread
    void onWorkerFinished();
/**
 * 实现下载状态信息的暂存
 * 实现下载状态状态的合并
 * 实现下载状态拆分发出
 * 实现三角通信的一个节点，另两个是GUI和aria2实例
 */
public:
    bool checkAndDispatchStat(Aria2StatCollector *sclt);
    bool checkAndDispatchServerStat(Aria2StatCollector *sclt);
    bool confirmBackendFinished(int tid, Aria2EmbedWorker *eaw);

private:
    // int _option_processing(aria2::Option& op, std::vector<std::string>& uris,
    //                    int argc, char* argv[]);
};

class Aria2EmbedWorker : public QThread
{
    Q_OBJECT;
public:
    Aria2EmbedWorker(QObject *parent = 0);
    virtual ~Aria2EmbedWorker();

    void run();

protected:
    friend class Aria2EmbedManager;

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


#endif /* _ARIA2EMBEDMANAGER_H_ */
