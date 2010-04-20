// taskqueue.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 22:14:39 +0800
// Version: $Id$
// 

#include <cassert>

#include "taskinfodlg.h"	//for TaskOption class 

#include "taskqueue.h"
#include "sqlitetaskmodel.h"
#include "sqlitesegmentmodel.h"
#include "segmentlogmodel.h"
#include "taskballmapwidget.h"

#include "torrentpeermodel.h"
#include "taskservermodel.h"

Task::Task(int taskId, QString taskUrl)
{
    this->mTaskId = taskId; 
    this->mTaskUrl = taskUrl;
    this->btPeerModel = new TorrentPeerModel();
    this->serverModel = new TaskServerModel();
    this->seedFileModel = NULL;
}
Task::~Task()
{
}

/**
   用于管理一组任务，各种状态的任务都在这其中管理维护

   去掉这些静态函数，太多了。
*/

// static
TaskQueue *TaskQueue::mInstance = NULL;
// static 
TaskQueue *TaskQueue::instance(QObject *parent)
{
    TaskQueue *ins = TaskQueue::mInstance;
    if (ins == NULL) {
        ins = TaskQueue::mInstance = new TaskQueue();
    }
    return ins;
}

bool TaskQueue::removeInstance( int taskId )
{	
    if (this->mTasks.contains(taskId)) {
        this->mTasks.remove(taskId);
    } else {
        assert(1 == 2);
    }
	return false;
}

bool TaskQueue::containsInstance(int taskId) 
{
    return this->mTasks.contains(taskId);
}

TaskQueue::TaskQueue( QObject *parent )
    : QObject(parent)
{
	//assert( option != NULL );
    // this->btPeerModel = NULL;

	this->initDefaultMember();

	this->setObjectName("Class TaskQueue");
}

void TaskQueue::initDefaultMember()
{
	// this->mCreateTime = QDateTime::currentDateTime();
	// this->mStartTime = QDateTime::currentDateTime();
	// this->mEclapsedTime = 0.0 ;
	// this->mLastEclapsedTime = 0.0;	

	// this->mTotalLength = 0 ;
	// this->mGotLength  = 0 ;

	// this->canceled = false ;

}

TaskQueue::~TaskQueue() 
{

}

// void TaskQueue::setParameter( int pTaskId , QString pTaskUrl,long pTotalLength , long pGotLength , 
// 		int pTotalSegmentCount )
// {
// 	this->mTaskId = pTaskId ;
// 	this->mTaskUrl = pTaskUrl ;
// 	this->mTotalLength = pTotalLength ;
// 	this->mGotLength  = pGotLength ;
// 	this->mTotalSegmentCount = pTotalSegmentCount ;
// }

//取状态描述
QString TaskQueue::getStatusString(int status) 
{
	QString ss;	//enum { TS_READY , ............
	switch (status)
	{
	case TaskQueue::TS_READY:
		ss = "ready";
		break ;
	case TaskQueue::TS_WAITING:
		ss = "waiting";
		break;
	case TaskQueue::TS_RUNNING:
		ss = "running";
		break ;
	case TaskQueue::TS_ERROR:
		ss = "error";
		break;
	case TaskQueue::TS_COMPLETE:
		ss = "complete";
		break;
	case TaskQueue::TS_DELETED:
		ss = "deleted";
		break ;
	default: 
		ss = "unknown status";
		break ;
	}
	return ss ;
}

