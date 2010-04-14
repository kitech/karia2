// sqlitestorage.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 19:36:28 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>
#include <QMainWindow>
#include <QMenu>
#include <QActionGroup>
#include <QStatusBar>
#include <QMessageBox>
#include <QDialog>
#include <QFileDialog>
#include <QUrlInfo>
#include <QFileInfo>
#include <QList>
#include <QToolTip>
#include <QScrollArea>
#include <QRect>
#include <QSize>
#include <QProcess>
#include <QDockWidget>
#include <QSystemTrayIcon>
#include <QDesktopServices>
#include <QProgressDialog>

#include <QSqlRecord>

#include "sqlitestorage.h"

//static 
SqliteStorage * SqliteStorage::mHandle = 0;
SqliteStorage * SqliteStorage::instance(QObject *parent)
{
	if( SqliteStorage::mHandle == 0 )
	{
		//SqliteStorage::mHandle = new SqliteStorage(parent);
		SqliteStorage::mHandle = new SqliteStorage(0);
	}
	return SqliteStorage::mHandle ;
}


SqliteStorage::SqliteStorage(QObject *parent)
	: AbstractStorage(parent)
{
	this->dbSuffix = ".ndb";
	this->optionDBName = this->storePath+this->optionsPrefixName + this->dbSuffix ;
	this->taskDBName  = this->storePath + this->tasksPrefixName + this->dbSuffix  ;

	optInsSql = "insert into %1 ( option_name  , option_value  , option_type , dirty ) "
		" values ( '%2' , '%3' , '%4', '%5' )" ;

	catInsSql = " insert into categorys ( cat_id , display_name , raw_name , folder , path , can_child , parent_cat_id , create_time , delete_flag , dirty) "
		" values ( '%1', '%2','%3','%4','%5','%6','%7','%8','%9','%10' ) " ;

	segInsSql = " insert into segments ( seg_id,task_id,start_offset,create_time,finish_time,total_length,abtained_length,current_speed,average_speed,abtained_percent,segment_status,total_packet,abtained_packet,left_packet,total_timestamp,finish_timestamp,left_timestamp,dirty) "
		" values (  '%1', '%2','%3','%4','%5','%6','%7','%8' , '%9' , '%10' , '%11' , '%12' , '%13' , '%14' , '%15' , '%16','%17','%18' ) " ;
	//19
	taskInsSql = " insert into tasks ( task_id,file_size,retry_times,create_time,current_speed,average_speed,eclapsed_time,abtained_length,left_length,block_activity,total_block_count,active_block_count,cat_id,comment,place_holder,file_name,abtained_percent,org_url,real_url,redirect_times,finish_time,task_status,total_packet,abtained_packet,left_packet,total_timestamp,abtained_timestamp,left_timestamp,file_length_abtained,dirty,aria_gid ) "
		"values ( '%1', '%2','%3','%4','%5','%6','%7','%8','%9' , '%10', '%11','%12','%13','%14','%15','%16','%17','%18' , '%19' , '%20', '%21' , '%22', '%23', '%24', '%25', '%26', '%27', '%28', '%29', '%30', '%31') " ;

    // Marks the string literal sourceText for dynamic translation in the current context (class),
	tasksModelColumnsOrderShow = QT_TR_NOOP("task_id, file_size, retry_times, create_time, current_speed, average_speed, eclapsed_time, abtained_length, left_length, block_activity, total_block_count, active_block_count, cat_id,comment, place_holder, file_name,abtained_percent, org_url, real_url, redirect_times, finish_time, task_status, total_packet, abtained_packet,left_packet, total_timestamp, abtained_timestamp, left_timestamp, file_length_abtained, dirty, aria_gid");
	catsModelColumnsOrderShow = QT_TR_NOOP("display_name, path, cat_id, parent_cat_id, can_child, raw_name, folder, delete_flag, create_time, dirty");
	segsModelColumnsOrderShow = QT_TR_NOOP("seg_id, task_id, start_offset, create_time, finish_time, total_length, abtained_length, current_speed, average_speed, abtained_percent, segment_status, total_packet, abtained_packet, left_packet, total_timestamp, finish_timestamp, left_timestamp, dirty");

	this->tasksModelColumnsOrder = tasksModelColumnsOrderShow ;
	this->catsModelColumnsOrder = catsModelColumnsOrderShow;
	this->segsModelColumnsOrder = segsModelColumnsOrderShow; 

}

