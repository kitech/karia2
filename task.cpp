#include "task.h"

Task::Task(QObject *parent)
	: QObject(parent)
{

}

Task::~Task()
{

}


//取状态描述
//static 
QString Task::getStatusString(int status) 
{
	QString ss ;	//enum { TS_READY , TS_WAIT_FOR_RUN , TS_RUNNING , TS_ERROR , TS_SCHEDULED , TS_DONE } ;
	switch (status)
	{
	case Task::TS_READY:
		ss = "ready";
		break ;
	case Task::TS_RUNNING:
		ss = "running";
		break ;
	case Task::TS_ERROR:
		ss = "error";
		break;
	case Task::TS_DONE:
		ss = "done";
		break;
	case Task::TS_DELETED:
		ss = "deleted";
		break ;
	default : 
		ss = "unknown status";
		break ;
	}
	return ss ;
}
//static 
QString Task::getStatusDescription( int status ) 
{
	QString ss ;
	switch (status)
	{
	case Task::TS_READY:
		ss = "ready";
		break ;

	case Task::TS_RUNNING:
		ss = "running";
		break ;
	case Task::TS_ERROR:
		ss = "error";
		break;
	case Task::TS_DONE:
		ss = "done";
		break;
	case Task::TS_DELETED:
		ss = "deleted";
		break ;
	default : 
		ss = "unknown status";
		break ;
	}
	return ss ;
}

//static 
QString Task::getFileNameByUrl(QString url )
{
	QString fname ;
	QFileInfo fi(url);
	if(fi.fileName().length() == 0 ) 
	{
		fname = "index.html";
	}
	else
	{
		fname = fi.fileName();
	}	
	return fname ;
}

int Task::onRedirectLocation(int pTaskId , int pSegId ,QString redirectLocation,int redirectTimes)	//handle redirect case
{
	qDebug() << __FUNCTION__ ;


	return 0 ;
}

void Task::onOneSegmentFinished(  ) 
{

	long startOffset ;
	long totalLength ;
	long gotLength ;
	long totalEmitLength ;
	int  segCount ;
	long splitLength , splitOffset , splitLeftLength ;
	
}