bool TaskQueue::addTaskModel(int taskId , TaskOption *option)
{
	assert(option != 0);
    Task *task = NULL;
    if (!this->mTasks.contains(taskId)) {
        task = new Task(taskId, option->mTaskUrl);
        this->mTasks[taskId] = task;
    } else {
        task = this->mTasks[taskId];
    }

    qDebug()<<__FUNCTION__<<option->mCatId;
	//将任务信息添加到 task list view 中
	QModelIndex index;
	// QAbstractItemModel * mdl = SqliteTaskModel::instance(ng::cats::downloading, 0);
    QAbstractItemModel *mdl = SqliteTaskModel::instance(option->mCatId, 0);
    QModelIndexList mil = mdl->match(mdl->index(0, ng::tasks::task_id), Qt::DisplayRole, 
                                     QString("%1").arg(taskId), 1, Qt::MatchExactly | Qt::MatchWrap);
    if (mil.count() == 0) {
        int modelRows = mdl->rowCount();
        mdl->insertRows(modelRows, 1);
        index = mdl->index(modelRows, ng::tasks::task_id);
        mdl->setData(index, taskId);
        index = mdl->index(modelRows, ng::tasks::task_status);
        mdl->setData(index, "ready");
        index = mdl->index(modelRows, ng::tasks::save_path);
        mdl->setData(index, option->mSavePath);
        index = mdl->index(modelRows, ng::tasks::file_name);
        mdl->setData(index, TaskQueue::getFileNameByUrl(option->mTaskUrl));
        index = mdl->index(modelRows, ng::tasks::block_activity);
        mdl->setData(index, QString("0/0") );
        index = mdl->index(modelRows, ng::tasks::active_block_count);
        mdl->setData(index, QString("0"));
        index = mdl->index(modelRows, ng::tasks::split_count);
        mdl->setData(index, QString("%1").arg(option->mSplitCount));
        index = mdl->index(modelRows, ng::tasks::real_url);
        mdl->setData(index, option->mTaskUrl);	
        index = mdl->index(modelRows, ng::tasks::org_url);
        mdl->setData(index, option->mTaskUrl);		
        index = mdl->index(modelRows, ng::tasks::cat_id );
        // mdl->setData(index ,ng::cats::downloading);
        mdl->setData(index, option->mCatId);
        index = mdl->index(modelRows, ng::tasks::create_time );
        mdl->setData(index, QDateTime::currentDateTime().toString("hh:mm:ss yyyy-MM-dd"));
        index = mdl->index(modelRows, ng::tasks::file_length_abtained);
        mdl->setData(index, QString("false"));
    } else {
        // model exists, resume task
    }
    // TaskQueue *tq = TaskQueue::instance(task_id);
    // tq->setUrl(option->mTaskUrl);


	return true;
}

//static 
QString TaskQueue::getFileNameByUrl(QString url)
{
	QString fname ;
	QFileInfo fi(url);
	if (fi.fileName().length() == 0) {
		fname = "index.html";
	} else {
		fname = fi.fileName();
	}	
	return fname;
}

// void TaskQueue::setUrl(QString url)
// {
//     this->mTaskUrl = url;
//     if (this->mTaskUrl.endsWith(".torrent")) {
//         if (this->btPeerModel == NULL) {
//             this->btPeerModel = new TorrentPeerModel();
//         }
//     }
// }
TorrentPeerModel *TaskQueue::torrentPeerModel(int taskId)
{
    TorrentPeerModel *pm = NULL;
    if (this->mTasks.contains(taskId)) {
        pm = this->mTasks[taskId]->btPeerModel;
    }
    return pm;
}

TaskServerModel *TaskQueue::taskServerModel(int taskId)
{
    TaskServerModel *sm = NULL;

    if (this->mTasks.contains(taskId)) {
        sm = this->mTasks[taskId]->serverModel;
    }
    return sm;
}


bool TaskQueue::isTorrentTask(int taskId)
{
    // if (this->mTaskUrl.endsWith(".torrent")) {
    //     return true;
    // }
    if (this->mTasks.contains(taskId)) {
        if (this->mTasks[taskId]->mTaskUrl.endsWith(".torrent")) {
            return true;
        }
    }
    return false;
}

bool TaskQueue::isMagnetTask(int taskId) 
{
    // if (this->mTaskUrl.startsWith("magnet:?")) {
    //     return true;
    // }
    if (this->mTasks.contains(taskId)) {
        if (this->mTasks[taskId]->mTaskUrl.startsWith("magnet:?")) {
            return true;
        }
    }
    return false;
}

bool TaskQueue::isMetalinkTask(int taskId)
{
    // if (this->mTaskUrl.endsWith(".metalink")) {
    //     return true;
    // }
    if (this->mTasks.contains(taskId)) {
        if (this->mTasks[taskId]->mTaskUrl.endsWith(".metalink")) {
            return true;
        }
    }
    return false;
}