SqliteStorage::~SqliteStorage()
{

}

bool SqliteStorage::open()
{
	//QSqlDatabase::addDatabase("QSQLITE");

	if( ! QSqlDatabase::contains(this->optionsPrefixName) )
		QSqlDatabase::addDatabase("QSQLITE",this->optionsPrefixName);
	if( ! QSqlDatabase::contains(this->tasksPrefixName) )
		QSqlDatabase::addDatabase("QSQLITE",this->tasksPrefixName);
	//QSqlDatabase::addDatabase("QSQLITE",this->logsPrefixName);
	//QSqlDatabase::addDatabase("QSQLITE",this->mirrorsPrefixName);

	this->mOptionsDB = QSqlDatabase::database(this->optionsPrefixName,false );
	this->mTasksDB = QSqlDatabase::database(this->tasksPrefixName,false );
	//this->mLogsDB = QSqlDatabase::database(this->logsPrefixName);
	//this->mMirrorsDB = QSqlDatabase::database(this->mirrorsPrefixName);

	this->mOptionsDB.setDatabaseName(this->optionDBName);
	this->mTasksDB.setDatabaseName(this->taskDBName);
	qDebug()<< this->optionDBName ;
	qDebug()<< this->taskDBName ;

	if( QFile::exists(this->optionDBName ) )
	{
		this->mOptionsDB.open();
		//this->initDefaultOptions();
		//dumpDefaultOptions () ;
	}
	else
	{
		this->mOptionsDB.open();
		if( this->mOptionsDB.isValid() )
		{
			this->mOptionsDB.close();
			QFile::remove(this->optionDBName);
			this->mOptionsDB.open();
			this->initDefaultOptions();
		}
	}

	if( QFile::exists(this->taskDBName) )
	{
		this->mTasksDB.open();
		//this->dumpDefaultTasks();
	}
	else
	{
		this->mTasksDB.open();
		if( this->mTasksDB.isValid() )
		{
			this->mTasksDB.close();
			QFile::remove(this->optionDBName);
			this->mTasksDB.open();
			this->initDefaultTasks();
			
		}
	}

	return true ;
}

bool SqliteStorage::close()
{
	if(this->mOptionsDB.isValid() && this->mOptionsDB.isOpen())
		this->mOptionsDB.close();
	if(this->mTasksDB.isValid() && this->mTasksDB.isOpen())
		this->mTasksDB.close();
	//this->mLogsDB.close();
	//this->mMirrorsDB.close();
	return true ;
}


bool SqliteStorage::initDefaultOptions () 
{
	char * createSql = "create table default_options ( option_name varchar(32) primary key , option_value varchar(64) , option_type varchar(32) , dirty varchar(32) default 'false' ) " ;

	QSqlQuery q( this->mOptionsDB );
	q.exec( createSql );

	QString insSql  = this->optInsSql ;
	QMap<QString , QString>::const_iterator mit;
	for( mit = this->defaultOptions.begin() ; mit != this->defaultOptions.end() ; mit ++)
	{
		QString sql = insSql.arg("default_options").arg(mit.key()).arg(mit.value()).arg("string").arg("false") ;
		//qDebug()<< sql ;
		q.exec(sql );
	}

	createSql = "create table user_options (  option_name varchar(32) primary key  , option_value varchar(64) , option_type varchar(32) , dirty varchar(32)  default 'false' ) " ;
	q.exec( createSql );

	return true ;
}

bool SqliteStorage::dumpDefaultOptions () 
{
	QString sql = " select * from default_options ";

	QSqlQuery q( this->mOptionsDB );
	q.exec( sql );

	while( q.next() )
	{
		QSqlRecord rec = q.record();
		//qDebug()<< " field Count : "<< rec.count() ;
		//for( int  i = 0 ; i < rec.count() ; i ++)
		{
			qDebug()<< rec.fieldName(0)<<"="<< rec.value(0) << " ," 
				<< rec.fieldName(1)<<"="<< rec.value(1) << " ," 
				<< rec.fieldName(2)<<"="<< rec.value(2) ;
		}
	}

	return true ;
}

