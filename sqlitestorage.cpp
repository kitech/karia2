// sqlitestorage.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 19:36:28 +0800
// Version: $Id: sqlitestorage.cpp 195 2013-01-29 01:42:42Z drswinghead $
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
// #include <QUrlInfo>
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

#include <boost/bind.hpp>

#include "simplelog.h"
#include "asyncdatabase.h"

#include "sqlitestorage.h"

//static 
SqliteStorage * SqliteStorage::mHandle = 0;
SqliteStorage * SqliteStorage::instance(QObject *parent)
{
	if (SqliteStorage::mHandle == 0) {
		//SqliteStorage::mHandle = new SqliteStorage(parent);
		SqliteStorage::mHandle = new SqliteStorage(0);
	}
	return SqliteStorage::mHandle;
}


SqliteStorage::SqliteStorage(QObject *parent)
	: AbstractStorage(parent)
{
	this->dbSuffix = ".ndb";
	this->optionDBName = this->storePath + this->optionsPrefixName + this->dbSuffix;
	this->taskDBName  = this->storePath + this->tasksPrefixName + this->dbSuffix;

	optInsSql = "INSERT INTO %1 (option_name, option_value, option_type, dirty) "
		" VALUES ('%2', '%3', '%4', '%5')";

	catInsSql = "INSERT INTO categorys (cat_id, display_name, raw_name, folder, path, can_child, parent_cat_id, create_time , delete_flag, dirty) "
		" VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', '%10')";

	segInsSql = "INSERT INTO segments (seg_id, task_id, start_offset, create_time, finish_time, total_length, abtained_length, current_speed, average_speed, abtained_percent, segment_status, total_packet, abtained_packet, left_packet, total_timestamp, finish_timestamp, left_timestamp, dirty) "
		" VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', '%10', '%11', '%12', '%13', '%14', '%15', '%16', '%17', '%18')";
	//19
	//taskInsSql = "INSERT INTO tasks (task_id, file_size, retry_times, create_time, current_speed, average_speed, eclapsed_time, abtained_length, left_length, split_count, block_activity, total_block_count, active_block_count, user_cat_id, comment, sys_cat_id, save_path, file_name, abtained_percent, org_url, real_url, referer, redirect_times, finish_time, task_status, total_packet, abtained_packet, left_packet, total_timestamp, abtained_timestamp, left_timestamp, file_length_abtained, dirty, aria_gid)"
    // "values ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', '%10', '%11', '%12', '%13', '%14', '%15', '%16', '%17', '%18', '%19', '%20', '%21', '%22', '%23', '%24', '%25', '%26', '%27', '%28', '%29', '%30', '%31', '%32', '%33', '%34')";
    taskInsSql = "INSERT INTO tasks (task_id, file_size, retry_times, create_time, current_speed, average_speed, eclapsed_time, abtained_length, left_length, split_count, block_activity, total_block_count, active_block_count, user_cat_id, comment, sys_cat_id, save_path, file_name, select_file, abtained_percent, org_url, real_url, referer, redirect_times, finish_time, task_status, total_packet, abtained_packet, left_packet, total_timestamp, abtained_timestamp, left_timestamp, file_length_abtained, dirty, aria_gid) VALUES (__VALUES_PLEACE__)";

    // this is from AbstractStorage.cpp
    // Marks the string literal sourceText for dynamic translation in the current context (class),
    tasksModelColumnsOrderShow = QT_TR_NOOP("task_id, file_size, retry_times, create_time, current_speed, average_speed, eclapsed_time, abtained_length, left_length, split_count, block_activity, total_block_count, active_block_count, user_cat_id, comment, sys_cat_id, save_path, file_name, select_file, abtained_percent, org_url, real_url, referer, redirect_times, finish_time, task_status, total_packet, abtained_packet,left_packet, total_timestamp, abtained_timestamp, left_timestamp, file_length_abtained, dirty, aria_gid");
	this->catsModelColumnsOrderShow = QT_TR_NOOP("display_name, path, cat_id, parent_cat_id, can_child, raw_name, folder, delete_flag, create_time, dirty");
    segsModelColumnsOrderShow = QT_TR_NOOP("seg_id, task_id, start_offset, create_time, finish_time, total_length, abtained_length, current_speed, average_speed, abtained_percent, segment_status, total_packet, abtained_packet, left_packet, total_timestamp, finish_timestamp, left_timestamp, dirty");

	// this->tasksModelColumnsOrder = tasksModelColumnsOrderShow ;
	// this->catsModelColumnsOrder = catsModelColumnsOrderShow;
	// this->segsModelColumnsOrder = segsModelColumnsOrderShow; 

    
    this->tasksModelColumnsOrder = this->mTaskColumnStr;
    this->catsModelColumnsOrder = this->mCatColumnStr;
    this->segsModelColumnsOrder = this->mSegColumnStr;

    this->m_adb.reset(new AsyncDatabase());
    QObject::connect(this->m_adb.get(), SIGNAL(ready(bool)), this, SLOT(onAdbStarted(bool)));
}

SqliteStorage::~SqliteStorage()
{
}

