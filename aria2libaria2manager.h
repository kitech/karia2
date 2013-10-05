#ifndef _ARIA2LIBARIA2MANAGER_H_
#define _ARIA2LIBARIA2MANAGER_H_

#include <QtCore>

#include "aria2manager.h"
#include "aria2/aria2.h"

class TaskOption;
class Aria2Libaria2Worker;
class Karia2StatCalc;
class Aria2StatCollector;

namespace aria2 {
    class Session;
    class MultiUrlRequestInfo;
}

class Aria2Libaria2Manager : public Aria2Manager
{
    Q_OBJECT;
public:
    Aria2Libaria2Manager();
    virtual ~Aria2Libaria2Manager();

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
    bool confirmBackendFinished(int tid, Aria2Libaria2Worker *eaw);

private:
    aria2::Session *a2sess;
    aria2::SessionConfig a2cfg;
};

class Aria2Libaria2Worker : public QThread
{
    Q_OBJECT;
public:
    Aria2Libaria2Worker(QObject *parent = 0);
    virtual ~Aria2Libaria2Worker();

    void run();

protected:
    friend class Aria2Libaria2Manager;

    int m_tid;
    // std::shared_ptr<aria2::MultiUrlRequestInfo> muri;
    // std::vector<std::shared_ptr<aria2::RequestGroup> > requestGroups_;

    // std::shared_ptr<aria2::Option> option_;
    // std::unique_ptr<Karia2StatCalc> statCalc_;
    // aria2::DownloadEngine *e;
    int exit_status;

    aria2::KeyVals options;
    aria2::Session *a2sess; // point to manager member
};

#endif /* _ARIA2LIBARIA2MANAGER_H_ */