bool SqliteStorage::initDefaultTasks() 
{
	QString createSql = " create table tasks (  %1 ) "; 
	
	QStringList sl = QString(this->mTaskColumnStr).split(",");
	QString tempSql  ;
	for( int i = 0 ; i < sl.count() ; i ++ )
	{
		if( sl.at(i).trimmed().endsWith("_id") )
		{
			tempSql += sl.at(i).trimmed() + " integer ";
		}
		else
		{
			tempSql += sl.at(i).trimmed() + " varchar(255) ";
		}
		if( sl.at(i).trimmed() == "task_id")
		{
			tempSql += " primary key  ";
		}
		if( i < sl.count() -1 ) tempSql += " , " ;
	}
	createSql = createSql.arg(tempSql ) ;	
	qDebug()<< createSql ;

	QSqlQuery q( this->mTasksDB );
	q.exec( createSql );

	createSql = " create table segments ( %1 ,  primary key ( seg_id,task_id) )   " ;
	sl = QString(this->mSegColumnStr).split(",");
	tempSql = "";
	for( int i = 0 ; i < sl.count() ; i ++ )
	{
		if( sl.at(i).trimmed().endsWith("_id") )
		{
			tempSql += sl.at(i).trimmed() + " integer ";
		}
		else
		{
			tempSql += sl.at(i).trimmed() + " varchar(255) ";
		}
		if( i < sl.count() -1 ) tempSql += " , " ;
	}
	createSql = createSql.arg(tempSql);
	q.exec( createSql );
	qDebug()<< createSql ;

	createSql = " create table categorys ( %1 )  ";
	sl = QString(this->mCatColumnStr).split(",");
	tempSql = "";
	for( int i = 0 ; i < sl.count() ; i ++ )
	{
		if( sl.at(i).trimmed().endsWith("_id") )
		{
			tempSql += sl.at(i).trimmed() + " integer ";
		}
		else
		{
			tempSql += sl.at(i).trimmed() + " varchar(255) ";
		}
		if( sl.at(i).trimmed() == "cat_id")
		{
			tempSql += " primary key  ";
		}
		if( i < sl.count() -1 ) tempSql += " , " ;
	}
	createSql = createSql.arg(tempSql);
	q.exec( createSql );
	qDebug()<< createSql ;

	QString insSql = this->catInsSql ;

	for( int cnt = 0 ; cnt < this->defaultCategorys.count() ;  cnt ++)
	{
		QMap<QString , QString> onecat = this->defaultCategorys.at(cnt);
		QString realSql = QString(insSql)
			.arg(onecat["cat_id"])
			.arg(onecat["display_name"])
			.arg(onecat["raw_name"])
			.arg(onecat["folder"])
			.arg(onecat["path"])
			.arg(onecat["can_child"])
			.arg(onecat["parent_cat_id"])
			.arg(onecat["create_time"])
			.arg(onecat["delete_flag"]) 
			.arg(onecat["dirty"]) ;
		q.exec( realSql );

	}

	createSql = " create table seq_tasks ( seq_id integer primary key autoincrement , seq_null integer ) ";

	q.exec(createSql);

	return true ;
}

bool SqliteStorage::dumpDefaultTasks() 
{
	QString sql = "select * from categorys ";
	
	QSqlQuery q ( this->mTasksDB);
	q.exec(sql );
	while( q.next() )
	{
		QSqlRecord rec = q.record();
		//qDebug()<< " field Count : "<< rec.count() ;
		for( int  i = 0 ; i < rec.count() ; i ++)
		{
			qDebug()<< rec.fieldName(i)<<"="<< rec.value(i)  ;
		}
	}
	qDebug()<< q.lastError()<<": " << sql ;

	return true ;
	return true ;
}