bool SqliteStorage::open()
{
    QMap<QString, QString> createSqls; // table_name -> create table
    QHash<QString, QStringList> cinitSqls; // table_name -> insert after create table
    QString tname, tcsql;
    QStringList cisqls;

    //// t1
    tname = "default_options";
    tcsql = "CREATE TABLE default_options(option_name VARCHAR(32) PRIMARY KEY, option_value VARCHAR(64), option_type VARCHAR(32), dirty VARCHAR(8) DEFAULT 'false')";

    QString insSql  = this->optInsSql;
    QMap<QString, QString>::const_iterator mit;

    for (mit = this->defaultOptions.begin() ; mit != this->defaultOptions.end() ; mit ++) {
        QString sql = insSql.arg("default_options").arg(mit.key()).arg(mit.value()).arg("string").arg("false");
        //qLogx()<< sql ;
       cisqls.append(sql);
    }
    createSqls.insert(tname, tcsql);
    cinitSqls.insert(tname, cisqls);

    //// t2
    tname = "user_options";
    tcsql = "CREATE TABLE user_options(option_name VARCHAR(32) PRIMARY KEY, option_value VARCHAR(64), option_type VARCHAR(32), dirty VARCHAR(8) DEFAULT 'false')";
    createSqls.insert(tname, tcsql);

    //// t3
    this->initDefaultTasks(createSqls, cinitSqls);

    this->m_adb->setInitSqls(createSqls, cinitSqls);
    if (!this->m_adb->isRunning()) {
        this->m_adb->start();
    }
	//QSqlDatabase::addDatabase("QSQLITE");

//    QSqlDriver *drv = NULL;
//	if (! QSqlDatabase::contains(this->optionsPrefixName)) {
//		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", this->optionsPrefixName);
//        drv = db.driver();
//        QObject::connect(drv, SIGNAL(notification(const QString&)),
//                         this, SLOT(onOptionDBNotification(const QString&)));
//    }
//	if (! QSqlDatabase::contains(this->tasksPrefixName)) {
//		QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", this->tasksPrefixName);
//        drv = db.driver();
//        QObject::connect(drv, SIGNAL(notification(const QString&)),
//                         this, SLOT(onTaskDBNotification(const QString&)));
//    }
//	//QSqlDatabase::addDatabase("QSQLITE",this->logsPrefixName);
//	//QSqlDatabase::addDatabase("QSQLITE",this->mirrorsPrefixName);

//	this->mOptionsDB = QSqlDatabase::database(this->optionsPrefixName, false);
//	this->mTasksDB = QSqlDatabase::database(this->tasksPrefixName, false);
//	//this->mLogsDB = QSqlDatabase::database(this->logsPrefixName);
//	//this->mMirrorsDB = QSqlDatabase::database(this->mirrorsPrefixName);

//	this->mOptionsDB.setDatabaseName(this->optionDBName);
//	this->mTasksDB.setDatabaseName(this->taskDBName);
//	qLogx()<< this->optionDBName ;
//	qLogx()<< this->taskDBName ;

//	if (QFile::exists(this->optionDBName)) {
//		this->mOptionsDB.open();
//		//this->initDefaultOptions();
//		//dumpDefaultOptions () ;
//	} else {
//		this->mOptionsDB.open();
//		if (this->mOptionsDB.isValid()) {
//			this->mOptionsDB.close();
//			QFile::remove(this->optionDBName);
//			this->mOptionsDB.open();
//			this->initDefaultOptions();
//		}
//	}

//	if (QFile::exists(this->taskDBName)) {
//		this->mTasksDB.open();
//		//this->dumpDefaultTasks();
//	} else {
//		this->mTasksDB.open();
//		if (this->mTasksDB.isValid()) {
//			this->mTasksDB.close();
//			QFile::remove(this->optionDBName);
//			this->mTasksDB.open();
//			this->initDefaultTasks();
//		}
//	}

	return true;
}

bool SqliteStorage::close()
{
//	if (this->mOptionsDB.isValid() && this->mOptionsDB.isOpen())
//		this->mOptionsDB.close();
//	if (this->mTasksDB.isValid() && this->mTasksDB.isOpen())
//		this->mTasksDB.close();
	//this->mLogsDB.close();
	//this->mMirrorsDB.close();
	return true;
}

bool SqliteStorage::isOpened()
{
    return this->m_adb->isConnected();
}

bool SqliteStorage::transaction()
{
//    this->mTasksDB.transaction();
    return true;
}

bool SqliteStorage::commit()
{
//    this->mTasksDB.commit();
    return true;
}

bool SqliteStorage::rollback()
{
//    this->mTasksDB.rollback();
    return true;
}

bool SqliteStorage::initDefaultOptions () 
{
//	char *createSql = "CREATE TABLE default_options(option_name VARCHAR(32) PRIMARY KEY, option_value VARCHAR(64), option_type VARCHAR(32), dirty VARCHAR(8) DEFAULT 'false')";

//    this->mOptionsDB.transaction();

//	QSqlQuery q(this->mOptionsDB);
//	q.exec(createSql);

//	QString insSql  = this->optInsSql;
//	QMap<QString, QString>::const_iterator mit;

//	for (mit = this->defaultOptions.begin() ; mit != this->defaultOptions.end() ; mit ++) {
//		QString sql = insSql.arg("default_options").arg(mit.key()).arg(mit.value()).arg("string").arg("false");
//		//qLogx()<< sql ;
//		q.exec(sql);
//	}

//	createSql = "CREATE TABLE user_options(option_name VARCHAR(32) PRIMARY KEY, option_value VARCHAR(64), option_type VARCHAR(32), dirty VARCHAR(8) DEFAULT 'false')";
//	q.exec(createSql);

//    this->mOptionsDB.commit();

	return true;
}

