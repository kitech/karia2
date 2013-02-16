#ifndef _ARIA2MANAGER_H_
#define _ARIA2MANAGER_H_


#include <QtCore>


class TaskOption;

Aria2Manager : public QThread
{
    Q_OBJECT;
 public:
    Aria2Manager();
    virtual ~Aria2Manager();
    
    virtual void run();

 signals:
    void taskStatChanged(int tid, QMap<int, QVariant> stats);
    void taskFinished(int tid, int code);
    
    pubolic slots:
        int addTask(int task_id, const QString &url, TaskOption *to);
        int removeTask(int task_id);
        int startTask(int task_id);
        int stopTask(int task_id);
        int pauseTask(int task_id);

 protected slots:
    //// from backend
    void onBackendFinished();       

 protected:
    static QAtomInt m_counter;
};


#endif /* _ARIA2MANAGER_H_ */