bool TaskQueue::setPeers(int taskId, QVariantList &peers)
{
    // bool rv = this->btPeerModel->setData(peers);
    // return rv;
    if (this->mTasks.contains(taskId)) {
        this->mTasks[taskId]->btPeerModel->setData(peers);
    }
    return true;
}

void TaskQueue::onOneSegmentFinished(int taskId, int segId , int finishStatus ) 
{
	qDebug() << __FUNCTION__<<taskId<< " "<< segId  ;

	long startOffset ;
	long totalLength ;
	long gotLength ;
	long totalEmitLength ;
	int  segCount ;
	long splitLength , splitOffset , splitLeftLength ;
	TaskQueue * tq = 0 ;
	// BaseRetriver * br = 0 , * brn = 0 , * nhs = 0 ;
	bool doneTask = false ;	//標識是否执行了完成任务的移动工作。
	SqliteSegmentModel * mdl = SqliteSegmentModel::instance(taskId,this);


	
}
	
void TaskQueue::onFirstSegmentReady(int pTaskId , long totalLength, bool supportBrokenRetrive)
{
	qDebug() << __FUNCTION__ ;

	qDebug()<<  __FUNCTION__ <<" return for nothing done " ;
	return ;
	//
	TaskQueue * tq = 0 ;
	//tq =  this->findTaskById(pTaskId );
	tq = this ;
	
	//cacl per len and start offset 
	long perLen;// = totalLength/tq->mTotalSegmentCount ;
	long startOffset = perLen ;

	QModelIndex index ;
	int modelRows ;

	if (tq == 0) {
		qDebug()<< "find task faild : "<<pTaskId ;
		// tq->mTaskStatus = TaskQueue::TS_ERROR;
		this->onTaskListCellNeedChange( pTaskId , 1 , tq->getStatusString(TaskQueue::TS_ERROR));
		return;
	}

	QUrl url;// (tq->mTaskUrl);

	//update task list view model cell
	this->onTaskListCellNeedChange( pTaskId , ng::tasks::file_size , QString("%1").arg( totalLength ));
	// tq->mTotalLength = totalLength ;
	//通知画任务状态图。先去掉了，看什么时候能加进去。
	//TaskBallMapWidget::instance()->onRunTaskCompleteState( tq );

	//set the first segment totalLength element
	// br = tq->mSegmentThread[0];

	//br->mTotalLength = perLen ;
	QAbstractItemModel * mdl = 0 ;
}
/**
 * 除了修改这个控制参数外还应该做点什么呢。
 * 这个可以由子线程发出多次，但如果第二次发应该是错误的，这并没有关系，我们只在这里判断就行了。
 */
// void TaskQueue::onAbtainedFileLength( int pTaskId , long totalLength , bool supportBrokenRetrive)
// {
// 	qDebug() << __FUNCTION__ << "taskId: "<< pTaskId << "totalLength :" << totalLength ;
// 	//if( this->mFileLengthAbtained == false )
// 	if( this->getFileAbtained(pTaskId,ng::cats::downloading) == false )
// 	{
// 		//this->mFileLengthAbtained = true ;	//这个参数非常重要
// 		this->mTotalLength = totalLength ;
// 		this->onTaskListCellNeedChange( pTaskId , (int)ng::tasks::file_length_abtained , QString("true") );
// 		this->onTaskListCellNeedChange( pTaskId , (int)ng::tasks::file_size , QString("%1").arg(totalLength) );
// 		//emit this->attemperTaskStatus(this ,(int) TaskQueue::TS_RUNNING );
// 		//SqliteSegmentModel::instance(pTaskId,this)->setCapacity(totalLength );
// 		this->onStartTask(pTaskId);
// 	}
// 	else
// 	{
// 		assert( 1== 2 );
// 	}
// }

// void TaskQueue::onFirstSegmentFaild( int taskId, int errorNo )
// {
// 	qDebug() << __FUNCTION__ << "taskId: "<< taskId << "errorNo :" << errorNo ;
// 	TaskQueue * tq = 0 ;
// 	//tq = this->findTaskById(taskId);
// 	tq = this ;
// 	if( tq == 0 ) 
// 	{
// 		return ;
// 	}
// 	tq->mTaskStatus = TaskQueue::TS_ERROR;
// 	this->onTaskListCellNeedChange( taskId , ng::tasks::task_status , tq->getStatusString(TaskQueue::TS_ERROR));