void Task::onStartSegment(int pTaskId,int pSegId)
{
	qDebug()<<__FUNCTION__<<pTaskId<<pSegId;
	int row =0 ;
	BaseRetriver *br = 0 ;
	QAbstractItemModel * logmdl = 0 ;
	SqliteSegmentModel * segmdl = 0 ;

	logmdl = SegmentLogModel::instance( pTaskId , pSegId , this );
	segmdl = SqliteSegmentModel::instance(pTaskId , this);

	//br = this->findRetriverById(pTaskId,pSegId);
	if( br != 0 )
	{
		qDebug()<<"br !=0 : " ;
		if( br->isRunning() )
		{
			qDebug()<<"br !=0 : and is running " ;	
		}
		else
		{
			qDebug()<<"br !=0 : but not running , delete it and recreate the segment " ;	
			//我们的决策是新创建一个空线程，再调度运行
			//this->mSegmentThread.replace(pSegId, 0 ) ;			
			//this->mSegMapMutex.lock();	
			//if( this->mSegmentThread.contains(pSegId) )
			//{
			//	this->mSegmentThread.remove(pSegId );
			//	delete br ; br = 0 ;
			//	this->onTaskListCellNeedChange(pTaskId,ng::tasks::active_block_count,
			//		QString("%1").arg( this->getActiveSegCount(pTaskId,ng::cats::downloading)) );
			//	qDebug()<< "this->mSegmentThread.remove(segId );"<<this->mSegmentThread.count()<<":"<<this->mSegmentThread.size();
			//}
			//this->mSegMapMutex.unlock();

			qDebug()<<"br !=0 : but not running , delete it and recreate the segment "<<  this->mSegmentThread.count() ;	
		}
	}
	else	//没有找到相应的线程运行实例
	{
		quint64 SP = segmdl->malloc();
		qDebug()<<"segmdl malloc size : "<< SP ;

		if( this->getFileAbtained(this->mTaskId,ng::cats::downloading) == true )
		{
			qDebug()<<__LINE__<<"if( this->mFileLengthAbtained == true )";
			if( SP == (quint64)(SqliteSegmentModel::MM_NULL ) )
			{	
				qDebug()<<" return from  "<< __FUNCTION__<<__LINE__ ;
				return ;	
			}
			br = this->getRetriverByUrlScheme(this->getBestTaskUrl() );	//new 
			this->connectRetriverSignals(br);
			assert( br != 0 ) ;
			assert( SP != (quint64)(SqliteSegmentModel::MM_NOT_EXSIT ) );
			//br->mTaskOption = this->mTaskOption ;
			br->mTaskId = pTaskId ;
			br->mSegId =  pSegId ;
			br->mUrl = this->getBestTaskUrl()  ;
			br->mStartOffset = SP ;
			
			
			if( SP == (quint64)(SqliteSegmentModel::MM_NOT_EXSIT ) )
			{	
				qDebug()<<__FUNCTION__<<__LINE__<< " alloc MM_NOT_EXSIT  , maybe no mem set" ;
				br->mStartOffset = 0 ;
			}
			else if( SP == (quint64)(SqliteSegmentModel::MM_NULL ) )
			{
				qDebug()<<__FUNCTION__<<__LINE__<< " alloc 0 bytes , maybe no mem left" ;
				return ;
			}
			else
			{
				
			}
			segmdl->setBlockBusy(SP,true);
			br->start();
			this->mSegMapMutex.lock();
			this->mSegmentThread.insert(pSegId,br);
			this->mSegMapMutex.unlock();
			this->onTaskListCellNeedChange(pTaskId,ng::tasks::active_block_count,
					QString("%1").arg( this->getActiveSegCount(pTaskId,ng::cats::downloading)) );
		}	// end if( this->mFileLengthAbtained == true )
		else
		{
			qDebug()<<__LINE__<<"if( this->mFileLengthAbtained != true )";
			//可能需要等待，也可能它一马当先去获取这个文件大小。

			br = this->getRetriverByUrlScheme( this->getBestTaskUrl() );	//new 
			this->connectRetriverSignals(br);
			assert( br != 0 ) ;
			//br->mTaskOption = this->mTaskOption ;
			br->mTaskId = pTaskId ;
			br->mSegId =  pSegId ;
			br->mUrl = this->getBestTaskUrl()  ;
			
			if( SP == (quint64)(SqliteSegmentModel::MM_NOT_EXSIT ) )
			{				
			}
			else if( SP == (quint64)(SqliteSegmentModel::MM_NULL ) )
			{
				qDebug()<<__FUNCTION__<<__LINE__<< " alloc 0 bytes , maybe no mem left" ;
				return ;
			}
			else
			{
				br->mStartOffset = SP ;
			}
			segmdl->setBlockBusy(SP,true);
			br->start();
			//this->mSegmentThread.append(br);
			this->mSegMapMutex.lock();
			this->mSegmentThread.insert(pSegId,br);
			this->mSegMapMutex.unlock();
			this->onTaskListCellNeedChange(pTaskId,ng::tasks::active_block_count,
					QString("%1").arg( this->getActiveSegCount(pTaskId,ng::cats::downloading) ) );
			qDebug()<<"task: "<< pTaskId <<" , seg : "<< pSegId <<" , file length unknown and no retr instance .  action omited";
		}
	}
}

void Task::onPauseSegment(int pTaskId,int pSegId)
{
	QList<quint64>  keys = this->mSegmentThread.keys();
	qDebug()<< keys ;
	if( this->mSegmentThread.contains(pSegId) == true)
	{
		BaseRetriver * br = this->mSegmentThread.value(pSegId);
		if( br != 0 )
		{
			br->bePauseing = true ;
			//br->wait();
			br->quit();
			//qDebug()<< "before waiting the retriver thread to stop" ;
			//br->wait();
			//qDebug()<< "after waiting the retriver thread to stop" ;
		}
	}

}


void Task::onLogSegment( int taskId , int segId , QString log , int type ) 
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

	
void Task::onFirstSegmentReady(int pTaskId , long totalLength, bool supportBrokenRetrive)
{
	qDebug() << __FUNCTION__ ;

	qDebug()<<  __FUNCTION__ <<" return for nothing done " ;
	return ;

}

