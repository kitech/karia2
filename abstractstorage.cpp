// abstractstorage.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-07 21:03:15 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QDir>
#include <QStringList>
#include "abstractstorage.h"

AbstractStorage::AbstractStorage(QObject *parent)
	: QThread(parent)
{
	
#ifdef WIN32	
	this->storePath = QCoreApplication::applicationDirPath() + "/data/";
#else	//其他像linux平台，该用户可能无法在程序的安装目录创建文件夹权限,所以在用户主目录下创建。

	//this->storePath = QCoreApplication::applicationDirPath() + "/data/";
	this->storePath = QDir::homePath() + "/.karia2/data/";

#endif

	if (!QDir(this->storePath).exists()) {
		QDir(this->storePath ).mkpath(this->storePath );//如果出错，怎么办呢。
	}

	this->dbSuffix = ".dat";
	this->optionsPrefixName = "options" ;
	this->tasksPrefixName = "tasks" ;

	///
	this->optionDBName = this->storePath + this->optionsPrefixName + this->dbSuffix  ;

	//option 对话框的变量默认值
	this->defaultOptions.insert("language","Chinese");
	this->defaultOptions.insert("skin","default");
	this->defaultOptions.insert("usebandwidthlimit","false");
	this->defaultOptions.insert("bandwidthlimittype","unlimited");
	this->defaultOptions.insert("rememberbandwidthsetting","true");
	this->defaultOptions.insert("maxbandwidth","10240");
	this->defaultOptions.insert("maxtaskcount","20");
	this->defaultOptions.insert("maxsegmentcount","20");
	this->defaultOptions.insert("maxconnectretrytimes","3");
	this->defaultOptions.insert("maxreadretrytimes","4");
	this->defaultOptions.insert("maxwriteretrytimes","5");
	this->defaultOptions.insert("retrydelay","3");
	this->defaultOptions.insert("randomretrydelay","false");
	this->defaultOptions.insert("cachebufferlength","204800");	//以K计
	this->defaultOptions.insert("socketconnecttimeout","5");	
	this->defaultOptions.insert("socketdatatimeout","5");
	this->defaultOptions.insert("minsegmentsize","256");

	//connection
	this->defaultOptions.insert("maxsimultaneousjobs" , "10" ) ;

#ifdef WIN32
    #define SAVE_PREFIX_STR "C:"
#else
	#define SAVE_PREFIX_STR "~"
#endif

	///////////////////////
	///////////////////////
	char *rawcat[256] = {
		"cat_id=0,display_name=NullGet,raw_name=NullGet,folder=no,path=,can_child=true,parent_cat_id=-1,create_time=,delete_flag=false,dirty=false"                       ,
		"cat_id=1,display_name=Download,raw_name=Download,folder=no,path="SAVE_PREFIX_STR"/NGDownload,can_child=true,parent_cat_id=0,create_time=,delete_flag=false,dirty=false"         ,
		"cat_id=2,display_name=Downloaded,raw_name=Downloaded,folder=no,path="SAVE_PREFIX_STR"/NGDownload,can_child=true,parent_cat_id=0,create_time=,delete_flag=false,dirty=false,dirty=false"         ,
		"cat_id=3,display_name=software,raw_name=software,folder=no,path="SAVE_PREFIX_STR"/NGDownload/software,can_child=true,parent_cat_id=2,create_time=,delete_flag=false,dirty=false",
		"cat_id=4,display_name=game,raw_name=game,folder=no,path="SAVE_PREFIX_STR"/NGDownload/game,can_child=true,parent_cat_id=2,create_time=,delete_flag=false,dirty=false"            ,
		"cat_id=5,display_name=music,raw_name=music,folder=no,path="SAVE_PREFIX_STR"/NGDownload/music,can_child=true,parent_cat_id=2,create_time=,delete_flag=false,dirty=false"         ,
		"cat_id=6,display_name=movie,raw_name=movie,folder=no,path="SAVE_PREFIX_STR"/NGDownload/movie,can_child=true,parent_cat_id=2,create_time=,delete_flag=false,dirty=false"    ,
		"cat_id=8,display_name=documents,raw_name=documents,folder=no,path="SAVE_PREFIX_STR"/NGDownload/documents,can_child=true,parent_cat_id=2,create_time=,delete_flag=false,dirty=false"   ,
		"cat_id=7,display_name=deleted,raw_name=deleted,folder=no,path="SAVE_PREFIX_STR"/NGDownload/deleted,can_child=true,parent_cat_id=0,create_time=,delete_flag=false,dirty=false" 
	};
	
	for (int line = 0 ; line < 9; line ++) {
		QStringList cat = QString(rawcat[line]).split(",");
		QMap<QString , QString> catmap ;
		for (int fieldCount = 0 ; fieldCount < cat.size() ; fieldCount ++) {
			QStringList kv = cat.at(fieldCount).split("=");
			catmap.insert(kv.at(0), kv.at(1));
		}
		this->defaultCategorys.append(catmap);
	}
	
	this->mTaskColumnStr =  
			"task_id             ,"
			"file_size           ,"
			"retry_times         ,"
			"create_time         ,"
			"current_speed       ,"
			"average_speed       ,"
			"eclapsed_time       ,"
			"abtained_length     ,"
			"left_length         ,"
        "split_count,"
			"block_activity      ,"
			"total_block_count   ,"
			"active_block_count  ,"
			"user_cat_id              ,"
			"comment             ,"
			"sys_cat_id        ,"
        "save_path,"
			"file_name           ,"
        "select_file,"
			"abtained_percent    ,"
			"org_url             ,"
			"real_url            ,"
        "referer,"
			"redirect_times      ,"
			"finish_time         ,"
			"task_status         ,"
			"total_packet        ,"
			"abtained_packet     ,"
			"left_packet         ,"
			"total_timestamp     ,"
			"abtained_timestamp,"
			"left_timestamp     ,"
			"file_length_abtained,"
			"dirty               ,"
            "aria_gid            "
            ;

	this->mSegColumnStr = 
			"seg_id           ,"  
			"task_id          ,"
			"start_offset     ,"
			"create_time      ,"
			"finish_time      ,"
			"total_length     ,"
			"abtained_length  ,"
			"current_speed    ,"
			"average_speed    ,"
			"abtained_percent ,"
			"segment_status   ,"
			"total_packet     ,"
			"abtained_packet  ,"
			"left_packet      ,"
			"total_timestamp  ,"
			"finish_timestamp,"
			"left_timestamp  ,"
			"dirty            "
		;

	this->mCatColumnStr = "display_name, path, cat_id, parent_cat_id, can_child, raw_name, folder, delete_flag, create_time, dirty";

}

AbstractStorage::~AbstractStorage()
{

}