bool SqliteStorage::dumpDefaultOptions ()
{
	QString sql = "SELECT * FROM default_options";

//	QSqlQuery q(this->mOptionsDB);
//	q.exec(sql);

//	while (q.next()) {
//		QSqlRecord rec = q.record();
//		//qLogx()<< " field Count : "<< rec.count() ;
//		//for( int  i = 0 ; i < rec.count() ; i ++)
//		{
//			qLogx()<< rec.fieldName(0)<<"="<< rec.value(0) << " ,"
//                    << rec.fieldName(1)<<"="<< rec.value(1) << " ,"
//                    << rec.fieldName(2)<<"="<< rec.value(2) ;
//		}
//	}

    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::dumpDefaultOptionsDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

	return true;
}

bool SqliteStorage::dumpDefaultOptionsDone(boost::shared_ptr<SqlRequest> req)
{
    qLogx()<<req->mRet;

    this->mRequests.remove(req->mReqno);
    return true;
}

bool SqliteStorage::initDefaultTasks(QMap<QString, QString> &createSqls, QHash<QString, QStringList> &cinitSqls)
{
	QString createSql = "CREATE TABLE tasks (%1)"; 
	
	QStringList sl = QString(this->mTaskColumnStr).split(",");
	QString tempSql;
	for (int i = 0 ; i < sl.count() ; i ++) {
		if (sl.at(i).trimmed().endsWith("_id")) {
			tempSql += sl.at(i).trimmed() + " INTEGER ";
		} else {
			tempSql += sl.at(i).trimmed() + " VARCHAR(255) ";
		}
		if (sl.at(i).trimmed() == "task_id") {
			tempSql += " PRIMARY KEY ";
		}
		if (i < sl.count() - 1) tempSql += " , ";
	}
	createSql = createSql.arg(tempSql);
    qLogx()<<createSql;
    createSqls.insert("tasks", createSql);

//	QSqlQuery q( this->mTasksDB );
    // 
//    this->mTasksDB.transaction();

//	q.exec(createSql);

	createSql = "CREATE TABLE segments (%1, PRIMARY KEY (seg_id, task_id))";
	sl = QString(this->mSegColumnStr).split(",");
	tempSql = "";
	for (int i = 0 ; i < sl.count() ; i ++) {
		if (sl.at(i).trimmed().endsWith("_id")) {
			tempSql += sl.at(i).trimmed() + " INTEGER ";
		} else {
			tempSql += sl.at(i).trimmed() + " VARCHAR(255) ";
		}
		if (i < sl.count() - 1) tempSql += " , ";
	}
	createSql = createSql.arg(tempSql);
//	q.exec(createSql);
    qLogx()<<createSql;
    createSqls.insert("segments", createSql);

	createSql = "CREATE TABLE categorys (%1) ";
	sl = QString(this->mCatColumnStr).split(",");
	tempSql = "";
	for (int i = 0 ; i < sl.count() ; i ++) {
		if (sl.at(i).trimmed().endsWith("_id")) {
			tempSql += sl.at(i).trimmed() + " INTEGER ";
		} else {
			tempSql += sl.at(i).trimmed() + " VARCHAR(255) ";
		}
		if (sl.at(i).trimmed() == "cat_id") {
			tempSql += " PRIMARY KEY ";
		}
		if (i < sl.count() - 1) tempSql += " , ";
	}
	createSql = createSql.arg(tempSql);
//	q.exec(createSql);
    qLogx()<<createSql;
    createSqls.insert("categorys", createSql);

	QString insSql = this->catInsSql;
    QStringList insSqls;
	for (int cnt = 0 ; cnt < this->defaultCategorys.count() ;  cnt ++) {
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
        insSqls.append(realSql);
//		q.exec(realSql);
	}
    cinitSqls.insert("categorys", insSqls);

	createSql = "CREATE TABLE seq_tasks (seq_id INTEGER PRIMARY KEY AUTOINCREMENT, seq_null INTEGER)";
    createSqls.insert("seq_tasks", createSql);
//	q.exec(createSql);

//    this->mTasksDB.commit();

	return true ;
}

bool SqliteStorage::dumpDefaultTasks()
{
	QString sql = "SELECT * FROM categorys ";
	
//	QSqlQuery q(this->mTasksDB);
//	q.exec(sql);
//	while (q.next()) {
//		QSqlRecord rec = q.record();
//		//qLogx()<< " field Count : "<< rec.count() ;
//		for (int  i = 0 ; i < rec.count() ; i ++) {
//			qLogx()<< rec.fieldName(i)<<"="<< rec.value(i);
//		}
//	}
//	qLogx()<< q.lastError()<<": " << sql ;

    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::dumpDefaultTasksDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;


	return true ;
}