// }

void TaskQueue::onStartSegment(int pTaskId,int pSegId)
{
	qDebug()<<__FUNCTION__<<pTaskId<<pSegId;
	int row =0 ;
	// BaseRetriver *br = 0 ;
	QAbstractItemModel * logmdl = 0 ;
	SqliteSegmentModel * segmdl = 0 ;

	logmdl = SegmentLogModel::instance( pTaskId , pSegId , this );
	segmdl = SqliteSegmentModel::instance(pTaskId , this);
}

void TaskQueue::onPauseSegment(int pTaskId,int pSegId)
{

}

void TaskQueue::onLogSegment(int taskId, int segId, QString log, int type) 
{
	//qDebug() << __FUNCTION__ << taskId <<" "<< segId ;

	int nowRows ;
	QModelIndex index ;
	QAbstractItemModel * mdl = 0 ;	
	
	mdl = SegmentLogModel::instance(taskId, segId ,this);

	nowRows = mdl->rowCount();
	mdl->insertRows(nowRows,1);
	index = mdl->index(nowRows,ng::logs::log_type);
	mdl->setData(index,type);
	index = mdl->index(nowRows,ng::logs::add_time);
	mdl->setData(index,
		QTime::currentTime().toString("hh:mm:ss ") + QDate::currentDate().toString("yyyy-MM-dd") );
	index = mdl->index(nowRows,ng::logs::log_content);
	mdl->setData(index,log);
}

