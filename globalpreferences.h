// globalpreferences.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-07 21:02:46 +0800
// Version: $Id$
// 
#ifndef GLOBALPREFERENCES_H
#define GLOBALPREFERENCES_H

#include <QObject>

/**
 * 单实例模式全局配置参数管理类
 * 它负责提供数据，保存数据等工作。
 *
 * 有两种方式存储这些数据，第一是使用map类，第二种使用定义成员变量的方式。
 * 为了安全期间，还是使用第二种方法，虽然管理起来麻烦一点。成员变量全部采用公有形式
 */
class GlobalPreferences : public QObject
{
	Q_OBJECT;
public:
	static GlobalPreferences * instance() ;

	~GlobalPreferences();

	//从永久存储中加载值。
	bool loadFromStorage();
	//将值全部存储到永久存储中
	bool saveToStorage();

public:
	bool mGeneralStartMainWindowMinimized ;
	int mGeneralMinimumSegmentSize ;	//以字节计，注意转换
	bool mGeneralStopDownloadWhenErrorOccurs;
	bool mGeneralStillDownloadFromNonResumableSite ;
	bool mGeneralAutoSaveFileList ;
	int  mGeneralAutoSaveFileListEveryMinute;//秒计 ，注意转换
	int  mGeneralWriteDataEveryBytes; //以字节计，注意转换
	bool mGeneralGetDateTimeFromServer;//
	bool mStartDownloadingWhenStartup;
	bool mGeneralUseNGAlertExtensionWhenDownloading;
	bool mGeneralAutoBackupDownloadDatabaseEveryDay;
	bool mGeneralWriteIndividualLogFileForEachFile ;
	bool mGeneralMoveOrDeleteLogFileWithFile;
	bool mPlayNotifyAudioWhenDone ;

	////////////
	QString mDefaultPropertiesReference;
	bool mDefaultPropertiesShowReferenceWhenNew;
	int mDefaultPropertiesStartStatus ; //0 , manual,1,imidiate
	int mDefaultPropertiesOrgUrlSegmentCount ; //
	int mDefaultPropertiesMaxSegmentCountForEveryTask ;
	bool mDefaultPropertiesOpenFileWhenDone ;

	////////////////



private:
	GlobalPreferences(QObject *parent = 0 );

	//
	static GlobalPreferences * mInstance ;

};

#endif // GLOBALPREFERENCES_H