bool SqliteStorage::addUserOptions(QString key , QString value , QString type  )
{
	QString sql = this->optInsSql.arg("user_options").arg(key).arg(value).arg(type);
	
	QSqlQuery q ( this->mOptionsDB);
	q.exec(sql );

	qDebug()<< q.lastError()<<": " << sql ;

	return true ;
}
bool SqliteStorage::deleteUserOption( QString key )
{
	QString sql = QString("delete from user_options where option_name='%1'").arg(key);

	QSqlQuery q ( this->mOptionsDB);
	q.exec(sql );
	
	qDebug()<< q.lastError()<<": " << sql ;
	return true ;
}
bool SqliteStorage::addTask( int task_id , 
			QString file_size            , 
			QString retry_times          ,
			QString create_time          ,
			QString current_speed        ,
			QString average_speed        ,
			QString eclapsed_time        ,
			QString abtained_length      ,
			QString left_length          ,
			QString block_activity         ,
			QString total_block_count    ,
			QString active_block_count   ,
			QString cat_id               ,
			QString comment              ,
			QString place_holder         ,
			QString file_name            ,
			QString abtained_percent          ,
			QString org_url              ,
			QString real_url             ,
			QString redirect_times       ,
			QString finish_time          ,
			QString task_status          ,
			QString total_packet         ,
			QString abtained_packet      ,
			QString left_packet          ,
			QString total_timestamp      ,
			QString abtained_timestamp  ,
			QString left_timestamp       ,
			QString file_length_abtained ,
			QString dirty                									
							)
{
	QString sql = this->taskInsSql.arg(task_id).arg(file_size).arg(retry_times).arg(create_time).arg(current_speed).arg(average_speed).arg(eclapsed_time).arg(abtained_length).arg(left_length).arg(block_activity).arg(total_block_count).arg(active_block_count).arg(cat_id).arg(comment).arg(place_holder).arg(file_name).arg(abtained_percent).arg(org_url).arg(real_url).arg(redirect_times).arg(finish_time).arg(task_status).arg(total_packet).arg(abtained_packet).arg(left_packet).arg(total_timestamp).arg(abtained_timestamp).arg(left_timestamp).arg(file_length_abtained).arg(dirty).arg("0") ;

	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	//qDebug()<< q.lastError()<<": " << sql ;
	return true ;
}
bool SqliteStorage::updateTask( int task_id ,
			QString file_size            , 
			QString retry_times          ,
			QString create_time          ,
			QString current_speed        ,
			QString average_speed        ,
			QString eclapsed_time        ,
			QString abtained_length      ,
			QString left_length          ,
			QString block_activity         ,
			QString total_block_count    ,
			QString active_block_count   ,
			QString cat_id               ,
			QString comment              ,
			QString place_holder         ,
			QString file_name            ,
			QString abtained_percent     ,
			QString org_url              ,
			QString real_url             ,
			QString redirect_times       ,
			QString finish_time          ,
			QString task_status          ,
			QString total_packet         ,
			QString abtained_packet      ,
			QString left_packet          ,
			QString total_timestamp      ,
			QString abtained_timestamp  ,
			QString left_timestamp       ,
			QString file_length_abtained ,
                                QString dirty                ,
                                QString aria_gid
							   
							   )
{
	QString sql = "update tasks set file_size='%1',retry_times='%2',create_time='%3',current_speed='%4',average_speed='%5',eclapsed_time='%6',abtained_length='%7',left_length='%8',block_activity='%9',total_block_count='%10',active_block_count='%11',cat_id='%12',comment='%13',place_holder='%14',file_name='%15',abtained_percent='%16',org_url='%17',real_url='%18',redirect_times='%19',finish_time='%20',task_status='%21',total_packet='%22',abtained_packet='%23',left_packet='%24',total_timestamp='%25',abtained_timestamp='%26',left_timestamp='%27',file_length_abtained='%28',dirty='%29', aria_gid='%30' where task_id='%31' ";

	sql = sql.arg(file_size).arg(retry_times).arg(create_time).arg(current_speed).arg(average_speed).arg(eclapsed_time).arg(abtained_length).arg(left_length).arg(block_activity).arg(total_block_count).arg(active_block_count).arg(cat_id).arg(comment).arg(place_holder).arg(file_name).arg(abtained_percent).arg(org_url).arg(real_url).arg(redirect_times).arg(finish_time).arg(task_status).arg(total_packet).arg(abtained_packet).arg(left_packet).arg(total_timestamp).arg(abtained_timestamp).arg(left_timestamp).arg(file_length_abtained).arg("false").arg(aria_gid).arg(task_id) ;

	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	//qDebug()<< q.lastError()<<": " << sql ;
	return true ;

}