void TaskQueue::onTaskStatusNeedUpdate(int taskId, QVariantMap &sts)
{
	//qDebug() << __FUNCTION__ ;

	QModelIndex idx , idx2,idx3;
	quint64 fsize , abtained ;
	double  percent ;
    // bool found = false;
    unsigned short bitfield = 0;
    int numPieces = 0;
    QStringList bitList;
    QString bitString;

	//maybe should use mCatID , but we cant know the value here , so use default downloading cat
	QAbstractItemModel *mdl = SqliteTaskModel::instance(ng::cats::downloading, this);

	//QDateTime bTime , eTime ; 
	//bTime = QDateTime::currentDateTime();
    QModelIndexList mil = mdl->match(mdl->index(0, ng::tasks::task_id), Qt::DisplayRole,
                                     QVariant(QString("%1").arg(taskId)), 1, Qt::MatchExactly | Qt::MatchWrap);

    if (mil.count() == 1) {
        idx = mil.at(0);
        if (sts["status"].toString() == "complete") {
            mdl->setData(mdl->index(idx.row(), ng::tasks::finish_time), QDateTime::currentDateTime().toString());

            mdl->setData(mdl->index(idx.row(), ng::tasks::abtained_length), 
                         mdl->data(mdl->index(idx.row(), ng::tasks::file_size)));
            mdl->setData(mdl->index(idx.row(), ng::tasks::task_status), sts["status"]);
            mdl->setData(mdl->index(idx.row(), ng::tasks::left_length), 
                         (sts["totalLength"].toLongLong() - sts["completedLength"].toLongLong()));
            mdl->setData(mdl->index(idx.row(), ng::tasks::abtained_percent), QString("%1 %").arg(100));
        } else {
            mdl->setData(mdl->index(idx.row(), ng::tasks::file_size), sts["totalLength"]);
            mdl->setData(mdl->index(idx.row(), ng::tasks::current_speed), sts["downloadSpeed"]);
            mdl->setData(mdl->index(idx.row(), ng::tasks::abtained_length), sts["completedLength"]);
            mdl->setData(mdl->index(idx.row(), ng::tasks::task_status), sts["status"]);
            mdl->setData(mdl->index(idx.row(), ng::tasks::active_block_count), sts["connections"]);
            mdl->setData(mdl->index(idx.row(), ng::tasks::left_length), 
                         (sts["totalLength"].toLongLong() - sts["completedLength"].toLongLong()));
            mdl->setData(mdl->index(idx.row(), ng::tasks::total_block_count), sts["numPieces"]);
            mdl->setData(mdl->index(idx.row(), ng::tasks::block_activity), 
                         QString("%1/%2").arg(sts["connections"].toString()).arg(sts["numPieces"].toString()));
            mdl->setData(mdl->index(idx.row(), ng::tasks::abtained_percent)
                         , QString("%1 %").arg((sts["totalLength"].toLongLong() == 0) ? 0 
                                               : (sts["completedLength"].toLongLong()*100.0 / sts["totalLength"].toLongLong()*1.0)));
        }
            
        // 计算完成的pieces
        if (sts["status"].toString() == "complete") {
            int blockCount = mdl->data(mdl->index(idx.row(), ng::tasks::total_block_count)).toInt();
            QStringList bitList;
            for (int i = 0 ; i < blockCount; i ++) bitList << "1";
            mdl->setData(mdl->index(idx.row(), ng::tasks::total_packet), bitList.join(","));
                
        } else {
            mdl->setData(mdl->index(idx.row(), ng::tasks::total_packet), 
                         this->fromBitArray(this->fromHexBitString(sts["bitfield"].toString())));
        }

        // 处理bt信息
        if (sts.contains("bittorrent")) {
            QVariantMap btSts = sts["bittorrent"].toMap();
            qDebug()<<"announceList:"<<btSts["announceList"]
                    <<"comment:"<<btSts["comment"]
                    <<"createDate:"<<btSts["createDate"]
                    <<"mode:"<<btSts["mode"]
                    <<"info:"<<btSts["info"];
        }
    } else {
        qDebug()<<__FUNCTION__<<" cant found update model";
    }

	// int taskCount = mdl->rowCount();
	// for (int i = 0 ; i < taskCount; i ++) {
	// 	//find the cell index
	// 	idx = mdl->index(i, ng::tasks::task_id );

	// 	if (idx.data().toInt() == taskId) {
    //         // qDebug()<<"found index of cell";
            
    //         if (sts["status"].toString() == "complete") {
    //             mdl->setData(mdl->index(i, ng::tasks::finish_time), QDateTime::currentDateTime().toString());

    //             mdl->setData(mdl->index(i, ng::tasks::abtained_length), mdl->data(mdl->index(i, ng::tasks::file_size)));
    //             mdl->setData(mdl->index(i, ng::tasks::task_status), sts["status"]);
    //             mdl->setData(mdl->index(i, ng::tasks::left_length), 
    //                          (sts["totalLength"].toLongLong() - sts["completedLength"].toLongLong()));
    //             mdl->setData(mdl->index(i, ng::tasks::abtained_percent), QString("%1 %").arg(100));
    //         } else {
    //             mdl->setData(mdl->index(i, ng::tasks::file_size), sts["totalLength"]);
    //             mdl->setData(mdl->index(i, ng::tasks::current_speed), sts["downloadSpeed"]);
    //             mdl->setData(mdl->index(i, ng::tasks::abtained_length), sts["completedLength"]);
    //             mdl->setData(mdl->index(i, ng::tasks::task_status), sts["status"]);
    //             mdl->setData(mdl->index(i, ng::tasks::active_block_count), sts["connections"]);
    //             mdl->setData(mdl->index(i, ng::tasks::left_length), 
    //                          (sts["totalLength"].toLongLong() - sts["completedLength"].toLongLong()));
    //             mdl->setData(mdl->index(i, ng::tasks::total_block_count), sts["numPieces"]);
    //             mdl->setData(mdl->index(i, ng::tasks::block_activity), 
    //                          QString("%1/%2").arg(sts["connections"].toString()).arg(sts["numPieces"].toString()));
    //             mdl->setData(mdl->index(i, ng::tasks::abtained_percent)
    //                          , QString("%1 %").arg((sts["totalLength"].toLongLong() == 0) ? 0 
    //                                              : (sts["completedLength"].toLongLong()*100.0 / sts["totalLength"].toLongLong()*1.0)));
    //         }
            
    //         // 计算完成的pieces
    //         if (sts["status"].toString() == "complete") {
    //             int blockCount = mdl->data(mdl->index(i, ng::tasks::total_block_count)).toInt();
    //             QStringList bitList;
    //             for (int i = 0 ; i < blockCount; i ++) bitList << "1";
    //             mdl->setData(mdl->index(i, ng::tasks::total_packet), bitList.join(","));
                
    //         } else {
    //             mdl->setData(mdl->index(i, ng::tasks::total_packet), 
    //                          this->fromBitArray(this->fromHexBitString(sts["bitfield"].toString())));
    //         }

    //         // 处理bt信息
    //         if (sts.contains("bittorrent")) {
    //             QVariantMap btSts = sts["bittorrent"].toMap();
    //             qDebug()<<"announceList:"<<btSts["announceList"]
    //                     <<"comment:"<<btSts["comment"]
    //                     <<"createDate:"<<btSts["createDate"]
    //                     <<"mode:"<<btSts["mode"]
    //                     <<"info:"<<btSts["info"];
    //         }
            
    //         found = true;
	// 		break;
	// 	}
	// }
    // qDebug()<<"found status:"<<found;
}

