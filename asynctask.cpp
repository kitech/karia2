// asynctask.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-01-30 00:06:51 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "ui_karia2.h"

#include "utility.h"
#include "karia2.h"
#include "aboutdialog.h"
#include "dropzone.h"
#include "taskinfodlg.h"

#include "sqlitecategorymodel.h"
#include "sqlitestorage.h"
#include "sqlitetaskmodel.h"
#include "sqlitesegmentmodel.h"
#include "segmentlogmodel.h"
#include "taskitemdelegate.h"
#include "optionmanager.h"

#include "catmandlg.h"
#include "catpropdlg.h"
#include "preferencesdialog.h"

#include "batchjobmandlg.h"
#include "webpagelinkdlg.h"
#include "taskqueue.h"

#include "taskballmapwidget.h"
#include "instantspeedhistogramwnd.h"
#include "walksitewndex.h"

// #include "norwegianwoodstyle.h"
#include "torrentpeermodel.h"
#include "taskservermodel.h"
#include "seedfilemodel.h"
#include "seedfilesdialog.h"

//////
#include "libng/html-parse.h"

//labspace
#include "labspace.h"

#if !defined(Q_OS_WIN32)
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#endif

#include "simplelog.h"

//#include "ariaman.h"
//#include "maiaXmlRpcClient.h"

#include "emaria2c.h"
#include "karia2statcalc.h"

#include "metauri.h"
//#include "skype.h"
//#include "skypetunnel.h"
//#include "skypetracer.h"

#include "asynctask.h"

AsyncTask::AsyncTask(Karia2 *pwin)
    : QThread(pwin)
    , mpwin(pwin)
{
    QObject::connect(this, &AsyncTask::canFirstShow, mpwin, &Karia2::firstShowHandler);
    QObject::connect(this, &AsyncTask::canAsyncFirstShow, mpwin, &Karia2::asyncFirstShowHandler);
}

AsyncTask::~AsyncTask()
{
}

// QObject::setParent: Cannot set parent, new parent is in a different thread
// The canonical way to solve this problem is to use signals and slots.
void AsyncTask::run()
{
    // this->moveToThread(this->mpwin->thread());
    // this->mpwin->firstShowHandler();

    emit this->canFirstShow();

    // this->sleep(1); // 延迟1秒比原来好很多，界面会快速显示，数据加载有动态效果
    this->msleep(300);
    // this->mpwin->asyncFirstShowHandler();
    // emit this->canAsyncFirstShow();
    qLogx()<<"";

    this->exec();
}