bool SqliteStorage::addSegment ( int seg_id , int task_id , 
				QString start_offset      ,
				QString create_time       ,
				QString finish_time       ,
				QString total_length      ,
				QString abtained_length   ,
				QString current_speed     ,
				QString average_speed     ,
				QString abtained_percent  ,
				QString segment_status    ,
				QString total_packet      ,
				QString abtained_packet   ,
				QString left_packet       ,
				QString total_timestamp   ,
				QString finish_timestamp  ,
				QString left_timestamp    ,
				QString dirty					
								) 
{
	QString sql = this->segInsSql.arg(seg_id).arg(task_id).arg(start_offset).arg(create_time).arg(finish_time).arg(total_length).arg(abtained_length).arg(current_speed).arg(average_speed).arg(abtained_percent).arg(segment_status).arg(total_packet).arg(abtained_packet).arg(left_packet).arg(total_timestamp).arg(finish_timestamp).arg(left_timestamp).arg(dirty) ;

	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	//qDebug()<< q.lastError()<<": " << sql ;
	return true ;
}

bool SqliteStorage::updateSegment( int seg_id , int task_id , 								  
				QString start_offset      ,
				QString create_time       ,
				QString finish_time       ,
				QString total_length      ,
				QString abtained_length   ,
				QString current_speed     ,
				QString average_speed     ,
				QString abtained_percent  ,
				QString segment_status    ,
				QString total_packet      ,
				QString abtained_packet   ,
				QString left_packet       ,
				QString total_timestamp   ,
				QString finish_timestamp  ,
				QString left_timestamp    ,
				QString dirty								  
								  ) 
{
	QString sql = QString("update segments set  start_offset = '%1' , create_time = '%2' , finish_time = '%3' , total_length = '%4' , abtained_length = '%5' , current_speed = '%6' , average_speed = '%7' , abtained_percent = '%8' , segment_status = '%9' , total_packet = '%10' , abtained_packet = '%11' , left_packet = '%12' , total_timestamp = '%13' , finish_timestamp = '%14' , left_timestamp = '%15' , dirty='%16' where task_id='%17' and seg_id='%18' ")
		.arg(start_offset).arg(create_time).arg(finish_time).arg(total_length).arg(abtained_length).arg(current_speed).arg(average_speed).arg(abtained_percent).arg(segment_status).arg(total_packet).arg(abtained_packet).arg(left_packet).arg(total_timestamp).arg(finish_timestamp).arg(left_timestamp).arg("fasle")
		.arg(task_id).arg(seg_id) ;

	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	//qDebug()<< q.lastError()<<": " << sql ;
	return true ;
}

bool SqliteStorage::addCategory( int cat_id , QString display_name , QString raw_name , QString folder , 
								QString path , QString can_child , QString parent_cat_id , 
								QString create_time , QString delete_flag   ) 
{

	QString sql = this->catInsSql.arg(cat_id).arg(display_name).arg(raw_name).arg(folder)
		.arg(path).arg(can_child).arg(parent_cat_id).arg(create_time).arg(delete_flag).arg("false") ;

	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	//qDebug()<< q.lastError()<<": " << sql ;
	return true ;
}
bool SqliteStorage::deleteTask( int task_id )
{
	QString sql = QString("delete from tasks where task_id='%1' ").arg(task_id) ;

	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	//qDebug()<< q.lastError()<<": " << sql ;
	return true ;
}
bool SqliteStorage::deleteSegment( int task_id ,  int seg_id )
{
	QString sql = QString("delete from segments where task_id='%1'and seg_id='%2' ").arg(task_id).arg(seg_id) ;

	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	//qDebug()<< q.lastError()<<": " << sql ;
	return true ;
}
bool SqliteStorage::deleteCategory( int cat_id , bool deleteChild )
{
	QString sql = QString("delete from categorys where cat_id='%1' ").arg(cat_id) ;
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	//qDebug()<< q.lastError()<<": " << sql ;
	return true ;
}

QVector<QSqlRecord> SqliteStorage::getCatSet()
{
	QVector<QSqlRecord> dataSet ;

	//视图中显示的顺序与这里的顺序有直接的关系，可以说是一样的
	//还有排序的问题。
	QString sql = "select  " +this->catsModelColumnsOrder+ " from categorys order by parent_cat_id , cat_id  ";
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	while( q.next() )
	{
		QSqlRecord rec = q.record();
		dataSet.append(rec);
	}
	//qDebug()<< q.lastError()<<": " << sql ;
	
	return dataSet ;
}

