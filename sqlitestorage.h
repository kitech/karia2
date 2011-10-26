// sqlitestorage.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 19:36:20 +0800
// Version: $Id$
// 

#ifndef SQLITESTORAGE_H
#define SQLITESTORAGE_H

#include <cassert>
#include <QtCore>
#include <QtGui>
#include <QVector>
#include <QVariant>
#include <QAbstractListModel>
#include <QStandardItemModel>
#include <QPixmap>
#include <QIcon>
#include <QTextStream>
#include <QtXml>
#include <QDomImplementation>
#include <QDomDocumentType>
#include <QDomDocument>
#include <QDomComment>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomElement>
#include <QDomText>
#include <QDomNamedNodeMap>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMap>
#include <QList>
#include <QDateTime>


//////////////////////
#include <QDomNode>
#include <QHash>


#include <QtSql>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include "abstractstorage.h"

namespace ng
{
	namespace tasks
	{
        // todo, bt-select-file and cookie field, drop no used 
		enum {task_id, file_size, retry_times, create_time, current_speed, average_speed,
              eclapsed_time, abtained_length, left_length, split_count, block_activity, total_block_count,
              active_block_count, user_cat_id, comment, sys_cat_id, save_path, file_name, select_file,
              abtained_percent,
              org_url, real_url, referer, redirect_times, finish_time, task_status, total_packet,
              abtained_packet, left_packet, total_timestamp, abtained_timestamp, left_timestamp,
              file_length_abtained, dirty, aria_gid
        };
	};
	namespace cats
	{
		enum {cat_root = 0, downloading = 1, downloaded = 2, software = 3, 
              game = 4, music = 5, movie = 6, deleted= 7, documents = 8};
		enum {display_name = 0, path, cat_id, parent_cat_id, can_child, raw_name, folder, delete_flag, create_time, dirty};
	};
	namespace options
	{

	};
	namespace segs
	{
		enum {seg_id, task_id, start_offset, create_time, finish_time, total_length,
              abtained_length, current_speed, average_speed,
              abtained_percent, segment_status, total_packet, abtained_packet, left_packet, total_timestamp,
              finish_timestamp, left_timestamp, dirty};
	};
	namespace logs
	{
		enum {log_type, add_time, log_content, task_id, seg_id, dirty};

		//输出到日志窗口的信息类型
		enum {DEBUG_MSG, UP_MSG, DOWN_MSG, INFO_MSG, USER_MSG, ERROR_MSG};
		//	toggle_log.png	up.png down.png messagebox_info.png presence_online.png messagebox_critical.png
	};	
};

/**
	这一模型结构设计为　cat_id,task_id,segment_id必须是各自唯一的，并且都是全局唯一的。
	并且每个task都必须属于一个cat，每个segment都必须属于一个task，不能存在没有归属的记录。

*/
class SqliteStorage : public AbstractStorage
{
	Q_OBJECT;
public:
	SqliteStorage(QObject *parent = 0);
	~SqliteStorage();

	static SqliteStorage *instance(QObject *parent = 0);


	virtual bool open();
	virtual bool close();
	//bool isOpened();

    virtual bool transaction();
    virtual bool commit();
    virtual bool rollback();

	bool initDefaultOptions();
	bool initDefaultTasks();
	
	bool dumpDefaultOptions();
	bool dumpDefaultTasks();

    bool addDefaultOption(QString key, QString value, QString type);
	bool addUserOption(QString key , QString value , QString type);
	bool deleteUserOption( QString key );
    QString getDefaultOption(QString key);
    QString getUserOption(QString key);
    QVector<QPair<QString, QString> > getUserOptionsByType(QString type); // for proxy

    bool addTask(QHash<QString, QString> taskHash);  // new 
	bool addTask(int task_id ,		
                 QString file_size            , 
                 QString retry_times          ,
                 QString create_time          ,
                 QString current_speed        ,
                 QString average_speed        ,
                 QString eclapsed_time        ,
                 QString abtained_length      ,
                 QString left_length          ,
                 QString split_count,
                 QString block_number         ,
                 QString total_block_count    ,
                 QString active_block_count   ,
                 QString user_cat_id               ,
                 QString comment              ,
                 QString sys_cat_id         ,
                 QString save_path, 
                 QString file_name            ,
                 QString select_file,
                 QString got_percent          ,
                 QString org_url              ,
                 QString real_url             ,
                 QString referer              ,
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
                 );

    bool updateTask(QHash<QString, QString> taskHash); // new 
	bool updateTask(int task_id , 
                    QString file_size, QString retry_times, QString create_time, QString current_speed, QString average_speed, QString eclapsed_time, QString abtained_length, QString left_length, QString split_count, QString block_activity, QString total_block_count, QString active_block_count, QString user_cat_id, QString comment, QString sys_cat_id, QString save_path, QString file_name, QString select_file, QString abtained_percent, QString org_url, QString real_url, QString referer, QString redirect_times, QString finish_time, QString task_status, QString total_packet, QString abtained_packet, QString left_packet, QString total_timestamp, QString abtained_timestamp, QString left_timestamp, QString file_length_abtained, QString dirty, QString aria_gid
                    );
	bool addSegment(int seg_id, int task_id, 
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
                    ) ;
	bool updateSegment(int seg_id , int task_id ,
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
                       ) ;
	bool addCategory(int cat_id, QString display_name, QString raw_name, QString folder, QString path, 
                     QString can_child, QString parent_cat_id, QString create_time, QString delete_flag);
	bool deleteTask(int task_id);
	bool deleteSegment(int task_id, int seg_id);
	bool deleteCategory(int cat_id, bool deleteChild);

	QString getSavePathByCatId(int cat_id);

	QVector<QSqlRecord> getCatSet();

	QVector<QSqlRecord> getTaskSet(int cat_id);
	QVector<QSqlRecord> getSementSet(int task_id);

	QVector<QString> getCatsColumns();
	QVector<QString> getTasksColumns();
	QVector<QString> getSegmentsColumns();

	QVector<QString> getInternalCatsColumns();
	QVector<QString> getInternalTasksColumns();
	QVector<QString> getInternalSegmentsColumns();

	int getNextValidTaskID();
	bool containsTask(int task_id );
	bool containsSegment( int task_id , int seg_id );

	int getSubCatCountById(int cat_id);
	int getTotalFileCountById(int cat_id);
	int getDownloadedFileCountById( int cat_id );
	long getTotalDownloadedLength( int cat_id);

	quint64 getFileSizeById( int task_id);

private slots: 
    void onTaskDBNotification(const QString &name);
    void onOptionDBNotification(const QString &name);

private:
   static SqliteStorage *mHandle;

	QSqlDatabase  mOptionsDB;
	QSqlDatabase  mTasksDB;
	//QSqlDatabase  mLogsDB;
	//QSqlDatabase  mMirrorsDB;

	QString optInsSql ;
	QString catInsSql ;
	QString segInsSql ;	
	QString taskInsSql ;

	QString tasksModelColumnsOrder  ;
	QString catsModelColumnsOrder ;
	QString segsModelColumnsOrder ;

    char *tasksModelColumnsOrderShow;
    char *catsModelColumnsOrderShow;
    char *segsModelColumnsOrderShow;

};

//


#endif // SQLITESTORAGE_H

