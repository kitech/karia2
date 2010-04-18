// taskqueue.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 22:14:45 +0800
// Version: $Id$
// 

#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include <string>
#include <vector>
#include <queue>

#include <QtCore>
#include <QThread>
#include <QMutex>

class TaskOption;
class TorrentPeerModel;
class TaskServerModel;
class SeedFileModel;

class Task  {
public:
    Task(int taskId, QString taskUrl);
    ~Task();

    int mTaskId;
    QString mTaskUrl;
    int mCatId;

    QVector<QPair<QString, QString> > mLogs;  // itime, log
public:
    TorrentPeerModel *btPeerModel;
    TaskServerModel *serverModel;
    SeedFileModel *seedFileModel;
};

//////////////////////////
class TaskQueue : public QObject
{
	Q_OBJECT;
public:
	//任务状态表 , 共有6个，一是ready ，也就是pause，可以启动，2是等运行状态，因为有最大运行任务的限制，3是运行状态，
	//4是错误状态，5是sheduled状态，6是完成状态 , 7 是删除状态（有可能还需要把它重新找回来）。

	enum {TS_UNKNOWN, TS_READY , TS_WAITING, TS_RUNNING , TS_ERROR , TS_COMPLETE , TS_DELETED } ;
	~TaskQueue() ;

	//取状态描述
	static QString getStatusString(int status) ;
	
	// for multi instance pattern
    static TaskQueue *instance(QObject *parent = 0);
	// static TaskQueue *instance ( int task_id , QObject *parent = 0 );
	bool removeInstance(int taskId);
    bool containsInstance(int taskId) ;

    bool addTaskModel(int taskId, TaskOption * option);
	static QString getFileNameByUrl(QString url );

    bool onStartTask(int pTaskId);
    void onPauseTask(int pTaskId );
	
	// more from nullget.h

    // void setUrl(QString url);
    TorrentPeerModel *torrentPeerModel(int taskId);
    TaskServerModel *taskServerModel(int taskId);
    bool isTorrentTask(int taskId);
    bool isMagnetTask(int taskId);
    bool isMetalinkTask(int taskId);
    bool setPeers(int taskId, QVariantList &peers);

public slots:
	void onOneSegmentFinished(int taskId, int segId , int finishStatus ) ;
	
	void onFirstSegmentReady(int pTaskId , long totalLength, bool supportBrokenRetrive);
	// void onAbtainedFileLength( int pTaskId , long totalLength , bool supportBrokenRetrive);
	// void onFirstSegmentFaild( int taskId, int errorNo );
	void onLogSegment( int taskId , int segId , QString log , int type ) ; 

	//void onTaskDone(int pTaskId);	//

    void onTaskStatusNeedUpdate(int taskId, QVariantMap &sts);
	void onTaskListCellNeedChange(int taskId , int cellId , QString value  );
	//void onSegmentGotLengthNeedUpdate ( int taskId , int segId , long delta , QString opt );
	void onSegmentGotLengthNeedUpdate ( int taskId , int segId , long delta , int optType );
	void onSegmentCellNeedChange( int taskId , int segId ,  int cellId , QString value ) ;

	void onStartSegment(int pTaskId,int pSegId);
	void onPauseSegment(int pTaskId,int pSegId);

	void onMemoryOverLoad();

	// void setTaskId(int task_id);
	int getMaxSegCount(int pTaskId,int cat_id);
	bool getFileAbtained( int pTaskId , int cat_id);
	QString getRealUrlModel(int pTaskId,int cat_id);
	int getActiveSegCount(int pTaskId, int cat_id);

    QBitArray getCompletionBitArray(int taskId);

    void onTaskLogArrived(QString cuid, QString itime, QString log);

	//test 
	void deleteLater () ;

signals:	
	//与主窗口类通信用的信号。调度该任务的执行。
	void onTaskDone(int pTaskId);
	void taskCompletionChanged(int pTaskId, int pCompletion, QString pFileName);
	void taskCompletionChanged(TaskQueue *pTask, bool pSwitch);

private:
	void initDefaultMember();
	// for multi instance pattern
	TaskQueue(QObject *parent = 0);
	// QString getBestTaskUrl();

    QBitArray fromHexBitString(QString fields);
    QString  fromBitArray(QBitArray ba);

private:
	
	int mCatID ;
	char mSwapBuff[1024] ;

    static TaskQueue *mInstance;
    QHash<int, Task*> mTasks;
    
    QHash<QString, int> mTaskCUIDs; // cuid, taskId
    QHash<QString, QVector<QPair<QString, QString> > > mUnattachedLogs; // cuid, <itime, log>
};


#endif


