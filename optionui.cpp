// optionui.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-05 16:27:47 +0000
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
#include "taskinfodlg.h"

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

#include "optionui.h"

OptionUi::OptionUi(Karia2 *pwin)
    : AbstractUi(pwin)
{
}

OptionUi::~OptionUi()
{
}