bool SqliteStorage::dumpDefaultTasksDone(boost::shared_ptr<SqlRequest> req)
{
    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

bool SqliteStorage::addDefaultOption(QString key, QString value, QString type)
{
	QString sql = this->optInsSql.arg("default_options").arg(key).arg(value).arg(type).arg("false");
	
//	QSqlQuery q(this->mOptionsDB);
//	q.exec(sql);

//	qLogx()<<__FUNCTION__<< q.lastError()<<": " << sql;

    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::dumpDefaultTasksDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

	return true;
}

bool SqliteStorage::addDefaultOptionDone(boost::shared_ptr<SqlRequest> req)
{
    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

bool SqliteStorage::addUserOption(QString key, QString value, QString type)
{
	QString sql = this->optInsSql.arg("user_options").arg(key).arg(value).arg(type).arg("false");
	
//	QSqlQuery q ( this->mOptionsDB);
//	q.exec(sql );

//	qLogx()<<__FUNCTION__<< q.lastError()<<": " << sql ;

    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::addUserOptionDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

	return true ;
}

bool SqliteStorage::addUserOptionDone(boost::shared_ptr<SqlRequest> req)
{
    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

bool SqliteStorage::deleteUserOption( QString key )
{
	QString sql = QString("DELETE FROM user_options WHERE option_name='%1'").arg(key);

//	QSqlQuery q ( this->mOptionsDB);
//	q.exec(sql );
	
//	qLogx()<< q.lastError()<<": " << sql ;

    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::deleteUserOptionDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

	return true ;
}

bool SqliteStorage::deleteUserOptionDone(boost::shared_ptr<SqlRequest> req)
{
    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

QString SqliteStorage::getDefaultOption(QString key)
{
    QString sql = QString("SELECT option_value FROM default_options WHERE option_name='%1'").arg(key);
    QString ov = QString::null;

//    QSqlQuery q(this->mOptionsDB);
//    q.exec(sql);
//	if (q.next()) {
//        QSqlRecord rec = q.record();

//        // qLogx()<<__FUNCTION__<<rec;
//        //assert( rec.count() == 1 );
//        if (rec.count() == 1) {
//            ov = rec.value(0).toString();
//        } else {
//            //result = (quint64)(-1);
//            // assert( 1 == 2 );
//        }
//    } else {
//        // qLogx()<<__FUNCTION__<<"no result set";
//    }
    // qLogx()<< __FUNCTION__<<q.lastError()<<": " << sql  ;

	return ov;
}

QString SqliteStorage::getUserOption(QString key)
{
    QString sql = QString("SELECT option_value FROM user_options WHERE option_name='%1'").arg(key);
    QString ov = QString::null;

//    QSqlQuery q(this->mOptionsDB);
//    q.exec(sql);
//	if (q.next()) {
//        QSqlRecord rec = q.record();

//        // qLogx()<<__FUNCTION__<<rec;
//        //assert( rec.count() == 1 );
//        if (rec.count() == 1) {
//            ov = rec.value(0).toString();
//        } else {
//            //result = (quint64)(-1);
//            // assert( 1 == 2 );
//        }
//    } else {
//        // qLogx()<<__FUNCTION__<<"no result set";
//    }

    // qLogx()<< __FUNCTION__<<q.lastError()<<": " << sql  ;

	return ov;   
}

QVector<QPair<QString, QString> > SqliteStorage::getUserOptionsByType(QString type)
{
    QVector<QPair<QString, QString> > options;

    QString sql = QString("SELECT option_name, option_value FROM user_options WHERE option_type='%1'").arg(type);

//    QSqlQuery query(this->mOptionsDB);
//    query.exec(sql);
//    while (query.next()) {
//        QSqlRecord rec = query.record();
//        options.append(QPair<QString, QString>(rec.value(0).toString(), rec.value(1).toString()));
//    }
    return options;
}

bool SqliteStorage::addTask(QHash<QString, QString> taskHash)
{

    return true;
}

bool SqliteStorage::addTask(int task_id , 
                            QString file_size            , 
                            QString retry_times          ,
                            QString create_time          ,
                            QString current_speed        ,
                            QString average_speed        ,
                            QString eclapsed_time        ,
                            QString abtained_length      ,
                            QString left_length          ,
                            QString split_count,
                            QString block_activity         ,
                            QString total_block_count    ,
                            QString active_block_count   ,
                            QString user_cat_id               ,
                            QString comment              ,
                            QString sys_cat_id         ,
                            QString save_path,
                            QString file_name            ,
                            QString select_file,
                            QString abtained_percent          ,
                            QString org_url              ,
                            QString real_url             ,
                            QString referer ,
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
    // TODO escape issue
    QStringList values;
    values << QString::number(task_id)<< file_size<< retry_times<< create_time<< current_speed
           << average_speed<< eclapsed_time<< abtained_length<< left_length
           << split_count<< block_activity<< total_block_count<< active_block_count<< user_cat_id
           << comment<< sys_cat_id<< save_path<< file_name<<select_file<< abtained_percent<< org_url
           << real_url<< referer
           << redirect_times<< finish_time<< task_status<< total_packet<< abtained_packet
           << left_packet<< total_timestamp<< abtained_timestamp<< left_timestamp
           << file_length_abtained <<"false" <<"0";
    // qLogx()<<"values count:"<<values.count();
    QString valueStr = values.join("','");
    valueStr = "'" + valueStr + "'";
    
    QString sql = this->taskInsSql;
    sql.replace("__VALUES_PLEACE__", valueStr);

//    this->mTasksDB.transaction();
//	QSqlQuery q (this->mTasksDB);
//	bool eok = q.exec(sql );
//    this->mTasksDB.commit();

//	qLogx()<<__FUNCTION__<<eok<<q.lastError()<<": " << sql ;

    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::addTaskDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

	return true ;
}

bool SqliteStorage::addTaskDone(boost::shared_ptr<SqlRequest> req)
{

    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

bool SqliteStorage::updateTask(QHash<QString, QString> taskHash)
{
    return true;
}

// TODO escape issue
bool SqliteStorage::updateTask(int task_id,
                               QString file_size, QString retry_times, QString create_time, QString current_speed, QString average_speed, QString eclapsed_time, QString abtained_length, QString left_length, QString split_count, QString block_activity, QString total_block_count, QString active_block_count, QString user_cat_id, QString comment, QString sys_cat_id, QString save_path, QString file_name, QString select_file, QString abtained_percent, QString org_url, QString real_url, QString referer, QString redirect_times, QString finish_time, QString task_status, QString total_packet, QString abtained_packet, QString left_packet, QString total_timestamp, QString abtained_timestamp, QString left_timestamp, QString file_length_abtained, QString dirty, QString aria_gid
							   )
{
	// QString sql = "UPDATE tasks SET file_size='%1',retry_times='%2',create_time='%3',current_speed='%4',average_speed='%5',eclapsed_time='%6',abtained_length='%7',left_length='%8', block_activity='%9',total_block_count='%10',active_block_count='%11',user_cat_id='%12',comment='%13', sys_cat_id='%14',file_name='%15',abtained_percent='%16',org_url='%17',real_url='%18', referer='%19', redirect_times='%20',finish_time='%21',task_status='%22',total_packet='%23',abtained_packet='%24',left_packet='%25',total_timestamp='%26',abtained_timestamp='%27',left_timestamp='%28',file_length_abtained='%29',dirty='%30', aria_gid='%31' WHERE task_id='%32' ";

    QString sql = "UPDATE tasks SET ";
    sql += "file_size='" + file_size + "',";                                         
    sql += "retry_times='" + retry_times + "',";
    sql += "create_time='" + create_time + "',";
    sql += "current_speed='" + current_speed + "',";
    sql += "average_speed='" + average_speed + "',";
    sql += "eclapsed_time='" + eclapsed_time + "',";
    sql += "abtained_length='" + abtained_length + "',";
    sql += "left_length='" + left_length + "',";
    sql += "split_count='" + split_count + "',";
    sql += "block_activity='" + block_activity + "',";
    sql += "total_block_count='" + total_block_count + "',";
    sql += "active_block_count='" + active_block_count + "',";
    sql += "user_cat_id='" + user_cat_id + "',";
    sql += "comment='" + comment + "',";
    sql += "sys_cat_id='" + sys_cat_id + "',";
    sql += "save_path='" + save_path + "',";
    sql += "file_name='" + file_name + "',";
    sql += "select_file='" + select_file + "',";
    sql += "abtained_percent='" + abtained_percent + "',";
    sql += "org_url='" + org_url + "',";
    sql += "real_url='" + real_url + "',";
    sql += "referer='" + referer + "',";
    sql += "redirect_times='" + redirect_times + "',";
    sql += "finish_time='"   + finish_time + "',";
    sql += "task_status='" + task_status + "',";
    sql += "total_packet='" + total_packet + "',";
    sql += "abtained_packet='" + abtained_packet + "',";
    sql += "left_packet='" + left_packet + "',";
    sql += "total_timestamp='" + total_timestamp + "',";
    sql += "abtained_timestamp='" + abtained_timestamp + "',";
    sql += "left_timestamp='" + left_timestamp + "',";
    sql += "file_length_abtained='" + file_length_abtained + "',";
    sql += "dirty='false" + QString()+ "',";
    sql += "aria_gid='" + aria_gid + "'";
    sql += " WHERE task_id='" + QString::number(task_id) + "'";


	// sql = sql.arg(file_size).arg(retry_times).arg(create_time).arg(current_speed).arg(average_speed).arg(eclapsed_time).arg(abtained_length).arg(left_length).arg(block_activity).arg(total_block_count).arg(active_block_count).arg(user_cat_id).arg(comment).arg(sys_cat_id).arg(file_name).arg(abtained_percent).arg(org_url).arg(real_url).arg(referer).arg(redirect_times).arg(finish_time).arg(task_status).arg(total_packet).arg(abtained_packet).arg(left_packet).arg(total_timestamp).arg(abtained_timestamp).arg(left_timestamp).arg(file_length_abtained).arg("false").arg(aria_gid).arg(task_id) ;

//    this->mTasksDB.transaction();
//	QSqlQuery q(this->mTasksDB);
//	q.exec(sql );
//    this->mTasksDB.commit();

//	qLogx()<<__FUNCTION__<< q.lastError()<<": " << sql ;
    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::updateTaskDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

    return true ;
}

bool SqliteStorage::updateTaskDone(boost::shared_ptr<SqlRequest> req)
{

    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
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

//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

    //qLogx()<< q.lastError()<<": " << sql ;
    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::addSegmentDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

    return true ;
}

bool SqliteStorage::addSegmentDone(boost::shared_ptr<SqlRequest> req)
{

    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
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
	QString sql = QString("UPDATE segments SET start_offset = '%1' , create_time = '%2' , finish_time = '%3' , total_length = '%4' , abtained_length = '%5' , current_speed = '%6' , average_speed = '%7' , abtained_percent = '%8' , segment_status = '%9' , total_packet = '%10' , abtained_packet = '%11' , left_packet = '%12' , total_timestamp = '%13' , finish_timestamp = '%14' , left_timestamp = '%15' , dirty='%16' WHERE task_id='%17' AND seg_id='%18' ")
		.arg(start_offset).arg(create_time).arg(finish_time).arg(total_length).arg(abtained_length).arg(current_speed).arg(average_speed).arg(abtained_percent).arg(segment_status).arg(total_packet).arg(abtained_packet).arg(left_packet).arg(total_timestamp).arg(finish_timestamp).arg(left_timestamp).arg("fasle")
		.arg(task_id).arg(seg_id) ;

//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

    //qLogx()<< q.lastError()<<": " << sql ;
    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::updateSegmentDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

    return true ;
}

bool SqliteStorage::updateSegmentDone(boost::shared_ptr<SqlRequest> req)
{

    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

bool SqliteStorage::addCategory( int cat_id , QString display_name , QString raw_name , QString folder , 
								QString path , QString can_child , QString parent_cat_id , 
								QString create_time , QString delete_flag   ) 
{

	QString sql = this->catInsSql.arg(cat_id).arg(display_name).arg(raw_name).arg(folder)
		.arg(path).arg(can_child).arg(parent_cat_id).arg(create_time).arg(delete_flag).arg("false") ;

//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

//	qLogx()<< q.lastError()<<": " << sql ;
    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::addCategoryDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

    return true ;
}

bool SqliteStorage::addCategoryDone(boost::shared_ptr<SqlRequest> req)
{

    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

bool SqliteStorage::deleteTask( int task_id )
{
	QString sql = QString("DELETE FROM tasks WHERE task_id='%1' ").arg(task_id) ;

//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

    //qLogx()<< q.lastError()<<": " << sql ;
    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::deleteTaskDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

    return true ;
}

bool SqliteStorage::deleteTaskDone(boost::shared_ptr<SqlRequest> req)
{

    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

bool SqliteStorage::deleteSegment( int task_id ,  int seg_id )
{
	QString sql = QString("DELETE FROM segments WHERE task_id='%1' AND seg_id='%2' ").arg(task_id).arg(seg_id) ;

//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

    //qLogx()<< q.lastError()<<": " << sql ;
    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::deleteSegmentDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

    return true ;
}

bool SqliteStorage::deleteSegmentDone(boost::shared_ptr<SqlRequest> req)
{

    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

bool SqliteStorage::deleteCategory( int cat_id , bool deleteChild )
{
	QString sql = QString("DELETE FROM categorys WHERE cat_id='%1' ").arg(cat_id) ;
	
//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

    //qLogx()<< q.lastError()<<": " << sql ;
    boost::shared_ptr<SqlRequest> req(new SqlRequest());

    req->mCbFunctor = boost::bind(&SqliteStorage::deleteCategoryDone, this, _1);
    // req->mCbObject = this;
    // req->mCbSlot = SLOT(onAddContactDone(boost::shared_ptr<SqlRequest>));
    // req->mSql = QString("INSERT INTO kp_contacts (group_id,phone_number) VALUES (%1, '%2')")
    //     .arg(pc->mGroupId).arg(pc->mPhoneNumber);
    req->mSql = sql;
    req->mReqno = this->m_adb->execute(req->mSql);
    this->mRequests.insert(req->mReqno, req);

    qLogx()<<req->mSql;

    return true ;
}

bool SqliteStorage::deleteCategoryDone(boost::shared_ptr<SqlRequest> req)
{

    qLogx()<<req->mRet;
    this->mRequests.remove(req->mReqno);
    return true;
}

QVector<QSqlRecord> SqliteStorage::getCatSet()
{
	QVector<QSqlRecord> dataSet ;

	// 视图中显示的顺序与这里的顺序有直接的关系，可以说是一样的
	// 还有排序的问题。
	QString sql = "SELECT " + this->catsModelColumnsOrder+ " FROM categorys ORDER BY parent_cat_id, cat_id ";
	
//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

//	while (q.next()) {
//		QSqlRecord rec = q.record();
//		dataSet.append(rec);
//	}
    //qLogx()<< q.lastError()<<": " << sql ;

    QList<QSqlRecord> recs;
    this->m_adb->syncExecute(sql, recs);
    for (int i = 0; i < recs.count(); ++i) {
        dataSet.append(recs.at(i));
    }
	
	return dataSet ;
}

QVector<QSqlRecord> SqliteStorage::getTaskSet( int cat_id )
{
	QVector<QSqlRecord> dataSet ;

//	QSqlQuery q ( this->mTasksDB );

	//
	//视图中显示的顺序与这里的顺序有直接的关系，可以说是一样的
	//还有排序的问题。
	QString sql = QString("SELECT " + QString(this->tasksModelColumnsOrder) +" FROM tasks WHERE sys_cat_id='%1' OR sys_cat_id IN (SELECT cat_id FROM categorys WHERE parent_cat_id='%1') ORDER BY task_id DESC").arg(cat_id);
	
//	q.exec(sql );

//	while (q.next()) {
//		QSqlRecord rec = q.record();
//		dataSet.append(rec);
//	}
    //qLogx()<< q.lastError()<<": " << sql  ;

    int iret = this->m_adb->syncExecute(sql, dataSet);
	
	return dataSet ;
}
QVector<QSqlRecord> SqliteStorage::getSementSet( int task_id )
{
	QVector<QSqlRecord> dataSet ;

	//视图中显示的顺序与这里的顺序有直接的关系，可以说是一样的
	//还有排序的问题。
	QString sql = QString( "SELECT * FROM segments WHERE task_id = '%1' ORDER BY seg_id ASC " ) .arg(task_id) ;
	
//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

//	while (q.next()) {
//		QSqlRecord rec = q.record();
//		dataSet.append(rec);
//	}
    //qLogx()<< q.lastError()<<": " << sql ;
	
	return dataSet ;
}

int SqliteStorage::getNextValidTaskID()
{
	QVector<QSqlRecord> recs;
	QString sql = "INSERT INTO seq_tasks (seq_null) VALUES (0)";
//	QSqlQuery q( this->mTasksDB );
//	q.exec(sql);

//	qLogx()<< q.lastError()<<": " << sql ;
    this->m_adb->syncExecute(sql, recs);
    qLogx()<<recs.count()<<recs;

	sql = "SELECT MAX(seq_id) AS max_seq_id FROM seq_tasks ";
	// sql = "SELECT last_insert_rowid() AS max_seq_id FROM seq_tasks";
    //     this->m_adb->syncExecute(sql, recs);


//	q.exec(sql);

//	q.next();
//	QSqlRecord rec = q.record();

//	assert( rec.count() == 1 );

    int max_task_id = 0;
//	int max_task_id = rec.value(0).toInt();
    max_task_id = recs.at(0).value(0).toInt();

    //qLogx()<< q.lastError()<<": " << sql << max_task_id ;

	return (max_task_id);

}

QVector<QString> SqliteStorage::getCatsColumns()
{
	QVector<QString>  result ;
	QStringList ll;
	ll = QString(tr(this->catsModelColumnsOrderShow)).split(',');
	result.clear();
	for (int i = 0 ; i < ll.count() ; i ++) {		
		result.append( ll.at(i).trimmed());
	}
    //qLogx()<< this->catsModelColumnsOrderShow ;

	return result ;
}

QVector<QString> SqliteStorage::getTasksColumns()
{
	QVector<QString>  result ;

    QStringList ll;

	ll = QString(tr(this->tasksModelColumnsOrderShow)).split(',');
	result.clear();
	for (int i = 0 ; i < ll.count() ; i ++) {		
		result.append( ll.at(i).trimmed());
	}
    //qLogx()<< this->tasksModelColumnsOrderShow ;

	return result ;
}

QVector<QString> SqliteStorage::getSegmentsColumns()
{
	QVector<QString>  result ;

	QStringList ll ;
	ll = QString(tr(this->segsModelColumnsOrderShow)).split(',');
	result.clear();
	for (int i = 0 ; i < ll.count() ; i ++) {		
		result.append( ll.at(i).trimmed());
	}

	return result;
}



QVector<QString> SqliteStorage::getInternalCatsColumns()
{
	QVector<QString>  result ;
	QStringList ll;
	ll = QString((this->mCatColumnStr)).split(',');
	result.clear();
	for( int i = 0 ; i < ll.count() ; i ++ )
	{		
		result.append( ll.at(i).trimmed() );
	}
    //qLogx()<< this->catsModelColumnsOrderShow ;

	return result ;
}

QVector<QString> SqliteStorage::getInternalTasksColumns()
{
	QVector<QString>  result ;

	QStringList ll  ;

	ll = QString((this->tasksModelColumnsOrder)).split(',');
	result.clear();
	for( int i = 0 ; i < ll.count() ; i ++ )
	{		
		result.append( ll.at(i).trimmed() );
	}
    //qLogx()<< this->tasksModelColumnsOrderShow ;

	return result ;
}

QVector<QString> SqliteStorage::getInternalSegmentsColumns()
{
	QVector<QString>  result ;

	QStringList ll ;
	ll = QString((this->segsModelColumnsOrder)).split(',');
	result.clear();
	for (int i = 0 ; i < ll.count() ; i ++) {		
		result.append( ll.at(i).trimmed() );
	}
	return result ;
}

bool SqliteStorage::containsTask(int task_id)
{
	QString sql = QString("SELECT count(*) AS task_count FROM tasks WHERE task_id = '%1' " ).arg(task_id);
	
//	QSqlQuery q(this->mTasksDB);
//	q.exec(sql);

//	q.next();
//	QSqlRecord rec = q.record();

//	assert( rec.count() == 1 );

//	qLogx()<< q.lastError()<<": " << sql << rec.value(0).toInt();

//	if( rec.value(0).toInt() == 1 )
//		return true;
//	else if( rec.value(0).toInt() > 1 ) {
//		assert( 1==2);
//	} else if( rec.value(0).toInt() == 0 ) {
//		return false ;
//	} else {
//		assert( 1==2);
//	}
//	qLogx()<< q.lastError()<<": "<< sql;

	return false ;
}

bool SqliteStorage::containsSegment(int task_id, int seg_id)
{
	

	QString sql = QString("SELECT COUNT(*) AS segment_count FROM segments WHERE task_id = '%1' AND seg_id='%2'")
		.arg(task_id) .arg( seg_id) ;
	
//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

//	q.next();
//	QSqlRecord rec = q.record();

//	assert( rec.count() == 1 );

//	qLogx()<< q.lastError()<<": " << sql << rec.value(0).toInt() ;

//	if (rec.value(0).toInt() == 1)
//		return true;
//	else if (rec.value(0).toInt() > 1) {
//		assert( 1==2);
//	} else if (rec.value(0).toInt() == 0) {
//		return false;
//	} else {
//		assert( 1==2);
//	}
//	qLogx()<< q.lastError()<<": " << sql;


	return false;
}

int SqliteStorage::getSubCatCountById(int cat_id)
{
	int result = 0 ;
	QString sql = QString("SELECT COUNT(*) AS subcat_count FROM categorys WHERE parent_cat_id = '%1'" )
		.arg(cat_id);
	
//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

//	q.next();
//	QSqlRecord rec = q.record();

//	assert( rec.count() == 1 );

//	result = rec.value(0).toInt();

//	qLogx()<< q.lastError()<<": " << sql  ;
	return result ;
}
int SqliteStorage::getTotalFileCountById(int cat_id)
{
	int result = 0 ;
	QString sql = QString("SELECT COUNT(*) AS file_count FROM tasks WHERE sys_cat_id = '%1'" )
		.arg(cat_id)  ;
	
//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

//	q.next();
//	QSqlRecord rec = q.record();

//	assert( rec.count() == 1 );

//	result = rec.value(0).toInt();

//	qLogx()<< q.lastError()<<": " << sql  ;
	return result ;
}
int SqliteStorage::getDownloadedFileCountById( int cat_id )
{
	int result = 0 ;
	QString sql = QString("SELECT COUNT(*) AS file_count FROM tasks WHERE sys_cat_id = '%1'"  )
		.arg(cat_id)  ;
	
//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

//	q.next();
//	QSqlRecord rec = q.record();

//	assert( rec.count() == 1 );

//	result = rec.value(0).toInt();

//	qLogx()<< q.lastError()<<": " << sql  ;
	return result ;
}
long SqliteStorage::getTotalDownloadedLength( int cat_id)
{
	long result = 0 ;
	QString sql = QString("SELECT file_size FROM tasks WHERE sys_cat_id = '%1'").arg(cat_id);
	
//	QSqlQuery q(this->mTasksDB);
//	q.exec(sql);

//	while (q.next()) {
//		QSqlRecord rec = q.record();
//		result += rec.value(0).toLongLong();
//	}
	
//	qLogx()<< q.lastError()<<": " << sql  ;
	return result ;
}

quint64 SqliteStorage::getFileSizeById( int task_id)
{
	quint64 result = 0 ;
	QString sql = QString("SELECT file_size AS file_count FROM tasks WHERE task_id = '%1'"  )
		.arg(task_id)  ;
	
//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

//	q.next();
//	QSqlRecord rec = q.record();

//	//assert( rec.count() == 1 );

//	if (rec.count() == 1) {
//		result = rec.value(0).toULongLong() ;
//	} else {
//		result = (quint64)(-1);
//	}

//	qLogx()<< q.lastError()<<": " << sql;
	return result;
}

QString SqliteStorage::getSavePathByCatId( int cat_id)
{
	QString path ;
	
	QString sql = QString("SELECT path FROM categorys WHEre cat_id = '%1'"  )
		.arg(cat_id)  ;
	
//	QSqlQuery q ( this->mTasksDB );
//	q.exec(sql );

//	q.next();
//	QSqlRecord rec = q.record();

//	//assert( rec.count() == 1 );

//	if (rec.count() == 1) {
//		path = rec.value(0).toString();
//	} else {
//		//result = (quint64)(-1);
//		assert( 1 == 2 );
//	}

    //qLogx()<< q.lastError()<<": " << sql  ;

	return path;
}

void SqliteStorage::onTaskDBNotification(const QString &name)
{
    qLogx()<<__FUNCTION__<<name;
}

void SqliteStorage::onOptionDBNotification(const QString &name)
{
    qLogx()<<__FUNCTION__<<name;
}

void SqliteStorage::onAdbStarted(bool started)
{
    qLogx()<<started;
    if (started) {
        emit this->opened();
    }
}

void SqliteStorage::onSqlExecuteDone(const QList<QSqlRecord> & results, int reqno, bool eret, const QString &estr, const QVariant &eval)
{
    qLogx()<<__FILE__<<__LINE__<<__FUNCTION__<<reqno;

    QObject *cb_obj = NULL;
    const char *cb_slot = NULL;
    boost::function<bool(boost::shared_ptr<SqlRequest>)> cb_functor;
    boost::shared_ptr<SqlRequest> req;
    bool bret = false;
    // QGenericReturnArgument qret;
    // QGenericArgument qarg;
    bool qret;
    QMetaMethod qmethod;
    char raw_method_name[32] = {0};

    if (this->mRequests.contains(reqno)) {
        req = this->mRequests[reqno];
        req->mRet = eret;
        req->mErrorString = estr;
        req->mExtraValue = eval;
        req->mResults = results;

        // 实现方法太多，还要随机使用一种方法，找麻烦
        cb_functor = req->mCbFunctor;
        bret = cb_functor(req);
    }
}