QBitArray TaskQueue::fromHexBitString(QString fields)
{
    QBitArray ba;
    int hLen = fields.length();
    unsigned char ch = 0;

    ba.resize(hLen * 4);
    for (int i = 0; i < hLen; i ++) {
        ch = QString(fields.at(i)).toUShort(NULL, 16);
        for (int j =  3 ; j >= 0; j --) {
            ba.setBit(i * 4 + 3 - j, (ch >> j) & 0x01);
        }
    }
    
    // qDebug()<<fields<<ba<<ba.size();
    return ba;
}

QString TaskQueue::fromBitArray(QBitArray ba)
{
    QStringList bitList;
    for (int i = 0 ; i < ba.size(); i ++) {
        bitList << (ba.testBit(i) ? "1" : "0");
    }
    return bitList.join(",");
}

void TaskQueue::onTaskListCellNeedChange(int taskId, int cellId, QString value)
{
	//qDebug() << __FUNCTION__ ;

	QModelIndex idx, idx2, idx3;
	quint64 fsize, abtained;
	double  percent;
    // bool found = false;
    int row;

	//maybe should use mCatID , but we cant know the value here , so use default downloading cat
	QAbstractItemModel *mdl = SqliteTaskModel::instance(ng::cats::downloading, this);

    QModelIndexList mil = mdl->match(mdl->index(0, ng::tasks::task_id), Qt::DisplayRole, 
                                     QString("%1").arg(taskId), 1, Qt::MatchExactly | Qt::MatchWrap);

    // qDebug()<<"match found cell change:"<<mil<<taskId<<value;
    if (mil.count() == 1) {
        row = mil.at(0).row();
        idx = mdl->index(row, cellId);
        //change the value 
        qDebug()<<mdl->data(idx);
        mdl->setData(idx, value);
        if(cellId == ng::tasks::abtained_length) {
            idx = mdl->index(row, ng::tasks::file_size);
            idx2 = mdl->index(row, ng::tasks::left_length) ;
            idx3 = mdl->index(row, ng::tasks::abtained_percent) ;
            abtained = value.toULongLong() ;
            fsize = mdl->data(idx).toULongLong() ;
            percent = 100.0*abtained/ fsize ;
            //change the value 
            mdl->setData(idx2, fsize - abtained);
            mdl->setData(idx3, percent);

            //通知DropZone
            emit this->taskCompletionChanged(taskId, (int)percent, 
                                             this->getRealUrlModel(taskId, ng::cats::downloading));
            emit this->taskCompletionChanged(this, false) ;
        }
    }

}

//void TaskQueue::onSegmentGotLengthNeedUpdate ( int taskId , int segId , long delta , QString opt )
void TaskQueue::onSegmentGotLengthNeedUpdate ( int taskId , int segId , long delta , int optType )
{
	//qDebug() << __FUNCTION__ << delta << optType ;
	TaskQueue * tq = 0 ;
	SqliteSegmentModel * mdl = 0 ;
	
	QDateTime currTime = QDateTime::currentDateTime();

	//mAtomMutex.lock();

	int nowRows ;
	QModelIndex index ;	
	
	int segCount ;
	// QVector<BaseRetriver*> sq ;
	// BaseRetriver* psq ;
	quint64 task_abtained = 0 ;
	
	tq = this ;	


	return;	
}
/**
 * 用于修改线程模型的数据。
 */