QVector<QSqlRecord> SqliteStorage::getTaskSet( int cat_id )
{
	QVector<QSqlRecord> dataSet ;

	QSqlQuery q ( this->mTasksDB );

	//
	//视图中显示的顺序与这里的顺序有直接的关系，可以说是一样的
	//还有排序的问题。
	QString sql = QString( " select  " + QString(this->tasksModelColumnsOrderShow) +" from tasks where cat_id='%1' order by task_id desc").arg(cat_id) ;	
	
	q.exec(sql );

	while( q.next() )
	{
		QSqlRecord rec = q.record();
		dataSet.append(rec);
	}
	//qDebug()<< q.lastError()<<": " << sql  ;
	
	return dataSet ;
}
QVector<QSqlRecord> SqliteStorage::getSementSet( int task_id )
{
	QVector<QSqlRecord> dataSet ;

	//视图中显示的顺序与这里的顺序有直接的关系，可以说是一样的
	//还有排序的问题。
	QString sql = QString( "select  * from segments where task_id = '%1' order by seg_id asc " ) .arg(task_id) ;
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	while( q.next() )
	{
		QSqlRecord rec = q.record();
		dataSet.append(rec);
	}
	//qDebug()<< q.lastError()<<": " << sql ;
	
	return dataSet ;
}

int SqliteStorage::getNextValidTaskID()
{
	
	QString sql = "insert into seq_tasks ( seq_null ) values ( 0 ) " ;
	QSqlQuery q( this->mTasksDB );
	q.exec(sql);

	qDebug()<< q.lastError()<<": " << sql ;

	sql =  "select max(seq_id) as max_seq_id from seq_tasks ";

	q.exec(sql);

	q.next();
	QSqlRecord rec = q.record();

	assert( rec.count() == 1 );

	int max_task_id = rec.value(0).toInt();

	//qDebug()<< q.lastError()<<": " << sql << max_task_id ;

	return ( max_task_id  );

}

QVector<QString> SqliteStorage::getCatsColumns()
{
	QVector<QString>  result ;
	QStringList ll;
	ll = QString(tr(this->catsModelColumnsOrderShow)).split(',');
	result.clear();
	for( int i = 0 ; i < ll.count() ; i ++ )
	{		
		result.append( ll.at(i).trimmed() );
	}
	//qDebug()<< this->catsModelColumnsOrderShow ;

	return result ;
}

QVector<QString> SqliteStorage::getTasksColumns()
{
	QVector<QString>  result ;

    QStringList ll;

	ll = QString(tr(this->tasksModelColumnsOrderShow)).split(',');
	result.clear();
	for( int i = 0 ; i < ll.count() ; i ++ )
	{		
		result.append( ll.at(i).trimmed() );
	}
	//qDebug()<< this->tasksModelColumnsOrderShow ;

	return result ;
}

QVector<QString> SqliteStorage::getSegmentsColumns()
{
	QVector<QString>  result ;

	QStringList ll ;
	ll = QString(tr(this->segsModelColumnsOrderShow)).split(',');
	result.clear();
	for( int i = 0 ; i < ll.count() ; i ++ )
	{		
		result.append( ll.at(i).trimmed() );
	}

	return result ;
}



QVector<QString> SqliteStorage::getInternalCatsColumns()
{
	QVector<QString>  result ;
	QStringList ll;
	ll = QString((this->catsModelColumnsOrderShow)).split(',');
	result.clear();
	for( int i = 0 ; i < ll.count() ; i ++ )
	{		
		result.append( ll.at(i).trimmed() );
	}
	//qDebug()<< this->catsModelColumnsOrderShow ;

	return result ;
}

QVector<QString> SqliteStorage::getInternalTasksColumns()
{
	QVector<QString>  result ;

	QStringList ll  ;

	ll = QString((this->tasksModelColumnsOrderShow)).split(',');
	result.clear();
	for( int i = 0 ; i < ll.count() ; i ++ )
	{		
		result.append( ll.at(i).trimmed() );
	}
	//qDebug()<< this->tasksModelColumnsOrderShow ;

	return result ;
}

QVector<QString> SqliteStorage::getInternalSegmentsColumns()
{
	QVector<QString>  result ;

	QStringList ll ;
	ll = QString((this->segsModelColumnsOrderShow)).split(',');
	result.clear();
	for( int i = 0 ; i < ll.count() ; i ++ )
	{		
		result.append( ll.at(i).trimmed() );
	}
	return result ;
}