/**
 * 除了修改这个控制参数外还应该做点什么呢。
 * 这个可以由子线程发出多次，但如果第二次发应该是错误的，这并没有关系，我们只在这里判断就行了。
 */
void Task::onAbtainedFileLength( int pTaskId , long totalLength , bool supportBrokenRetrive)
{
	qDebug() << __FUNCTION__ << "taskId: "<< pTaskId << "totalLength :" << totalLength ;
	//if( this->mFileLengthAbtained == false )
	if( this->getFileAbtained(pTaskId,ng::cats::downloading) == false )
	{
		//this->mFileLengthAbtained = true ;	//这个参数非常重要
		this->mTotalLength = totalLength ;
		this->onTaskListCellNeedChange( pTaskId , (int)ng::tasks::file_length_abtained , QString("true") );
		this->onTaskListCellNeedChange( pTaskId , (int)ng::tasks::file_size , QString("%1").arg(totalLength) );
		//emit this->attemperTaskStatus(this ,(int) TaskQueue::TS_RUNNING );
		//SqliteSegmentModel::instance(pTaskId,this)->setCapacity(totalLength );
		//this->onStartTask(pTaskId);
	}
	else
	{
		assert( 1== 2 );
	}
}


void Task::onMemoryOverLoad()
{
	qDebug()<<__FUNCTION__<<__LINE__ ;
	emit this->taskDone( this->mTaskId );
}

int Task::getMaxSegCount(int pTaskId,int cat_id)
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

bool Task::getFileAbtained( int pTaskId , int cat_id)
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

QString Task::getRealUrlModel(int pTaskId,int cat_id)
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

int Task::getActiveSegCount(int pTaskId, int cat_id)
{
	int msc = -1 ;
	SqliteTaskModel * tm = SqliteTaskModel::instance(cat_id,0);
	QModelIndex mix ;

	//for( int i = 0 ; i < tm->rowCount() ; i ++ )
	//{
	//	mix = tm->index(i,ng::tasks::task_id);
	//	int currTaskId = tm->data(mix).toInt();
	//	if( currTaskId == pTaskId )
	//	{
	//		msc = tm->data(tm->index(i,ng::tasks::active_block_count)).toInt() ;

	//		break ;
	//	}
	//}
	msc = this->mSegmentThread.count();

	//assert( msc > 0 );
	return msc ;
}

QList<QPair<long , long> >  Task::getCompletionList() 
{
	QList<QPair<long , long> >  retlist ;

	QVector<QPair< quint64,quint64> >  bv ;

	QModelIndex idx ;
	SqliteSegmentModel * mdl = 0;
	mdl = SqliteSegmentModel::instance(this->mTaskId,0);

	for( int i = 0 ; i < mdl->rowCount(); i ++ )
	{
		idx = mdl->index(i,ng::segs::start_offset);
		quint64 so = mdl->data(idx).toULongLong();
		idx = mdl->index(i,ng::segs::abtained_length);
		quint64 gl = mdl->data(idx).toULongLong() ;
		retlist.append(QPair<long,long> ( so,gl) );
	}
	
	return retlist ;
}