void TaskQueue::onSegmentCellNeedChange( int taskId , int segId ,  int cellId , QString value ) 
{
	//qDebug() << __FUNCTION__ ;

	QModelIndex idx , segidx ;

	//maybe should use mCatID , but we cant know the value here , so use default downloading cat
	QAbstractItemModel * mdl = SqliteSegmentModel::instance( taskId ,  this) ;

	int taskCount = mdl->rowCount();
	for( int i = 0 ; i < taskCount ; i ++ )
	{	
		//find the cell index
		idx = mdl->index(i,ng::segs::task_id );
		segidx = mdl->index(i,ng::segs::seg_id );

		if( idx.data().toInt() == (taskId) && segidx.data().toInt() == segId  )
		{			
		//	qDebug()<<"found index of cell" ;

			idx = mdl->index(i,cellId);
			//change the value 
			mdl->setData(idx,value);

			break;
		}
	}
}

//void TaskQueue::onTaskDone(int pTaskId)	//
//{
//	qDebug() << __FUNCTION__ ;
//
//}

	// more from nullget.h

void TaskQueue::onMemoryOverLoad()
{
	qDebug()<<__FUNCTION__<<__LINE__ ;
	// emit this->onTaskDone( this->mTaskId );
}

bool TaskQueue::onStartTask(int pTaskId) 
{
	////假设任务的运行句柄即TaskQueue实例不存在,创建任务实例，并启动它。
	//assert( TaskQueue::containsInstance(pTaskId) == false ) ;
	
	if( TaskQueue::containsInstance(pTaskId) == false ) 
	{
		TaskQueue * taskQueue = NULL;//TaskQueue::instance(pTaskId,0);
		// if (taskQueue->canceled == true) {
		// 	qDebug()<<__FUNCTION__<<__LINE__<<" task caceled";
		// 	return false;
		// }
		if ( taskQueue->getFileAbtained(pTaskId,ng::cats::downloading) ) {
			int segCount = taskQueue->getMaxSegCount(pTaskId,ng::cats::downloading) ;
			for (int i = 0 ; i < segCount ; i ++ ) {
				taskQueue->onStartSegment(pTaskId, i);
			}
		} else {
			taskQueue->onStartSegment(pTaskId,0);
		}
    } else {
		TaskQueue * taskQueue = NULL;// TaskQueue::instance(pTaskId,0);
		// if( taskQueue->canceled == true ) 
		// {
		// 	qDebug()<<__FUNCTION__<<__LINE__<<" task caceled";
		// 	return false ;
		// }		
		// if ( taskQueue->getFileAbtained(pTaskId,ng::cats::downloading) )
		// {
		// 	int segCount = taskQueue->getMaxSegCount(pTaskId,ng::cats::downloading) ;
		// 	for( int i = 0 ; i < segCount ; i ++ )
		// 	{
		// 		taskQueue->onStartSegment(pTaskId,i);
		// 	}
		// }
		// else
		// {
		// 	taskQueue->onStartSegment(pTaskId,0);
		// }
	}

	return false ;
}

void TaskQueue::onPauseTask(int pTaskId ) 
{
	if (this->mTasks.contains(pTaskId)) {
		// assert(1 == 2);		
        Task *task = this->mTasks[pTaskId];
        this->mTasks.remove(pTaskId);
	} else {
		// TaskQueue * taskQueue = TaskQueue::instance(pTaskId,0);
		// taskQueue->canceled = true ;

		// TaskQueue::removeInstance(pTaskId);
		//delete taskQueue;taskQueue = 0 ;		
		//delete 操作在　onOneSegmentFinished　成员函数中
	}	
}


int TaskQueue::getMaxSegCount(int pTaskId,int cat_id)
{
	int msc = -1 ;
	SqliteTaskModel * tm = SqliteTaskModel::instance(cat_id,0);
	QModelIndex mix ;

	for( int i = 0 ; i < tm->rowCount() ; i ++ )
	{
		mix = tm->index(i,ng::tasks::task_id);
		int currTaskId = tm->data(mix).toInt();
		if( currTaskId == pTaskId )
		{
			msc = tm->data(tm->index(i,ng::tasks::total_block_count)).toInt() ;

			break ;
		}
	}

	assert( msc > 0 );
	return msc ;
}

