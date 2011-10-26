// resourcemonitor.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-05 11:02:42 +0800
// Version: $Id$
// 

#ifndef RESOURCEMONITOR_H
#define RESOURCEMONITOR_H

#ifdef WIN32
#include <windows.h>
#include <psapi.h>
#endif

#include <QObject>
#include <QString>
#include <QtCore>


class ResourceMonitor : public QObject
{
	Q_OBJECT;
public:
	static ResourceMonitor * instance( );
	~ResourceMonitor();

	QString getMemoryUsage();
	QString getCPUUsage();

private:
	ResourceMonitor(QObject *parent = 0 );
	static ResourceMonitor * mHandle ;

	QString seprateByColon(int num , int len = 3);

#ifdef WIN32
	QString getWin32MemoryUsage();
	QString getWin32CPUUsage();
	////////
	FILETIME mLastUserTime ;
	FILETIME mLastKernelTime ;
#else
    QString getUnixMemoryUsageByProc();
	QString getUnixMemoryUsageByTop();
	QString getUnixCPUUsage();

#endif

	quint64  mLastSysTime ;
	quint64  mLastCurrentProcessTime ;


};

#endif // RESOURCEMONITOR_H