void Task::onTaskListCellNeedChange(int taskId , int cellId , QString value  )
{
	//qDebug() << __FUNCTION__ ;

	QModelIndex idx , idx2,idx3;
	quint64 fsize , abtained ;
	double  percent ;

	//maybe should use mCatID , but we cant know the value here , so use default downloading cat
	QAbstractItemModel * mdl = SqliteTaskModel::instance( ng::cats::downloading ,  this) ;

	//QDateTime bTime , eTime ; 
	//bTime = QDateTime::currentDateTime();
	int taskCount = mdl->rowCount();
	for( int i = 0 ; i < taskCount ; i ++ )
	{	
		//find the cell index
		idx = mdl->index(i,ng::tasks::task_id );

		if( idx.data() == QVariant(taskId) )
		{			
		//	qDebug()<<"found index of cell" ;
			idx = mdl->index(i,cellId);
			//change the value 
			mdl->setData(idx,value);
			if(  cellId == ng::tasks::abtained_length )
			{
				idx = mdl->index(i,ng::tasks::file_size);
				idx2 = mdl->index(i,ng::tasks::left_length) ;
				idx3 = mdl->index(i,ng::tasks::abtained_percent) ;
				abtained = value.toULongLong() ;
				fsize = mdl->data(idx).toULongLong() ;
				percent = 	100.0*abtained/ fsize ;
				//change the value 
				mdl->setData(idx2 ,	fsize -  abtained );
				mdl->setData(idx3 ,percent );

				//通知DropZone
				emit this->taskCompletionChanged( taskId , (int)percent , 
					this->getRealUrlModel(taskId,ng::cats::downloading) );
				emit this->taskCompletionChanged( this , false ) ;

			}
			break;
		}
	}
	//eTime = QDateTime::currentDateTime(); 
	//qDebug()<<"begin:\t"<<bTime.toString("yyyy.MM.dd-hh:mm:ss.zzz")
	//	<<"\nend:\t"<<eTime.toString("yyyy.MM.dd-hh:mm:ss.zzz")<<"\n\n";
}


void Task::onSegmentGotLengthNeedUpdate ( int taskId , int segId , long delta , int optType )
{
	//qDebug() << __FUNCTION__ << delta << optType ;
	TaskQueue * tq = 0 ;
	SqliteSegmentModel * mdl = 0 ;
	
	QDateTime currTime = QDateTime::currentDateTime();

	//mAtomMutex.lock();

	int nowRows ;
	QModelIndex index ;	
	
	int segCount ;
	QVector<BaseRetriver*> sq ;
	BaseRetriver* psq ;
	quint64 task_abtained = 0 ;
	
	//tq = this ;	

	//sq = this->mSegmentThread ;
	//qDebug()<<sq.size() ;
	segCount = sq.size();
	

	mdl = SqliteSegmentModel::instance(taskId,this);

	
	for( int i = 0 ; i < mdl->rowCount() ; ++i)
	{
		if( mdl->data(mdl->index(i,ng::segs::seg_id)).toULongLong() == segId )
		{
			quint64 al = mdl->data(mdl->index(i,ng::segs::abtained_length)).toULongLong();
			mdl->setData(mdl->index(i,ng::segs::abtained_length),al);			
			//break;
			task_abtained += al ;
		}
		else
		{
			task_abtained += mdl->data(mdl->index(i,ng::segs::abtained_length)).toULongLong(); ;
		}
	}
	
	
	TaskBallMapWidget::instance()->onRunTaskCompleteState(tq , true );	
	
	this->onTaskListCellNeedChange(taskId,ng::tasks::abtained_length ,QString("%1").arg(task_abtained));

	
	//mAtomMutex.unlock();


	return;	
}
/**
 * 用于修改线程模型的数据。
 */
void Task::onSegmentCellNeedChange( int taskId , int segId ,  int cellId , QString value ) 
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


BaseRetriver * Task::getRetriverByUrlScheme(QString strurl)
{
	BaseRetriver * retr = 0 ;

	QUrl url (strurl);

	if( url.scheme().toUpper().compare("HTTP") == 0 )		
	{
		retr = new HTTPRetriver();
	}
	else if(url.scheme().toUpper().compare("HTTPS") == 0)
	{
		retr = new HTTPSRetriver();
	}
	else if( url.scheme().toUpper().compare("FTP") == 0 )
	{	
		retr = new FTPRetriver();
	}
	else if (  url.scheme().toUpper().compare("FILE") == 0 )
	{
		retr = new FileRetriver();
	}
	else if ( url.scheme().toUpper().compare("RTSP") == 0 )
	{
		retr = new RTSPRetriver();
		this->mNeedSpecialHandling = true ;
		this->mSpecialHander = & Task::RTSPSpecialHandler ;
	}
	else if ( url.scheme().toUpper().compare("MMS") == 0 
		|| url.scheme().toUpper().compare("MMSH") == 0 
		|| url.scheme().toUpper().compare("MMST") == 0 )
	{
		retr = new MMSRetriver();
		this->mNeedSpecialHandling = true ;
		this->mSpecialHander = & Task::MMSSpecialHandler  ;
	}
	else
	{
		qDebug()<<"Not Supported scheme"<<__FILE__<<__LINE__;
	}	

	return retr ;
}