bool SqliteStorage::containsTask(int task_id )
{
	

	QString sql = QString( "select  count(*) as task_count from tasks where task_id = '%1' " ).arg(task_id) ;
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	q.next();
	QSqlRecord rec = q.record();

	assert( rec.count() == 1 );

	qDebug()<< q.lastError()<<": " << sql << rec.value(0).toInt() ;

	if( rec.value(0).toInt() == 1 )
		return true;
	else if( rec.value(0).toInt() > 1 )
	{
		assert( 1==2);
	}
	else if( rec.value(0).toInt() == 0 )
	{
		return false ;
	}
	else
	{
		assert( 1==2);
	}
	qDebug()<< q.lastError()<<": " << sql  ;


	return false ;
}

bool SqliteStorage::containsSegment(int task_id, int seg_id)
{
	

	QString sql = QString( "select  count(*) as segment_count from segments where task_id = '%1' and seg_id='%2'" )
		.arg(task_id) .arg( seg_id) ;
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	q.next();
	QSqlRecord rec = q.record();

	assert( rec.count() == 1 );

	qDebug()<< q.lastError()<<": " << sql << rec.value(0).toInt() ;

	if( rec.value(0).toInt() == 1 )
		return true;
	else if( rec.value(0).toInt() > 1 )
	{
		assert( 1==2);
	}
	else if( rec.value(0).toInt() == 0 )
	{
		return false ;
	}
	else
	{
		assert( 1==2);
	}
	qDebug()<< q.lastError()<<": " << sql  ;


	return false ;
}

int SqliteStorage::getSubCatCountById(int cat_id)
{
	int result = 0 ;
	QString sql = QString( "select  count(*) as subcat_count from categorys where parent_cat_id = '%1'" )
		.arg(cat_id)  ;
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	q.next();
	QSqlRecord rec = q.record();

	assert( rec.count() == 1 );	

	result = rec.value(0).toInt();

	qDebug()<< q.lastError()<<": " << sql  ;
	return result ;
}
int SqliteStorage::getTotalFileCountById(int cat_id)
{
	int result = 0 ;
	QString sql = QString( "select  count(*) as file_count from tasks where cat_id = '%1'" )
		.arg(cat_id)  ;
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	q.next();
	QSqlRecord rec = q.record();

	assert( rec.count() == 1 );

	result = rec.value(0).toInt();

	qDebug()<< q.lastError()<<": " << sql  ;
	return result ;
}
int SqliteStorage::getDownloadedFileCountById( int cat_id )
{
	int result = 0 ;
	QString sql = QString( "select  count(*) as file_count from tasks where cat_id = '%1'"  )
		.arg(cat_id)  ;
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	q.next();
	QSqlRecord rec = q.record();

	assert( rec.count() == 1 );

	result = rec.value(0).toInt();

	qDebug()<< q.lastError()<<": " << sql  ;
	return result ;
}
long SqliteStorage::getTotalDownloadedLength( int cat_id)
{
	long result = 0 ;
	QString sql = QString("SELECT file_size FROM tasks WHERE cat_id = '%1'").arg(cat_id);
	
	QSqlQuery q(this->mTasksDB);
	q.exec(sql);

	while (q.next()) {
		QSqlRecord rec = q.record();
		result += rec.value(0).toLongLong();
	}
	
	qDebug()<< q.lastError()<<": " << sql  ;
	return result ;
}

quint64 SqliteStorage::getFileSizeById( int task_id)
{
	quint64 result = 0 ;
	QString sql = QString( "select file_size as file_count from tasks where task_id = '%1'"  )
		.arg(task_id)  ;
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	q.next();
	QSqlRecord rec = q.record();

	//assert( rec.count() == 1 );

	if( rec.count() == 1 )
	{
		result = rec.value(0).toULongLong() ;
	}
	else
	{
		result = (quint64)(-1);
	}

	qDebug()<< q.lastError()<<": " << sql  ;
	return result ;
}

QString SqliteStorage::getSavePathByCatId( int cat_id)
{
	QString path ;
	
	QString sql = QString( "select path from categorys where cat_id = '%1'"  )
		.arg(cat_id)  ;
	
	QSqlQuery q ( this->mTasksDB );
	q.exec(sql );

	q.next();
	QSqlRecord rec = q.record();

	//assert( rec.count() == 1 );

	if( rec.count() == 1 )
	{
		path = rec.value(0).toString();
	}
	else
	{
		//result = (quint64)(-1);
		assert( 1 == 2 );
	}

	//qDebug()<< q.lastError()<<": " << sql  ;

	return path ;
}