bool TaskQueue::getFileAbtained( int pTaskId , int cat_id)
{
	bool msc = false ;
	SqliteTaskModel * tm = SqliteTaskModel::instance(cat_id,0);
	QModelIndex mix ;

	for( int i = 0 ; i < tm->rowCount() ; i ++ )
	{
		mix = tm->index(i,ng::tasks::task_id);
		int currTaskId = tm->data(mix).toInt();
		if( currTaskId == pTaskId )
		{
			QString vv = tm->data(tm->index(i,ng::tasks::file_length_abtained)).toString() ;
			msc = vv == "true" ? true : false ;
			break ;
		}
	}
	
	return msc ;
}

QString TaskQueue::getRealUrlModel(int pTaskId,int cat_id)
{
	QString msc  ;
	SqliteTaskModel * tm = SqliteTaskModel::instance(cat_id,0);
	QModelIndex mix ;

	for( int i = 0 ; i < tm->rowCount() ; i ++ )
	{
		mix = tm->index(i,ng::tasks::task_id);
		int currTaskId = tm->data(mix).toInt();
		if( currTaskId == pTaskId )
		{
			msc = tm->data(tm->index(i,ng::tasks::real_url)).toString() ;
			break ;
		}
	}	
	return msc ;
}

int TaskQueue::getActiveSegCount(int pTaskId, int cat_id)
{
	int msc = -1 ;
	SqliteTaskModel *tm = SqliteTaskModel::instance(cat_id,0);
	QModelIndex mix;

	return msc;
}

// void TaskQueue::setTaskId(int task_id)
// {
// 	// this->mTaskId = task_id ;
// }

QBitArray TaskQueue::getCompletionBitArray(int taskId)
{
	QModelIndex idx;
    QString bitString;
    int numPieces = 0;

	//maybe should use mCatID , but we cant know the value here , so use default downloading cat
	QAbstractItemModel * mdl = SqliteTaskModel::instance(ng::cats::downloading, this);

    // i known it must be only one match
    QModelIndexList mil = mdl->match(mdl->index(0, ng::tasks::task_id), Qt::DisplayRole, 
                                     QVariant(QString("%1").arg(taskId)), 1, Qt::MatchExactly | Qt::MatchWrap);
    // qDebug()<<"found match model:"<<mil;

    if (mil.count() == 1) {
        idx = mil.at(0);
        bitString = mdl->data(mdl->index(idx.row(), ng::tasks::total_packet)).toString();
        numPieces = mdl->data(mdl->index(idx.row(), ng::tasks::total_block_count)).toInt();
        // qDebug()<<__FUNCTION__<<numPieces<<bitString;

        QStringList bitList = bitString.split(",");
        QBitArray ba(bitList.length());
        if (!bitString.isEmpty()) {
            for (int i = 0; i < bitList.count(); i++) {
                ba.setBit(i, bitList.at(i).at(0) == '1');
            }
        }
        // ba.resize(qMin(bitList.length(), numPieces));
        ba.resize(numPieces);
        // qDebug()<<"string is "<<bitString<<" BS:"<<ba.size()<<ba;
        // dumpBitArray(ba);
        // return bitString;
        return ba;
    } else {
        // qDebug()<<__FUNCTION__<<"can not found bit model";
        return QBitArray();
    }
}

void TaskQueue::onTaskLogArrived(QString cuid, QString itime, QString log)
{
    int taskId;
    Task *task = NULL;
    
    // qDebug()<<"LOG-PART:"<<cuid<<itime<<log;
    return; // use so much memory, omit it now

    if (this->mTaskCUIDs.contains(cuid)) {
        taskId = this->mTaskCUIDs.value(cuid);
        task = this->mTasks.value(taskId);

        task->mLogs.append(QPair<QString, QString>(itime, log));
    } else {
        // unattached log
        if (this->mUnattachedLogs.contains(cuid)) {
            
            this->mUnattachedLogs[cuid].append(QPair<QString, QString>(itime, log));
        } else {
            QVector<QPair<QString, QString> > vlog;
            vlog.append(QPair<QString, QString>(itime, log));
            this->mUnattachedLogs.insert(cuid, vlog);
        }
    }
}

void TaskQueue::deleteLater () 
{
	qDebug()<<__FUNCTION__<<__LINE__;
	QObject::deleteLater();
}



