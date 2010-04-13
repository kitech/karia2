// task.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-09 12:19:01 +0800
// Version: $Id$
// 

#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QtCore>
#include <cassert>
#include <string>
#include <vector>
#include <queue>

#include "taskinfodlg.h"	//for TaskOption class 

#include "fileretriver.h"
#include "ftpretriver.h"
#include "httpsretriver.h"
#include "mediaretriver.h"
#include "mmsretriver.h"
#include "rtspretriver.h"
#include "simplemmsretriver.h"

#include "sqlitetaskmodel.h"
#include "sqlitesegmentmodel.h"
#include "segmentlogmodel.h"
#include "taskballmapwidget.h"

/**
 * 表示一个下载任务对象
 * 
 */
class Task : public QObject
{
	Q_OBJECT;
public:
	Task(QObject *parent = 0);
	~Task();

	//任务状态表 , 共有5个，
	//1是ready ，也就是paused，可以启动，处于等待运行状态，(注意因为有最大运行任务的限制)
	//2是运行状态，
	//3是错误状态，4是完成状态 ,5 是删除状态（有可能还需要把它重新找回来）。
	//下载任务状态图
	enum { TS_READY , TS_RUNNING , TS_ERROR , TS_DONE , TS_DELETED } ;

	//取状态描述,工具函数
	static QString getStatusString(int status) ;
	static QString getStatusDescription( int status ) ;

	//根据下载任务的URL猜测出下载任务存储文件名。
	static QString getFileNameByUrl(QString url );

public :	//下载任务的一些状态值的存储变量
//private: 
	int mTaskId;
	QString mTaskUrl ;
	long mTotalLength ;
	long mGotLength ;

	int mTotalSegmentCount ;		
	int mActiveSegmentCount;	//活动的线程数，正在下载数据的。
	double mEclapsedTime ;	//eclapsed time 
	double mLastEclapsedTime ;	//eclapsed time , prev time 
	double mAverageSpeed ;	// 平均速度
	double mInstantSpeed ;	//瞬时速度
	QDateTime mCreateTime ;	//create time of this task
	QDateTime mCompleteTime ;	//finish time 
	QDateTime mStartTime ;	//start time of this task

	//	块ID
	QMap<quint64 , BaseRetriver* > mSegmentThread ;

	//QString mHumanAverageSpeed ;	// 与mAverageSpeed对应，即时计算出来吧，不在存储了。
	//QString mHumanmEclapsedTime ;
	
	int mMaxBandWidth ;	//最大速率限制值	bytes
	
	//TaskOption * mTaskOption ;	//该任务的任务参数，包括所有从new dowload对话框里来的参数。

	int  mTaskStatus ;	//任务的当前状态，其值为上面枚举值之一。

	//bool mFileLengthAbtained ;	//是否已经取到了文件大小。否则就不能用多线程下载。
	//bool mBreakenResume ;	//是否支持断点续传。	//临时还没有什么用。
	//bool mMultiConnectionSupported ;	//	 是否支持多线程下载。
	//bool canceled ; //是否被暂停了


public slots:
	int onRedirectLocation(int pTaskId,int pSegId,QString redirectLocation,int redirectTimes);	//handle redirect case
	//void onOneSegmentFinished(int taskId, int segId , int finishStatus ) ;
	void onOneSegmentFinished(  );
	
	void onFirstSegmentReady(int pTaskId , long totalLength, bool supportBrokenRetrive);
	void onAbtainedFileLength( int pTaskId , long totalLength , bool supportBrokenRetrive);
	//不再使用了
	//void onFirstSegmentFaild( int taskId, int errorNo );

	void onLogSegment( int taskId , int segId , QString log , int type ) ; 
	
	void onTaskListCellNeedChange(int taskId , int cellId , QString value  );
	void onSegmentGotLengthNeedUpdate ( int taskId , int segId , long delta , int optType );
	void onSegmentCellNeedChange( int taskId , int segId ,  int cellId , QString value ) ;

	void onStartSegment(int pTaskId,int pSegId);
	void onPauseSegment(int pTaskId,int pSegId);

	void onMemoryOverLoad();

	int getMaxSegCount(int pTaskId,int cat_id);
	bool getFileAbtained( int pTaskId , int cat_id);
	QString getRealUrlModel(int pTaskId,int cat_id);
	int getActiveSegCount(int pTaskId, int cat_id);

	//该下载任务的分块状态表。
	QList<QPair<long , long> >  getCompletionList() ;

signals:	
	//与主窗口类通信用的信号。调度该任务的执行。
	void taskCompletionChanged( int pTaskId , int pCompletion , QString pFileName  ) ;
	void taskCompletionChanged( Task * pTask , bool pSwitch  ) ;
	void taskDone(int pTaskId);	//

private:
	void initDefaultMember();
	
	QString getBestTaskUrl();
	BaseRetriver * getRetriverByUrlScheme(QString strurl);
	bool connectRetriverSignals(BaseRetriver *retr);

	//RTSP 结束处理，合并文件工作函数
	bool RTSPSpecialHandler( int a_int,void * a_void_pointer , Task * a_Task_pointer ); 
	bool MMSSpecialHandler( int a_int,void * a_void_pointer , Task * a_Task_pointer );



private:
	
	int mCatID ;
	char mSwapBuff[1024] ;

	QMutex mSegMapMutex ;

	QMutex  mAtomMutex ;	//用于查询任务

	bool mNeedSpecialHandling ;	//对于像rtsp，可能需要合并文件操作
	//特殊处理函数指针，参数没有实际意义，只是测试类成员函数指针罢了
	bool (Task::*mSpecialHander)(int,void*,Task*) ;

	typedef std::pair<std::string,int >  QueueItem  ;
	std::priority_queue<
		QueueItem,
		std::vector< QueueItem >		
	> mMirrorsQueue ;		//镜像URL优先队列结构
	
};

#endif // TASK_H