bool Task::connectRetriverSignals(BaseRetriver *retr)
{

	QObject::connect(retr,SIGNAL( oneFinished( int , int , int ) ) , 
		this,SLOT(onOneSegmentFinished(int,int,int)) );
	QObject::connect(retr,SIGNAL( firstFaild( int , int  ) ) , 	this,SLOT(onFirstSegmentFaild(int,int)) );
	QObject::connect(retr,SIGNAL( logSegment( int  , int  , QString  , int  ) ),
		this, SLOT(onLogSegment(int  , int  , QString  , int )) );
	////update task list view model cell signal				
	QObject::connect(retr,SIGNAL( taskElemNeedUpdate( int  , int  , QString  ) ),
		this, SLOT(onTaskListCellNeedChange(int  , int  , QString )) );
	QObject::connect(retr,SIGNAL( segmentGotLengthNeedUpdate( int  , int  ,long, int  ) ),
		this, SLOT(onSegmentGotLengthNeedUpdate(int  , int  , long , int )) );

	QObject::connect(retr,SIGNAL( fileLengthAbtained(int,long,bool)) ,
		this,SLOT(onAbtainedFileLength(int,long,bool) ) ) ;

	return true ;
}

QString Task::getBestTaskUrl()
{
	QString bestUrl ;
	//bestUrl = this->mTaskUrl ;
	bestUrl = this->getRealUrlModel(this->mTaskId,ng::cats::downloading);
	return bestUrl ;
}


bool Task::RTSPSpecialHandler( int a_int,void * a_void_pointer , Task * a_Task_pointer )
{
	qDebug()<<__FUNCTION__<<__LINE__<<"a_int:"<<a_int
		<<" a_void_pointer:"<<a_void_pointer
		<< " a_TaskQueue_pointer:"<<a_Task_pointer ;

	//////
	
	SqliteSegmentModel * model = SqliteSegmentModel::instance(this->mTaskId,0);
	QUrl theFile (this->getBestTaskUrl() );
	QString savePath = SqliteStorage::instance(this)->getSavePathByCatId(ng::cats::downloading);
	QString endFileName = savePath + "/"+ QFile(theFile.path()).fileName();
	QString fileDir = endFileName+".ng!/";
	qDebug()<<endFileName ;
	qDebug()<<fileDir ;

	QFile endFile(endFileName);
	int mrows = 0 ;
	char readBuffer[8192] = {0};
	qint64 readLength = 0 ; 

	mrows = model->rowCount();
	endFile.open(QIODevice::ReadWrite);
	endFile.seek(0);
	for( int i = 0 ; i < mrows ; i ++)
	{
		QString startOffset = model->data(model->index(i,ng::segs::start_offset)).toString();
		//QString tmpChunkFileName = fileDir + endFileName + ".ng!."+startOffset ;
		QString tmpChunkFileName =fileDir + QFile(theFile.path()).fileName() + ".ng!."+startOffset ;
		qDebug()<<" chunk file "<< i <<" "<< tmpChunkFileName ;
		QFile chunkFile(tmpChunkFileName);
		chunkFile.open(QIODevice::ReadWrite);
		while( true )
		{
			readLength = chunkFile.read(readBuffer,sizeof(readBuffer));
			if( readLength <=  0)
			{
				break ;
			}
			endFile.write(readBuffer,readLength);
		}
		chunkFile.close();
	}

	endFile.close();

	//deleting download temp chunk files and directories

	return true ;
}

bool Task::MMSSpecialHandler( int a_int,void * a_void_pointer , Task * a_Task_pointer )
{
	qDebug()<<__FUNCTION__<<__LINE__<<"a_int:"<<a_int
		<<" a_void_pointer:"<<a_void_pointer
		<< " a_TaskQueue_pointer:"<<a_Task_pointer ;

	return true ;
}
