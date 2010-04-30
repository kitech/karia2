// nullget.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 22:27:02 +0800
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
#include <QPluginLoader>
#include <QLibraryInfo>
#include <QVarLengthArray>
#include <QDialogButtonBox>

#include "nullget.h"

#include "taskinfodlg.h"

#include "sqlitecategorymodel.h"
#include "sqlitestorage.h"
#include "sqlitetaskmodel.h"
#include "sqlitesegmentmodel.h"
#include "segmentlogmodel.h"
#include "taskitemdelegate.h"

#include "catmandlg.h"
#include "catpropdlg.h"
#include "viewmodel.h"
#include "optiondlg.h"
#include "preferencesdialog.h"

#include "batchjobmandlg.h"
#include "webpagelinkdlg.h"
#include "taskqueue.h"

#include "taskballmapwidget.h"
#include "instantspeedhistogramwnd.h"
#include "walksitewndex.h"

#include "norwegianwoodstyle.h"
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

#include "ariaman.h"
#include "maiaXmlRpcClient.h"


extern QHash<QString, QString> gMimeHash;

////////////////////////////////////////////////
//主窗口类。
////////////////////////////////////////////////
NullGet::NullGet(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
    , mTaskMan(NULL)
    , mAriaMan(NULL), mAriaRpc(NULL)
{
    QDir().setCurrent(qApp->applicationDirPath()); //修改当前目录
	mainUI.setupUi(this);	
	firstShowEvent = true;

	/////////////
	orginalPalette = QApplication::palette();
	//dynamic language switch
	qmLocale = QLocale::system().name();
	qmPath = qApp->applicationDirPath() + QString("/translations");
	qDebug()<<"Switch Langague to: "<<qmLocale;
	qApp->installTranslator(&appTranslator);
	qApp->installTranslator(&qtTranslator);
	appTranslator.load(QString("nullget_") + qmLocale , qmPath );
	qtTranslator.load(QString("qt_") + qmLocale , qmPath );
	this->retranslateUi();
	if (qmLocale.startsWith("zh_CN")) {
		this->mainUI.action_Chinese_simple->setChecked(true);
	} else if (qmLocale.startsWith("zh_TW")) {
		this->mainUI.action_Chinese_trad->setChecked(true);
	} else {
		this->mainUI.action_English->setChecked(true);
	}
	this->mDropZone = new DropZone();	//浮动窗口句柄

    //this->mainUI.dockWidget->setWindowTitle(QDir::currentPath());		
}
/**
 * 用于首次显示用户界面时的初始化工作
 */
void NullGet::firstShowHandler()
{
	this->mISHW = new InstantSpeedHistogramWnd(this->mainUI.speed_histogram_toolBar);
	this->mainUI.speed_histogram_toolBar->addWidget(this->mISHW);

	QScrollArea *sa = new QScrollArea(this->mainUI.tab_4);
	this->mainUI.tab_4->setLayout(new QVBoxLayout());
	this->mainUI.tab_4->layout()->addWidget(sa);

	TaskBallMapWidget * wg = TaskBallMapWidget::instance(sa);
	//sa->setLayout(new QVBoxLayout());
	sa->setWidget(wg);
	sa->setWidgetResizable(false);
	sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	//
	this->initPopupMenus();
	this->initStatusBar();
	this->initSystemTray();	
    this->initAppIcons();
	
	this->initialMainWindow();
#ifdef WIN32
	// this->onSwitchWindowStyle(mainUI.actionWindowsX_P); //set default windowsxp UI style
#else
	// this->onSwitchWindowStyle(mainUI.action_Plastique); //set default Plastique UI style
#endif

	this->update();

	///////	
	//初始化配置数据库类实例
	this->mConfigDatabase = ConfigDatabase::instance(this);
	// this->mStorage = new SqliteStorage(this);
    this->mStorage = SqliteStorage::instance(this);
	this->mStorage->open();

    //
	this->mCatView = this->mainUI.mui_tv_category;
	this->mCatView->setSelectionMode(QAbstractItemView::SingleSelection );
	//this->mCatViewModel = CategoryModel::instance(0);
	this->mCatViewModel = SqliteCategoryModel::instance(0);
	//this->mCatViewModel =  TreeModel::instance(0);
	this->mCatView->setModel(this->mCatViewModel);
    this->mCatView->setRootIndex(this->mCatViewModel->index(0, 0)); // root is the topest node
	this->mCatView->expandAll();

    this->update();

    ////
	this->mTaskListView = this->mainUI.mui_tv_task_list;
    this->mTaskItemDelegate = new TaskItemDelegate();
    this->mTaskListView->setItemDelegate(this->mTaskItemDelegate);
	// this->mTaskTreeViewModel = SqliteTaskModel::instance(ng::cats::downloading, this);

	// this->mTaskListView->setModel(this->mTaskTreeViewModel);
	this->mTaskListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	this->mTaskListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	this->mTaskListView->setAlternatingRowColors(true);
	// QObject::connect(this->mTaskListView->selectionModel(),
    //                  SIGNAL(selectionChanged (const QItemSelection & , const QItemSelection &   )),
    //                  this, SLOT(onTaskListSelectChange(const QItemSelection & , const QItemSelection &   ) ) );

	this->mSegListView = this->mainUI.mui_tv_seg_list;
	this->mSegListView->setSelectionMode(QAbstractItemView::SingleSelection );
	this->mSegListView->setAlternatingRowColors(true);
	//this->mSegListView ->setModel(mdl);

	this->mSegLogListView = this->mainUI.mui_tv_seg_log_list;
	this->mSegLogListView->setSelectionMode(QAbstractItemView::ExtendedSelection );

    // 初始化任务队列管理实例
    this->mTaskMan = TaskQueue::instance();

	this->update();

    // select the downloading cat Automaticly
    

    this->mSeedFileDelegate = new SeedFileItemDelegate();
    this->mSeedFileView = this->mainUI.treeView_2;
    this->mSeedFileView->setItemDelegate(this->mSeedFileDelegate);

	//this->onLoadAllTask();	//loading database 
	
	//非UI成员
	
	this->mAverageSpeedTimer.setInterval(1000)	;	//1秒合计一次所有任务的平均速度。
	this->mAverageSpeedTimer.start();

	///////////////一些lazy模式实例化的类指针初始化。
	this->mWalkSiteWnd = 0;
	this->mWalkSiteDockWidget = 0;
	this->mHWalkSiteWndEx = 0;

	//
	this->connectAllSignalAndSlog();

    this->mainUI.action_Pause->setEnabled(false);
    this->mainUI.action_Start->setEnabled(false);
    this->mainUI.action_Delete_task->setEnabled(false);
    // this->onUpdateJobMenuEnableProperty();
	
	//this->showMaximized();

    // start backend
    this->mAriaMan = new AriaMan();
    this->mAriaMan->start();
    this->mAriaGlobalUpdater.setInterval(5*1000);
    QObject::connect(&this->mAriaGlobalUpdater, SIGNAL(timeout()),
                     this, SLOT(onAriaGlobalUpdaterTimeout()));
    this->mAriaGlobalUpdater.start();
    QObject::connect(this->mAriaMan, SIGNAL(taskLogReady(QString, QString, QString)),
                     this->mTaskMan, SLOT(onTaskLogArrived(QString, QString, QString)));
    QObject::connect(this->mAriaMan, SIGNAL(error(QProcess::ProcessError)),
                     this, SLOT(onAriaProcError(QProcess::ProcessError)));
    QObject::connect(this->mAriaMan, SIGNAL(finished(int, QProcess::ExitStatus)),
                     this, SLOT(onAriaProcFinished(int, QProcess::ExitStatus)));

	///////
	this->hideUnimplementUiElement();
	this->hideUnneededUiElement();


    // select the default cat model
    {
        // cacl the downloading model index
        QItemSelection readySelect, readDeselect;
        QModelIndex topCatIdx = this->mCatViewModel->index(0, 0);
        qDebug()<<topCatIdx.data();
        int l2RowCount = this->mCatViewModel->rowCount(topCatIdx);
        qDebug()<<l2RowCount;
        for (int r = 0 ; r < l2RowCount ; r ++) {
            QModelIndex currCatIdx = this->mCatViewModel->index(r, ng::cats::cat_id, topCatIdx);
            qDebug()<<currCatIdx;
            if (currCatIdx.data().toInt() == ng::cats::downloading) {
                // for (int c = 0; c <= ng::cats::dirty; c ++) {
                //     QModelIndex readyIndex = this->mCatViewModel->index(r, c, topCatIdx);
                //     readySelect.select(readyIndex, readyIndex);
                // }
                readySelect.select(this->mCatViewModel->index(r, 0, topCatIdx), 
                                   this->mCatViewModel->index(r, ng::cats::dirty, topCatIdx));
                break;
            }
        }
        this->mCatView->selectionModel()->select(readySelect, QItemSelectionModel::Select);
    }

    // process arguments 
    this->handleArguments();

	//test area 测试代码区　---------开始-----------------
	LabSpace * labspace = new LabSpace(this);
	//labspace->show();

#ifdef Q_OS_WIN32
	//注册系统热键。发向该窗口的。
	if (! ::RegisterHotKey(this->winId(), 'C', MOD_CONTROL|MOD_SHIFT, 'C')) {
		qDebug()<<"::RegisterHotKey faild";
	}
#else	
#endif

	//test area 测试代码区　---------结束-----------------
}

NullGet::~NullGet()
{
	if (this->mAverageSpeedTimer.isActive())
		this->mAverageSpeedTimer.stop();
    this->mAriaMan->stop();
    delete this->mAriaMan;
}
/**
 * 初始化主窗口大小，以及各分隔栏的比例，初始化窗口风格为XP
 */
void NullGet::initialMainWindow()
{
	//qDebug()<<__FUNCTION__;
	//set split on humanable postion , but not middle
	QList<int> splitSize;
	splitSize = this->mainUI.splitter_4->sizes();	
	splitSize[0] -= 70;
	splitSize[1] += 70;	
	this->mainUI.splitter_4->setSizes(splitSize);

	splitSize = this->mainUI.splitter_3->sizes();	
	splitSize[0] -= 100;
	splitSize[1] += 100;	
	this->mainUI.splitter_3->setSizes(splitSize);

	splitSize = this->mainUI.splitter_2->sizes();
	splitSize[0] -= 50;
	splitSize[1] += 50;	
	//this->mainUI.splitter_2->setSizes(splitSize);


	splitSize = this->mainUI.splitter->sizes();
	//qDebug()<<splitSize;
	splitSize[0] += 80;
	splitSize[1] -= 80;
	this->mainUI.splitter->setSizes(splitSize);

	//this->onSwitchToWindowsXPStyle(true);	
}

void NullGet::moveEvent (QMoveEvent * event )
{
	QPoint cp = event->pos();
	QPoint op = event->oldPos();

	
	QSize winSize = this->size();
	QRect winRect = this->rect();

	QPoint pos = this->pos();

	int dtwidth = QApplication::desktop()->width();
	int dtheight = QApplication::desktop()->height();
	//qDebug()<<event->pos()<<event->oldPos()<<"W: "<<dtwidth<<" H:"<<dtheight;

	//top 

	QMainWindow::moveEvent(event);

}



void NullGet::initPopupMenus()
{
	//action groups 
	QActionGroup *group = new QActionGroup(this);
	QObject::connect(group, SIGNAL(triggered(QAction *)), this, SLOT(onSwitchSpeedMode(QAction*)));
	group->addAction(mainUI.action_Unlimited);
	group->addAction(mainUI.action_Manual);
	group->addAction(mainUI.action_Automatic);

    // should dynamic check available styles
	//window style : "windows", "motif", "cde", "plastique", "windowsxp", or "macintosh" , "cleanlooks" 
	//NorwegianWood
	group = new QActionGroup(this);
	mainUI.action_NorwegianWood->setData("norwegianwood");
	group->addAction(mainUI.action_NorwegianWood);
    // automatic check supported style
    QStringList styleKeys = QStyleFactory::keys();
    QStyle *defaultStyle = QApplication::style();
    // qDebug()<<QApplication::style()<<styleKeys;
    for (int i = 0; i < styleKeys.count(); ++i) {
        QAction *styleAction = new QAction(styleKeys.at(i), this->mainUI.menuStyle);
        styleAction->setData(styleKeys.at(i));
        styleAction->setCheckable(true);
        if (styleKeys.at(i).toLower() == defaultStyle->objectName()) {
            styleAction->setChecked(true);
        }
        this->mainUI.menuStyle->addAction(styleAction);
        group->addAction(styleAction);
    }
	QObject::connect(group, SIGNAL(triggered(QAction *)), this, SLOT(onSwitchWindowStyle(QAction*)));

	//supported languages: en_US , zh_CN , zh_TW
	group = new QActionGroup(this);
	mainUI.action_Chinese_simple->setData("zh_CN");
	mainUI.action_Chinese_trad->setData("zh_TW");
	mainUI.action_English->setData("en_US");
	group->addAction(mainUI.action_Chinese_simple);
	group->addAction(mainUI.action_Chinese_trad);
	group->addAction(mainUI.action_English);
	QObject::connect(group, SIGNAL(triggered(QAction *)), this, SLOT(onSwitchLanguage(QAction*)));
	//mainUI.action_English->setChecked(true);

	/////skin
	group = new QActionGroup(this);
	group->addAction(mainUI.actionNone);
	group->addAction(mainUI.actionNormal);
	group->addAction(mainUI.actionXP_Luna);
	group->addAction(mainUI.actionXP_Luna_Gradient);
	group->addAction(mainUI.actionSkype_Gradient);
	group->addAction(mainUI.actionCustom_Background);
	group->addAction(mainUI.actionImageBk);
	QObject::connect(group, SIGNAL(triggered(QAction *)), this, SLOT(onSwitchSkinType(QAction*)));


	/////// all popups
	this->mTaskPopupMenu = new QMenu("&Task", this);
		
	QList<QAction*> actionList;
	actionList.append(this->mainUI.action_Start);
	actionList.append(this->mainUI.action_Pause);
	actionList.append(this->mainUI.action_Start_All);
	actionList.append(this->mainUI.action_Pause_All);
	actionList.append(this->mainUI.action_Schedule);
	this->mTaskPopupMenu->addActions(actionList);
	this->mTaskPopupMenu->addSeparator();
	actionList.clear();
	this->mTaskPopupMenu->addAction(this->mainUI.action_Move_top);
	this->mTaskPopupMenu->addAction(this->mainUI.action_Move_bottom);
	this->mTaskPopupMenu->addSeparator();
	this->mTaskPopupMenu->addAction(this->mainUI.action_Delete_task);
	this->mTaskPopupMenu->addSeparator();
	this->mTaskPopupMenu->addAction(this->mainUI.action_Properties);
	this->mTaskPopupMenu->addAction(this->mainUI.action_Site_Properties);
	this->mTaskPopupMenu->addAction(this->mainUI.action_Comment);
	this->mTaskPopupMenu->addAction(this->mainUI.action_Browse_Referer );
	this->mTaskPopupMenu->addAction(this->mainUI.action_Browse_With_Site_Explorer );
	this->mTaskPopupMenu->addSeparator();
	this->mTaskPopupMenu->addAction(this->mainUI.action_Download_Again );
	this->mTaskPopupMenu->addSeparator();
	this->mTaskPopupMenu->addAction(this->mainUI.action_Copy_URL_To_ClipBoard );

	/////////////
	this->mTaskPopupMenu->addActions(actionList);

	actionList.clear();
	actionList.append(this->mainUI.action_Seg_Log_Copy);
	actionList.append(this->mainUI.actionSelect_All);
	actionList.append(this->mainUI.action_Seg_Log_Save_To_File);
	actionList.append(this->mainUI.action_Clear_Seg_Log);

	this->mLogPopupMenu = new QMenu("&SegLog", this);
	this->mLogPopupMenu->addActions(actionList);

	actionList.clear();
	actionList.append(this->mainUI.action_Seg_List_Start);
	actionList.append(this->mainUI.action_Seg_List_Stop);
	actionList.append(this->mainUI.action_Seg_List_Restart);
	actionList.append(this->mainUI.action_Increase_split_parts);
	actionList.append(this->mainUI.action_Decrease_split_parts);

	this->mSegmentPopupMenu = new QMenu("&SegList", this);
	this->mSegmentPopupMenu->addActions(actionList);	

	this->mCatPopupMenu = new QMenu("&Category", this);
	actionList.clear();
	actionList.append(this->mainUI.actionNew_Cagegory);
	actionList.append(this->mainUI.action_Delete_cat);
	actionList.append(this->mainUI.action_cat_Move_To);
	actionList.append(this->mainUI.action_Cat_Property);
	this->mCatPopupMenu->addActions(actionList);


	//drop zone
	this->mDropZonePopupMenu = new QMenu("&DropZone", this);
	this->mDropZonePopupMenu->addAction(this->mainUI.action_Show_Hide_Main_Widow);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI.action_Monitor_Clipboard);
	this->mDropZonePopupMenu->addAction(this->mainUI.action_Disable_Browser_Monitor);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI.action_Start_All);
	this->mDropZonePopupMenu->addAction(this->mainUI.action_Pause_All);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI.action_New_Download);
	this->mDropZonePopupMenu->addAction(this->mainUI.actionAdd_batch_download);
	this->mDropZonePopupMenu->addAction(this->mainUI.actionPaste_URL);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addMenu(this->mainUI.menuSpeed_Limit_Mode);
	this->mDropZonePopupMenu->addSeparator ();	
	this->mDropZonePopupMenu->addMenu(this->mainUI.menu_Search);
	
	this->mDropZonePopupMenu->addAction(this->mainUI.action_Site_Explorer);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI.action_Options);
	this->mDropZonePopupMenu->addAction(this->mainUI.action_About_NullGet);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI.actionQuit);

	//mSysTrayMenu
	this->mSysTrayMenu = new QMenu("NullTray", this);
	this->mSysTrayMenu->addAction(this->mainUI.action_Show_Hide_Main_Widow);
	this->mSysTrayMenu->addSeparator();
	this->mSysTrayMenu->addAction(this->mainUI.actionQuit);
	this->mSysTrayMenu->addAction(this->mainUI.actionAbout_Qt);

}

void NullGet::initStatusBar()
{
	//进度条初始化状态应该从配置读取出来才对。开始结束的最大值也从配置文件来。

	//int begin = 1 , end = 500000;	//K	50M
	int begin = 1 , end = 500;	//K	500KB/s

	QStatusBar *bar = this->statusBar();
	QLabel *lab = new QLabel("" );
	lab->setFixedWidth(0);
	lab->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	//bar->addWidget(lab);
	bar->addPermanentWidget(lab);
	this->mStatusMessageLabel = lab;

	QProgressBar *pbar = new QProgressBar();	//进度条
	pbar->setFixedWidth(120);
	pbar->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	//bar->addWidget(pbar);
	bar->addPermanentWidget(pbar);
	this->mSpeedProgressBar = pbar;
	pbar->setRange (begin , end );
	pbar->setValue(begin);
	pbar->setHidden(true);	//初始化为隐藏

	QSlider *hslider = new QSlider(Qt::Horizontal);	//滑动条
	hslider->setFixedWidth(120);
	hslider->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    hslider->setTracking(false); // avoid drap signal and app busy with socket community

	//bar->addWidget(hslider);
	bar->addPermanentWidget(hslider);
	this->mSpeedBarSlider = hslider;
	hslider->setRange(begin , end );
	hslider->setValue(begin);
	hslider->setHidden(true);	////初始化为隐藏

	lab = new QLabel(QString("%1 KB/s").arg(hslider->value()) );	//速度显示条
	lab->setFixedWidth(100);
	lab->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	//bar->addWidget(lab);
	bar->addPermanentWidget(lab);
	this->mSpeedManualLabel = lab;
	
	lab->setHidden(true);	//初始化为隐藏

	lab = new QLabel("0.00KB/s" );	//for total task speed
	lab->setFixedWidth(100);
	//lab->setFixedHeight(28);
	lab->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	bar->addPermanentWidget(lab);
	this->mSpeedTotalLable = lab;

}
/**
 * 初始化系统托盘
 *
 */
void NullGet::initSystemTray()
{
	//init system tray icon
	this->mSysTrayIcon = new QSystemTrayIcon(this);
	this->mSysTrayIcon->setToolTip(tr("NullGet Icon Tray Control"));
	//this->mSysTrayIcon->setContextMenu(this->mSysTrayMenu);
	this->mSysTrayIcon->setContextMenu(this->mDropZonePopupMenu);

    QString iconname;
	int index = 0;
    switch (index) {
    default:
    case 0:
		//iconname = QString(qApp->applicationDirPath()+"/"+"resources/icon_16x16.png");
       // break;
    case 1:
        //iconname = QString(qApp->applicationDirPath()+"/"+"resources/icon_22x22.png");
        //break;
   case 2:
        // iconname = QString(qApp->applicationDirPath() + "/Resources/nullget-1.png");
        iconname = QString(qApp->applicationDirPath() + "/" +"Resources/karia2.png");
        break;
    }

    QPixmap pix(iconname);
	this->mSysTrayIcon->setIcon(QIcon(pix));

	this->mSysTrayIcon->show();

	//this->setWindowIcon(QIcon(qApp->applicationDirPath()+"/"+"resources/icon_16x16.png"));	
    // this->setWindowIcon(QIcon(qApp->applicationDirPath()+"/Resources/nullget-1.png"));	
    this->setWindowIcon(QIcon(qApp->applicationDirPath()+"/Resources/karia2.png"));	
}

void NullGet::initAppIcons()
{
    QString dir = qApp->applicationDirPath() + "/icons";
    this->mainUI.action_Start->setIcon(QIcon(dir + "/media-playback-start.png"));
    this->mainUI.action_Pause->setIcon(QIcon(dir + "/media-playback-pause.png"));
    this->mainUI.action_New_Download->setIcon(QIcon(dir + "/list-add.png"));
    this->mainUI.action_Delete_task->setIcon(QIcon(dir + "/list-remove.png"));
    this->mainUI.action_Properties->setIcon(QIcon(dir + "/document-properties.png"));
    this->mainUI.actionMove_Up->setIcon(QIcon(dir + "/arrow-up.png"));
    this->mainUI.actionMove_Down->setIcon(QIcon(dir + "/arrow-down.png"));
    this->mainUI.actionOpen_Exec_download_file->setIcon(QIcon(dir + "/system-run.png"));
    this->mainUI.actionOpen_de_stination_directory->setIcon(QIcon(dir + "/document-open-folder.png"));
    this->mainUI.actionFind->setIcon(QIcon(dir + "/edit-find.png"));
    this->mainUI.actionFind_next->setIcon(QIcon(dir + "/go-down-search.png"));
    this->mainUI.action_Connect_Disconnect->setIcon(QIcon(dir + "/network-connect.png"));
    // this->mainUI.actionDefault_Download_Properties->setIcon(QIcon(dir + "/configure.png"));
    this->mainUI.action_Options->setIcon(QIcon(dir + "/preferences-system.png"));
    this->mainUI.actionQuit->setIcon(QIcon(dir + "/system-shutdown.png"));
    this->mainUI.label->setPixmap(QPixmap(dir + "/status/unknown.png"));
}

void NullGet::connectAllSignalAndSlog()
{
	QObject::connect(this->mainUI.actionDrop_zone, SIGNAL(triggered(bool)), this->mDropZone,SLOT(setVisible(bool)));
	
	//file action
	//QObject::connect(this->mainUI.action_New_Database, SIGNAL(triggered()), this, SLOT(onNewDatabase()));	
	//QObject::connect(this->mainUI.action_Open_Database, SIGNAL(triggered()), this, SLOT(onOpenDatabase()));	
	//QObject::connect(this->mainUI.action_Save_Database, SIGNAL(triggered()), this, SLOT(onSaveAllTask()));	
	//QObject::connect(this->mainUI.actionSave_As, SIGNAL(triggered()), this, SLOT(onSaveAllTaskAs()));

	QObject::connect(this->mainUI.actionProcess_Web_Page_File, SIGNAL(triggered()), this, SLOT(showProcessWebPageInputDiglog()));

	//editr
	QObject::connect(this->mainUI.actionPaste_URL , SIGNAL(triggered()), this, SLOT(showNewDownloadDialog()));
	QObject::connect(this->mainUI.actionSelect_All, SIGNAL(triggered()), this, SLOT(onEditSelectAll()));
	QObject::connect(this->mainUI.actionInvert_Select, SIGNAL(triggered()), this, SLOT(onEditInvertSelect()));
	
	//cat action
	QObject::connect(this->mainUI.actionNew_Cagegory , SIGNAL(triggered()), this, SLOT(onNewCategory()));
	QObject::connect(this->mainUI.action_Cat_Property , SIGNAL(triggered()), this, SLOT(onShowCategoryProperty()));
	QObject::connect(this->mainUI.action_Delete_cat, SIGNAL(triggered()), this, SLOT(onDeleteCategory()));
	QObject::connect(this->mainUI.action_cat_Move_To , SIGNAL(triggered()), this, SLOT(onCategoryMoveTo()));

	//view
	QObject::connect(this->mainUI.action_Show_Columns_Editor , SIGNAL(triggered()), this, SLOT(onShowColumnEditor()));
	QObject::connect(this->mainUI.actionShow_Text, SIGNAL(triggered(bool)), this, SLOT(onShowToolbarText(bool) ) );
	//job action 
	QObject::connect(this->mainUI.menu_Jobs, SIGNAL(aboutToShow()), this, SLOT(onTaskListMenuPopup()));
	QObject::connect(this->mainUI.action_New_Download, SIGNAL(triggered()), this, SLOT(showNewDownloadDialog()));
    QObject::connect(this->mainUI.action_New_Bittorrent, SIGNAL(triggered()), this, SLOT(showNewBittorrentFileDialog())); 
    QObject::connect(this->mainUI.action_New_Metalink, SIGNAL(triggered()), this, SLOT(showNewMetalinkFileDialog()));
	QObject::connect(this->mainUI.actionAdd_batch_download, SIGNAL(triggered()), this, SLOT(showBatchDownloadDialog()));
	
	QObject::connect(this->mainUI.action_Start , SIGNAL(triggered()), this, SLOT(onStartTask()));
	QObject::connect(this->mainUI.action_Pause , SIGNAL(triggered()), this, SLOT(onPauseTask()));
	QObject::connect(this->mainUI.action_Start_All , SIGNAL(triggered()), this, SLOT(onStartTaskAll()));
	QObject::connect(this->mainUI.action_Pause_All , SIGNAL(triggered()), this, SLOT(onPauseTaskAll()));

	QObject::connect(this->mainUI.action_Delete_task, SIGNAL(triggered()), this, SLOT(onDeleteTask()));

	QObject::connect(this->mainUI.action_Properties, SIGNAL(triggered()), this, SLOT(onShowTaskProperty()));

	QObject::connect(this->mainUI.actionOpen_de_stination_directory, SIGNAL(triggered()), this, SLOT(onOpenDistDirector()));
	QObject::connect(this->mainUI.actionOpen_Exec_download_file, SIGNAL(triggered()), this, SLOT(onOpenExecDownloadedFile()));
	QObject::connect(this->mainUI.action_Browse_Referer, SIGNAL(triggered()), this, SLOT(onOpenRefererUrl()));
	
	//tools 
	QObject::connect(this->mainUI.action_Options, SIGNAL(triggered()), this, SLOT(onShowOptions()));
	QObject::connect(this->mainUI.action_Connect_Disconnect, SIGNAL(triggered()), this, SLOT(onShowConnectOption()));
	
	// QObject::connect(this->mainUI.actionDefault_Download_Properties , SIGNAL(triggered()), this, SLOT(onShowDefaultDownloadProperty()));

	//statusbar
	QObject::connect(this->mSpeedBarSlider, SIGNAL(valueChanged(int)), this, SLOT(onManualSpeedChanged(int)));

	//help action 
	this->connect(this->mainUI.action_Go_to_NullGet_Home_Page, SIGNAL(triggered()), this, SLOT(onGotoHomePage()));
	this->connect(this->mainUI.action_About_NullGet, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
	QObject::connect(this->mainUI.actionAbout_Qt, SIGNAL(triggered()), this, SLOT(onAboutQt()));

	//centrol 
	QObject::connect(this->mainUI.mui_tv_task_list, SIGNAL(customContextMenuRequested (const QPoint &  )),
		this, SLOT(onTaskListMenuPopup(/*const QPoint &*/)));
	QObject::connect(this->mainUI.mui_tv_task_list, SIGNAL(entered(const QModelIndex & )) ,
		this, SLOT(onShowTaskPropertyDigest(const  QModelIndex & ) ) );

	QObject::connect(this->mainUI.mui_tv_seg_log_list, SIGNAL(customContextMenuRequested (const QPoint &  )),
		this, SLOT(onLogListMenuPopup(const QPoint &)));
	QObject::connect(this->mainUI.mui_tv_seg_list, SIGNAL(customContextMenuRequested (const QPoint &  )),
		this, SLOT(onSegListMenuPopup(const QPoint &)));
	QObject::connect(this->mainUI.mui_tv_category, SIGNAL(customContextMenuRequested (const QPoint &  )),
		this, SLOT(onCateMenuPopup(const QPoint &)));


	QObject::connect(this->mDropZone, SIGNAL(doubleclicked()), this, SLOT(onDropZoneDoubleClicked()));
	QObject::connect(this->mainUI.action_Show_Hide_Main_Widow, SIGNAL(triggered()), this, SLOT(onDropZoneDoubleClicked()));
	QObject::connect(this->mDropZone, SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(onDropZoneCustomMenu(const QPoint &)));
	

	//seg view menu
	QObject::connect(this->mainUI.action_Seg_List_Start, SIGNAL(triggered()), this, SLOT(onStartSegment()));
	QObject::connect(this->mainUI.action_Seg_List_Stop, SIGNAL(triggered()), this, SLOT(onPauseSegment()));
	//QObject::connect(this->mainUI.action_Seg_List_Restart, SIGNAL(triggered()), this, SLOT(onRestartSegment()));

	//seg log menu
	QObject::connect(this->mainUI.action_Seg_Log_Copy, SIGNAL(triggered()), this, SLOT(onCopySelectSegLog()));
	QObject::connect(this->mainUI.action_Seg_Log_Save_To_File, SIGNAL(triggered()), this, SLOT(onSaveSegLog()));
	QObject::connect(this->mainUI.action_Clear_Seg_Log, SIGNAL(triggered()), this, SLOT(onClearSegLog()));

	//cat view
	QObject::connect(this->mCatView->selectionModel(), SIGNAL(selectionChanged (const QItemSelection & , const QItemSelection &   )),
		this, SLOT(onCatListSelectChange(const QItemSelection & , const QItemSelection &   ) ) );

	//toolbar	//在UI设计器中将信号传递到标准菜单中。
	//QObject::connect(this->mainUI.mui_tb_properties, SIGNAL(triggered()), this, SLOT(onShowTaskProperty()));
	//QObject::connect(this->mainUI.mui_tb_open_dir, SIGNAL(triggered()), this, SLOT(onOpenDistDirector()));
	//QObject::connect(this->mainUI.mui_tb_exec_file, SIGNAL(triggered()), this, SLOT(onOpenExecDownloadedFile()));

	//other
	QObject::connect(this->mainUI.action_Copy_URL_To_ClipBoard, SIGNAL(triggered()), this, SLOT(onCopyUrlToClipboard()));

	QObject::connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(onClipBoardDataChanged()));

    QObject::connect(&this->mAverageSpeedTimer, SIGNAL(timeout()), this, SLOT(caclAllTaskAverageSpeed()));

	QObject::connect(this->mainUI.actionWalk_Site, SIGNAL(triggered()), this, SLOT(onShowWalkSiteWindow()));
	QObject::connect(this->mSysTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onActiveTrayIcon(QSystemTrayIcon::ActivationReason)));
	QObject::connect(this->mSysTrayIcon, SIGNAL(messageClicked()),  this, SLOT(onBallonClicked()));

	//test it
	//this->connect(this->mainUI.pushButton_3, SIGNAL(clicked()), this, SLOT(testFunc()));
	//QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit () ), this, SLOT(testFunc2()));

}
//temporary 临时用于隐藏没有实现功能用户界面的代码。
void NullGet::hideUnimplementUiElement()
{
    this->mainUI.menu_Search->setVisible(false);
    this->mainUI.menu_Plugins->setVisible(false);
    delete this->mainUI.menu_Plugins;
    delete this->mainUI.menu_Search;
    this->mainUI.menu_Search = 0;
    this->mainUI.menu_Plugins = 0;

    // action
	this->mainUI.action_New_Database->setVisible(false);
	this->mainUI.action_Open_Database->setVisible(false);
	this->mainUI.action_Merge_Database->setVisible(false);
	this->mainUI.action_Save_Database->setVisible(false);
	this->mainUI.actionSave_As->setVisible(false);

	this->mainUI.actionImport_Previous_File->setVisible(false);
	this->mainUI.actionImport_Previous_Batch_File->setVisible(false);
	this->mainUI.action_Export_Information->setVisible(false);
	this->mainUI.action_Export->setVisible(false);
	this->mainUI.actionImport_Broken_D_ownloads->setVisible(false); 
	this->mainUI.actionImport_list->setVisible(false);
	this->mainUI.action_Export_list->setVisible(false);

	this->mainUI.action_Recently_Downloaded_Files->setVisible(false);

	//cat
	this->mainUI.action_cat_Move_To->setVisible(false);

	//edit
	this->mainUI.actionFind->setVisible(false);
	this->mainUI.actionFind_next->setVisible(false);

	//view
	this->mainUI.actionDetail->setVisible(false);
	//delete this->mainUI.menuToolbar; this->mainUI.menuToolbar = 0;
	this->mainUI.actionGrid->setVisible(false);
	//this->mainUI.menuSkin->deleteLater();		//这个不能删除，在翻译的时候使用它的指针时就崩溃了。
	//this->mainUI.actionShow_Text->setVisible(false);
	this->mainUI.actionButtons->setVisible(false);	

	//task
	this->mainUI.action_Move_bottom->setVisible(false);
	this->mainUI.action_Move_top->setVisible(false);
	this->mainUI.action_task_Move_To->setVisible(false);
	this->mainUI.actionMove_Down->setVisible(false);
	this->mainUI.actionMove_Up->setVisible(false);
	this->mainUI.action_Check_for_update->setVisible(false);
	
	//tool
	this->mainUI.action_Browse_With_Site_Explorer->setVisible(false);
	this->mainUI.actionRedial_if_Disconnected->setVisible(false);
	this->mainUI.action_Download_Rules->setVisible(false);
	this->mainUI.action_Save_as_default->setVisible(false);
	//this->mainUI.action_Options->setVisible(false);
	this->mainUI.action_Connect_Disconnect->setVisible(false);

	//search
	this->mainUI.action_Software->setVisible(false);
	this->mainUI.action_Game->setVisible(false);
	this->mainUI.action_Page->setVisible(false);
	this->mainUI.action_File->setVisible(false);

	//help
	this->mainUI.action_User_Manual_in_Internet->setVisible(false);

	this->mainUI.action_Manual_NullGet->setVisible(false);
	this->mainUI.actionC_heck_for_a_new_version->setVisible(false);
	this->mainUI.actionFAQ_in_Internet->setVisible(false);

	//other
	this->mainUI.action_Seg_List_Restart->setVisible(false);
}

void NullGet::hideUnneededUiElement()
{

}

/**
 * access private
 */
int NullGet::getNextValidTaskId() 
{
	int taskId = -1;

	SqliteStorage * storage = SqliteStorage::instance(this);
	storage->open();

	taskId = storage->getNextValidTaskID();


	return taskId;

}

/**
 * 第二种实现。
 */
int NullGet::createTask(TaskOption *option)
{
	//precondition: 
	assert(option != 0 );

	int taskId = -1;
	
	taskId = this->getNextValidTaskId();

	//qDebug()<<this->mTaskQueue << __FUNCTION__ << "in " <<__FILE__;
	if (taskId >= 0) {
        // cacl the downloading model index
        QItemSelection readySelect, readDeselect;
        QModelIndex topCatIdx = this->mCatViewModel->index(0, 0);
        qDebug()<<topCatIdx.data();
        int l2RowCount = this->mCatViewModel->rowCount(topCatIdx);
        qDebug()<<l2RowCount;
        for (int r = 0 ; r < l2RowCount ; r ++) {
            QModelIndex currCatIdx = this->mCatViewModel->index(r, ng::cats::cat_id, topCatIdx);
            qDebug()<<currCatIdx;
            if (currCatIdx.data().toInt() == ng::cats::downloading) {
                // for (int c = 0; c <= ng::cats::dirty; c ++) {
                //     QModelIndex readyIndex = this->mCatViewModel->index(r, c, topCatIdx);
                //     readySelect.select(readyIndex, readyIndex);
                // }
                readySelect.select(this->mCatViewModel->index(r, 0, topCatIdx), 
                                   this->mCatViewModel->index(r, ng::cats::dirty, topCatIdx));
                break;
            }
        }
        this->mCatView->selectionModel()->select(readySelect, QItemSelectionModel::ClearAndSelect);
        
		////将任务信息添加到 task list view 中
		//QModelIndex index;
		//QAbstractItemModel * mdl = 0;	//SqliteTaskModel::instance(ng::cats::downloading, this);
		// TaskQueue::addTaskModel(taskId, option);
        this->mTaskMan->addTaskModel(taskId, option);
		//在这里可以查看当前活动的任务数，并看是否需要启动该任务。
		//而在onTaskDone槽中可以查看当前活动的任务数，看是否需要调度执行其他的正在等待的任务。

		// this->onStartTask(taskId);	//为了简单我们直接启动这个任务
		//		
	}		

	return taskId;
}

int NullGet::createTask(int taskId, TaskOption *option)
{
    this->mTaskMan->addTaskModel(taskId, option);
    return taskId;
}

int NullGet::createTaskSegment(int pTaskId ,QString purl , long fromPostion , long yourLength )
{
	
	return -1;
}


void NullGet::onAddTaskList(QStringList list)
{
	////建立任务
	QString taskUrl = list.at(0);

	TaskOption * to = 0;	//创建任务属性。

	taskinfodlg *tid = new taskinfodlg(this);
	tid->setTaskUrl(taskUrl);
	int er = tid->exec();
	//taskUrl = tid->taskUrl();
	//segcnt = tid->segmentCount();	
	//to = tid->getOption();	//			

	if (er == QDialog::Accepted)
	{				
		//this->createTask(url,segcnt);
		//这里先假设用户使用相同的设置。
		//delete to; to = 0;
		for(int t = 0; t <list.size(); ++ t )
		{
			taskUrl = list.at(t);
			qDebug()<<taskUrl;
			tid->setTaskUrl(taskUrl );
			to = tid->getOption();
			this->createTask(to );
		}
	}
	else
	{
		//delete to; to = 0;
	}

	delete tid;
}

void NullGet::testFunc()
{
	qDebug()<<__FUNCTION__;
	int count = 7;	
	this->mTaskPopupMenu->popup(QCursor::pos ());
	return;

	this->onStartTask();
	return;
	
	return;

	QString url = "http://localhost/mtv.wmv";
	int nRet = 0;
	qDebug() <<nRet;

}

void NullGet::testFunc2()
{
	qDebug()<<__FUNCTION__;

	return;

	QStandardItemModel *model = 	mConfigDatabase->createCatModel();	
	this->mCatView->setModel( model );
	this->mCatView->expand(model->index(0,0));

	return;

	this->onPauseTask();

}


void NullGet::onSegmentListSelectChange(const QItemSelection & selected, const QItemSelection & deselected )
{
	qDebug()<<__FUNCTION__;

	int taskId;
	int segId;
	QString segName;

	QModelIndexList mil = selected.indexes();

	taskId = mil.at(1).data().toInt();
	segId = mil.at(0).data().toInt();
	segName = mil.at(2).data().toString();
	qDebug()<<taskId<<segName<<segId;

	//seach log model by taskid and seg id 

	QAbstractItemModel * mdl = SegmentLogModel::instance(taskId , segId, this);

	this->mSegLogListView->setModel(0);
	if (mdl != 0 )
	{
		this->mSegLogListView->setModel(mdl);
		this->mSegLogListView->resizeColumnToContents(2);
		this->mSegLogListView->scrollToBottom();
	}
	else
	{
		qDebug()<<__FUNCTION__<<" model mSegLogListView = 0 ";
	}
}

void NullGet::onTaskListSelectChange(const QItemSelection & selected, const QItemSelection & deselected)
{
	int taskId;
	int segId;
	QString segName;

	//更新JOB相关菜单enable状态。
	this->onUpdateJobMenuEnableProperty();

	///
	QModelIndexList mil = selected.indexes();

	if (selected.size() == 0 ) {
		//不能简单的退出，而应该做点什么吧， 看看它的表现 。
		//在没有任务选中的时候，是否应该清除线程列表及线程日志中的内容。
		return;	//no op needed 
	}

	taskId = mil.at(ng::tasks::task_id).data().toInt();

	qDebug()<<__FUNCTION__<<taskId<<segName;

	QModelIndex index;
	QAbstractItemModel *mdl = 0;

	// update task ball and peer view
    if (this->mTaskMan->isTorrentTask(taskId)) {
        mdl = this->mTaskMan->torrentPeerModel(taskId);
        this->mainUI.peersView->setModel(0);
        this->mainUI.peersView->setModel(mdl);
        mdl = this->mTaskMan->torrentTrackerModel(taskId);
        this->mainUI.trackersView->setModel(mdl);
        mdl = this->mTaskMan->taskSeedFileModel(taskId);
        this->mSeedFileView->setModel(mdl);
    } else {
        mdl = this->mTaskMan->torrentPeerModel(taskId);
        this->mainUI.peersView->setModel(0);
        this->mainUI.peersView->setModel(mdl);
        mdl = this->mTaskMan->torrentTrackerModel(taskId);
        this->mainUI.trackersView->setModel(mdl);
        mdl = this->mTaskMan->taskSeedFileModel(taskId);
        this->mSeedFileView->setModel(mdl);
    }
    {
        mdl = this->mTaskMan->taskServerModel(taskId);
        this->mSegListView->setModel(mdl);
    }

    qDebug()<<__FUNCTION__<<"Ball Ball"<<taskId<<mdl<<(mdl ? mdl->rowCount() : 0);
    TaskBallMapWidget::instance()->onRunTaskCompleteState(taskId, true);

    {
        // update task summary view
        QString fileName = mil.at(ng::tasks::file_name).data().toString();
        QString fileSize = mil.at(ng::tasks::file_size).data().toString();
        QString speed = mil.at(ng::tasks::average_speed).data().toString();
        QString blocks = mil.at(ng::tasks::block_activity).data().toString();
        QString savePath = mil.at(ng::tasks::save_path).data().toString();
        QString refer = mil.at(ng::tasks::referer).data().toString();
        QPair<QString, QString> mimeIconPosition = this->getFileTypeByFileName(fileName);

        this->mainUI.label_2->setText(fileName);
        this->mainUI.label_2->setToolTip(fileName);
        this->mainUI.label_3->setText(fileSize);
        this->mainUI.label_5->setText(speed);
        this->mainUI.label_7->setText(blocks);
        this->mainUI.label_9->setText(QString("<a href=\"%1\">%1</a>").arg(savePath));
        this->mainUI.label_9->setToolTip(QString(tr("Location: %1")).arg(savePath));
        this->mainUI.label_11->setText(QString("<a href=\"%1\">%1</a>").arg(refer));
        this->mainUI.label_11->setToolTip(QString(tr("Location: %1")).arg(refer));

        this->mainUI.label->setPixmap(QPixmap(mimeIconPosition.second).scaled(32, 32));
    }
}

void NullGet::onCatListSelectChange(const QItemSelection & selected, const QItemSelection & deselected )
{
	qDebug()<<__FUNCTION__<<selected;
	
	if (selected.size() == 1) {
		QModelIndex currentIndex;
		currentIndex = selected.at(0).indexes().at(0);
		qDebug()<<currentIndex;
		QModelIndex catIDIndex = selected.at(0).indexes().at(ng::cats::cat_id);
		int catID = catIDIndex.model()->data(catIDIndex ).toInt();

		qDebug()<<"My cat id is: "<<catIDIndex.model()->data(catIDIndex).toString();

		this->mTaskListView->setModel(0);

		if (catIDIndex.model()->data(catIDIndex).toString() == "0") {
			//this->mCatView->clearSelection(); // 不让选择树根也不合适啊。
			QAbstractItemModel *mdl = SqliteTaskModel::instance(catID, this);
			this->mTaskListView->setModel(mdl);
		} else {
			QAbstractItemModel *mdl = SqliteTaskModel::instance(catID, this);
			this->mTaskListView->setModel(mdl);
		}

		QObject::connect(this->mTaskListView->selectionModel(), 
                         SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection &)),
                         this, SLOT(onTaskListSelectChange(const QItemSelection &, const QItemSelection &)));
        
        // clean up
        // qDebug()<<deselected;
        // qDebug()<<deselected.count();
        // qDebug<<deselected.at(0); //.indexes();
        if (deselected.count() > 0) {
            QModelIndex deCatIdIndex = deselected.at(0).indexes().at(ng::cats::cat_id);
            if (deCatIdIndex.data().toInt() == ng::cats::downloading
                || deCatIdIndex.data().toInt() == ng::cats::deleted) {
            } else {
                int deCatId = deCatIdIndex.model()->data(deCatIdIndex).toInt();
                SqliteTaskModel::removeInstance(deCatId);
            }
        }
	}
}

// TODO, handle multi row select case
void NullGet::onStartTask() 
{
	qDebug()<<__FUNCTION__<<__LINE__;	
	QAbstractItemView *view = this->mTaskListView;
    QAbstractItemModel *model = view->model();
	int step = model->columnCount();
    
	int taskId = -1;
    QString url;
	QModelIndexList smil = view->selectionModel()->selectedIndexes();
	//qDebug()<<smil.size();
    if (smil.size() == 0) {
        return;
    }

    TaskOption *taskOptions = TaskOption::fromModelRow(model, smil.at(0).row());
    taskId = smil.value(0 + ng::tasks::task_id).data().toInt();
    url = smil.value(0 + ng::tasks::org_url).data().toString();

    this->initXmlRpc();

    QMap<QString, QVariant> payload;
    QVariantList args;
    QList<QVariant> uris;
    QMap<QString, QVariant> options;
    QString aria2RpcMethod;

    {
        // 先把这个任务对象创建了
        payload["taskId"] = QString("%1").arg(taskId);
        payload["url"] = url;
        this->createTask(taskId, taskOptions);
    }

    if (url.startsWith("file://") && url.endsWith(".torrent")) {
        aria2RpcMethod = QString("aria2.addTorrent");
        
        QFile torrentFile(url.right(url.length() - 7));
        torrentFile.open(QIODevice::ReadOnly);
        QByteArray torrentConntent = torrentFile.readAll();
        torrentFile.close();

        args.insert(0, torrentConntent);
        args.insert(1, uris);

        // options["split"] = QString("5");
        options["split"] = taskOptions->mSplitCount;
        options["dir"] = taskOptions->mSavePath;
        args.insert(2, options);
    } else if (url.startsWith("file://") && url.endsWith(".metalink")) {
        aria2RpcMethod = QString("aria2.addMetalink");
        
        QFile metalinkFile(url.right(url.length() - 7));
        metalinkFile.open(QIODevice::ReadOnly);
        QByteArray metalinkContent = metalinkFile.readAll();
        metalinkFile.close();

        args.insert(0, metalinkContent);
        // options["split"] = QString("5");
        options["split"] = taskOptions->mSplitCount;
        options["dir"] = taskOptions->mSavePath;
        args.insert(1, options);

    } else {
        aria2RpcMethod = QString("aria2.addUri");
        uris << QString(url);
        args.insert(0, uris);
    
        // options["split"] = QString("3");
        options["split"] = taskOptions->mSplitCount;
        options["dir"] = taskOptions->mSavePath;
        args.insert(1, options);
    }

    this->mAriaRpc->call(aria2RpcMethod, args, QVariant(payload),
                         this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
                         this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

    if (!this->mAriaUpdater.isActive()) {
        this->mAriaUpdater.setInterval(3000);
        QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
        this->mAriaUpdater.start();
    }
    if (!this->mAriaTorrentUpdater.isActive()) {
        this->mAriaTorrentUpdater.setInterval(4000);
        QObject::connect(&this->mAriaTorrentUpdater, SIGNAL(timeout()), this, SLOT(onAriaTorrentUpdaterTimeout()));
        this->mAriaTorrentUpdater.start();
    }

    delete taskOptions; taskOptions = NULL;
}

// void NullGet::onStartTask(int pTaskId)
// {
// 	qDebug()<<__FUNCTION__<<__LINE__;

// 	//假设任务的运行句柄即TaskQueue实例不存在
// 	if (this->mTaskMan->containsInstance(pTaskId)) {
// 		// TaskQueue::onStartTask(pTaskId);
// 	} else {
// 		// TaskQueue * hTask = TaskQueue::instance(pTaskId,0);
// 		// QObject::connect(hTask , SIGNAL( onTaskDone(int  ) ), 
// 		// 	this, SLOT(onTaskDone(int ) ) );
// 		// QObject::connect(hTask, SIGNAL(taskCompletionChanged(int  , int  , QString ) ) ,
// 		// 	this->mDropZone,SLOT(onRunTaskCompleteState(int  , int  , QString  ) ) );
// 		// QObject::connect(hTask , SIGNAL(taskCompletionChanged(TaskQueue *  , bool   ) ) ,
// 		// 	TaskBallMapWidget::instance(),SLOT(onRunTaskCompleteState(TaskQueue *  , bool  ) ) );

// 		// TaskBallMapWidget::instance()->onRunTaskCompleteState(hTask , true );

// 		// TaskQueue::onStartTask(pTaskId );
// 		// QObject::connect(hTask, SIGNAL(destroyed(QObject*)), this, SLOT(onObjectDestroyed(QObject*)) );
// 	}

// 	return;
// }

void NullGet::onStartTaskAll()
{
	qDebug()<<__FUNCTION__;

	TaskQueue * hTask = 0;
	int taskCount;
	int retrCount;

	//taskCount = this->mTaskQueue.size();
	//for(int i = 0; i < taskCount; i ++)
	{
		//hTask = this->mTaskQueue[i];
		if (hTask != 0 )
		{
			//this->onStartTask(hTask->mTaskId);
			//this->onStartTask(hTask );
		}
	}
}


void NullGet::onPauseTask() 
{
	qDebug()<<__FUNCTION__;
	
	QAbstractItemView *view = this->mTaskListView;
	QAbstractItemModel *model = view->model(); // SqliteTaskModel::instance(ng::cats::downloading, this);
	int step = model->columnCount();
	int taskId = -1;
    QString ariaGid;
	QModelIndexList smil = view->selectionModel()->selectedIndexes();
	qDebug()<<smil.size();

	if (smil.size() > 0) {
		for (int i = 0; i < smil.size(); i+= step) {
			taskId = smil.value(i).data().toInt();
            ariaGid = smil.value(i + ng::tasks::aria_gid).data().toString();
			qDebug()<<smil.value(i)<<taskId; 
			//this->onPauseTask(taskId);
			// TaskQueue::onPauseTask(taskId);
            Q_ASSERT(this->mAriaRpc != NULL);
            QVariantList args;

            args << ariaGid;
            this->mAriaRpc->call(QString("aria2.remove"), args, QVariant(taskId),
                                 this, SLOT(onAriaRemoveResponse(QVariant &, QVariant&)),
                                 this, SLOT(onAriaRemoveFault(int, QString, QVariant&)));
		}
	}
}
void NullGet::onPauseTask(int pTaskId ) 
{
	qDebug()<<__FUNCTION__;
	// BaseRetriver * hRetr = 0;
	int retrCount;
	TaskQueue * hTask = this->mTaskMan;
	if (hTask == 0)	{
		return;
	} else {
		//this->onPauseTask(hTask);
	}

	return;
}

void NullGet::onPauseTaskAll() 
{
	qDebug()<<__FUNCTION__;
	TaskQueue * hTask = 0;
	int taskCount;

	//taskCount = this->mTaskQueue.size();
	//for(int i = 0; i < taskCount; i ++)
	{
	//	hTask = this->mTaskQueue[i];
		if (hTask != 0 )
		{
		//	this->onPauseTask(hTask->mTaskId);
		}
	}

}

void NullGet::onStartSegment(int pTaskId,int pSegId)
{
	qDebug()<<__FUNCTION__<<pTaskId<<pSegId;
	int row =0;
	// BaseRetriver *br = 0;
	QAbstractItemModel * mdl = 0;

	mdl = SegmentLogModel::instance(pTaskId , pSegId , this );

	// br = TaskQueue::findRetriverById(pTaskId,pSegId);
	// if (br != 0 )
	// {
	// 	br->bePauseing = false;
	// 	row = mdl->rowCount();
	// 	if (row > 0 ) mdl->removeRows(0,row);

	// 	br->start();
	// }
}

/**
 * 触发：用户选择了一个视图中的线程模型，然后点击菜单上启动项
 */

void NullGet::onStartSegment()
{
	qDebug()<<__FUNCTION__;
	int pTaskId = -1;
	int pSegId = -1;
	// BaseRetriver *br = 0;
	int row =0;
	QAbstractItemModel * mdl = 0;

	//if (! this->findRetriverBySelectedModel(&pTaskId,&pSegId)) return;
	int crow;
	QItemSelectionModel *sm = this->mSegListView->selectionModel();
	if (sm == 0 )
	{
		qDebug()<<" no segment model selected";
		return;
	}
	QModelIndex mi = sm->currentIndex();
	qDebug()<<mi;
	if (mi.isValid()) 
	{
		crow = mi.row();
		if (pTaskId != 0 ) pTaskId = this->mSegListView->model()->index(crow,(int)ng::segs::task_id).data().toInt();
		if (pSegId != 0 ) pSegId = this->mSegListView->model()->index(crow,(int)ng::segs::seg_id).data().toInt();
		qDebug()<<pTaskId<<pSegId;

		//br = this->findRetriverById(pTaskId,pSegId);
		// br = TaskQueue::findRetriverById(pTaskId,pSegId);
		// if (br != 0 && br->bePauseing == true )
		// {
		// 	br->bePauseing = false;
		// 	//row = br->mLogModel.rowCount();
		// 	//if (row > 0 ) br->mLogModel.removeRows(0,row);
		// 	row = mdl->rowCount();
		// 	if (row > 0 ) mdl->removeRows(0,row);
		// 	br->start();
		// }		
	}
}

void NullGet::onPauseSegment(int pTaskId,int pSegId)
{
	qDebug()<<__FUNCTION__;

	// BaseRetriver *br = 0;

	// br = TaskQueue::findRetriverById(pTaskId,pSegId);
	// if (br != 0 )
	// {
	// 	br->bePauseing = true;
	// 	br->terminate();
	// }
}

void NullGet::onPauseSegment()
{
	qDebug()<<__FUNCTION__;
	int pTaskId = -1;
	int pSegId = -1;
	// BaseRetriver *br = 0;
		
	int crow;
	QItemSelectionModel *sm = this->mSegListView->selectionModel();
	if (sm == 0 )
	{
		qDebug()<<" no segment model selected";
		return;
	}
	QModelIndex mi = sm->currentIndex();
	qDebug()<<mi;
	// if (mi.isValid()) 
	// {
	// 	crow = mi.row();
	// 	if (pTaskId != 0 ) pTaskId = this->mSegListView->model()->index(crow,ng::segs::task_id).data().toInt();
	// 	if (pSegId != 0 ) pSegId = this->mSegListView->model()->index(crow,ng::segs::seg_id).data().toInt();
	// 	qDebug()<<pTaskId<<pSegId;

	// 	br = TaskQueue::findRetriverById(pTaskId,pSegId);
	// 	if (br != 0 && br->bePauseing == false )
	// 	{
	// 		br->bePauseing = true;	
	// 		br->terminate();
	// 	}	
	// }	
}

/////delete option
/**
 * 将任务数据移动到删除队列
 */
void NullGet::onDeleteTask()
{
	qDebug()<<__FUNCTION__;
	
	SqliteTaskModel * from_model = 0 , * to_model = 0;
	QModelIndex idx;
	QItemSelectionModel * sim = 0;
	QModelIndexList mil;
	int rowCount = 0;
	int row = -1;
	int colcnt = -1;
    QList<int> deleteModelRows;

	from_model = static_cast<SqliteTaskModel*>(this->mTaskListView->model());
    to_model = static_cast<SqliteTaskModel*>(SqliteTaskModel::instance(ng::cats::deleted, 0));

	colcnt = from_model->columnCount();
	sim = this->mTaskListView->selectionModel();
	mil = sim->selectedIndexes();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    int rowcnt = mil.size() / colcnt;
	for (int row = rowcnt - 1; row >= 0; row --) {
        int mrow = mil.value(row  * colcnt + ng::tasks::task_id).row();
        // qDebug()<<"prepare delete ROW:"<<mrow<<" All ROW:"<<rowcnt;
		int taskId = from_model->data(mil.value(row * colcnt + ng::tasks::task_id)).toInt();
		QString ariaGid = from_model->data(mil.value(row * colcnt + ng::tasks::aria_gid)).toString();
        int srcCatId = from_model->data(mil.value(row * colcnt + ng::tasks::sys_cat_id)).toInt();

        // qDebug()<<"DDDD:"<<taskId<<ariaGid<<srcCatId;
        deleteModelRows<<mrow;
        continue;
        
        // QModelIndexList rmil;
        // for (int col = 0; col < colcnt; col ++) {
        //     rmil.append(mil.at(row * colcnt + col));
        // }
        // if (srcCatId == ng::cats::deleted) {
        //     // delete it directly
        //     from_model->removeRows(mil.value(row * colcnt).row(), 1);
		// 	//在system tray 显示移动任务消息
		// 	this->mSysTrayIcon->showMessage(tr("Delete Task ."), QString(tr("Delete permanently. TaskId: %1")).arg(taskId),
        //                                     QSystemTrayIcon::Information, 5000);
        // } else {
        //     int rv = from_model->moveTasks(srcCatId, ng::cats::deleted, rmil);
		// 	//在system tray 显示移动任务消息
		// 	this->mSysTrayIcon->showMessage(tr("Move Task ."), QString(tr("Move To Trash Now. TaskId: %1")).arg(taskId),
        //                                     QSystemTrayIcon::Information, 5000);
        // }
	}

    // because current order depend user's select order, we reorder it here
    qSort(deleteModelRows);
    for (int i = deleteModelRows.count() - 1; i >= 0; --i) {
        int mrow = deleteModelRows.at(i);
        int taskId = from_model->data(from_model->index(mrow, ng::tasks::task_id)).toInt();
        QString ariaGid = from_model->data(from_model->index(mrow, ng::tasks::aria_gid)).toString();
        int srcCatId = from_model->data(from_model->index(mrow, ng::tasks::sys_cat_id)).toInt();
        qDebug()<<"DDDD:"<<mrow<<taskId<<ariaGid<<srcCatId;
        QModelIndexList rmil;
        for (int col = 0; col < colcnt; col ++) {
            rmil.append(from_model->index(mrow, col));
        }
        if (srcCatId == ng::cats::deleted) {
            // delete it directly
            from_model->removeRows(mrow, 1);
			//在system tray 显示移动任务消息
			this->mSysTrayIcon->showMessage(tr("Delete Task ."),
                                            QString(tr("Delete permanently. TaskId: %1")).arg(taskId),
                                            QSystemTrayIcon::Information, 5000);
        } else {
            int rv = from_model->moveTasks(srcCatId, ng::cats::deleted, rmil);
			//在system tray 显示移动任务消息
			this->mSysTrayIcon->showMessage(tr("Move Task ."), QString(tr("Move To Trash Now. TaskId: %1")).arg(taskId),
                                            QSystemTrayIcon::Information, 5000);
        }
        
    }

    QApplication::restoreOverrideCursor();
}

// void NullGet::onDeleteTask(int pTaskId)
// {
// 	qDebug()<<__FUNCTION__;

// 	TaskQueue * tq = this->mTaskMan;
// 	QAbstractItemModel * from_model = 0 , * to_model = 0;
// 	QModelIndex index; 
// 	int row = -1 , colcnt = -1;

// 	from_model = this->mTaskListView->model();
// 	to_model = SqliteTaskModel::instance(ng::cats::deleted, this);

// 	colcnt = from_model->rowCount();

// 	if (tq != 0 )
// 	{
// 		// if (tq->mGotLength < tq->mTotalLength )
// 		// {
// 		// 	int ret = QMessageBox::question(this,tr("Delete task..."),tr("Deleting an unfinished task,are you sure ?") ,QMessageBox::Ok,QMessageBox::Cancel);
// 		// 	if (ret == QMessageBox::Cancel) return;
// 		// }
// 		for(int i = 0; i < from_model->rowCount();  ++ i)
// 		{
// 			if (from_model->data(from_model->index(i,0)).toInt()  == pTaskId )
// 			{
// 				row = i;
// 				break;
// 			}
// 		}
// 		if (row >= 0 )	//找到这个任务的数据模型了。
// 		{

// 			//在system tray 显示移动任务消息
// 			this->mSysTrayIcon->showMessage(tr("Delete Task ."),QString(tr("Move It To Deleted Table Now. TaskId: %1")).arg(pTaskId),QSystemTrayIcon::Information,5000);
			
// 			tq->onTaskListCellNeedChange(pTaskId , ng::tasks::task_status , 
//                                           tq->getStatusString(TaskQueue::TS_COMPLETE));

// 			// //将线程数据移动出来。
// 			// for(int j = 0; j < tq->mSegmentThread.size(); ++j)
// 			// {
// 			// 	tq->mSegmentThread[j]->bePauseing = true;
// 			// 	tq->mSegmentThread[j]->terminate();
// 			// 	//assert(tq->mSegmentThread[j]->isRunning() == false  );
// 			// }
// 			//清除线程窗口列表
// 			this->mSegLogListView->setModel(0);
// 			this->mSegListView->setModel(0);
// 			this->mSegLogListView->setModel(SegmentLogModel::instance(-1,-1, this));
// 			this->mSegListView->setModel(SqliteSegmentModel::instance(-1, this) );

// 			//int idx = this->mTaskQueue.indexOf(tq);

// 			int torows = to_model->rowCount();
// 			to_model->insertRows(torows,1);
			
// 			for(int j = 0; j < colcnt; j ++)
// 			{
// 				index = to_model->index(torows,j);
// 				to_model->setData(index,from_model->data(from_model->index(row,j)) );
// 			}
// 			from_model->removeRows(row,1);
// 			from_model->submit();

// 			to_model->setData(to_model->index(torows,ng::tasks::cat_id),(int)ng::cats::deleted );
// 			to_model->submit();

// 			// tq->mTaskStatus = TaskQueue::TS_COMPLETE;
// 			// tq->mCurrSpeed = 0.0;	//画工具栏上的速度指示图使用该值。

// 			qDebug()<<__FUNCTION__<<" delete task done :"<<tq;

// 		}
// 		else
// 		{
// 			//竟然没有在模型数据中找到，靠，这个是不应该出现的情况。
// 			assert (1== 2 );
// 		}
// 	}	//end if (tq != 0 )
// 	else if (from_model == SqliteTaskModel::instance(ng::cats::deleted, this) )
// 	{
// 		qDebug()<<"cant delete from deletion model";
// 	}
// 	else
// 	{		
// 		//可能是上次没下载完成或者出错的任务，所有还没有任务执行对象
// 		//只需要处理一下模型中数的数据就可以了。
// 		for(int i = 0; i < from_model->rowCount();   i ++ )
// 		{
// 			if (from_model->data(from_model->index(i,0)).toInt()  == pTaskId )
// 			{				
// 				row = i;
// 				break;
// 			}
// 		}
// 		//qDebug()<<__FUNCTION__<<__LINE__<<" delete task done :"<<pTaskId << " at row:"<<row;
		
// 		if (row >= 0 )	//找到这个任务的数据模型了。
// 		{
// 			//在system tray 显示移动任务消息
// 			this->mSysTrayIcon->showMessage(tr("Delete Task ."),QString(tr("Move It To Deleted Table Now. TaskId: %1")).arg(pTaskId),QSystemTrayIcon::Information,5000);

// 			//清除线程窗口列表
// 			this->mSegLogListView->setModel(0);
// 			this->mSegListView->setModel(0);
// 			this->mSegLogListView->setModel(SegmentLogModel::instance(-1,-1, this));
			
// 			this->mSegListView->setModel(SqliteSegmentModel::instance(-1, this) );
			
// 			int torows = to_model->rowCount();
// 			to_model->insertRows(torows,1);

// 			for(int j = 0; j < colcnt; j ++)
// 			{
// 				index = to_model->index(torows,j);
// 				to_model->setData(index,from_model->data(from_model->index(row,j)) );
// 			}

// 			from_model->removeRows(row,1);
// 			from_model->submit();

// 			to_model->setData(to_model->index(torows,ng::tasks::cat_id),(int)ng::cats::deleted );
// 			to_model->submit();

// 			//this->mTaskQueue.remove(idx);
// 			//this->mDoneTaskQueue.append(tq);
// 			//tq->mTaskStatus = TaskQueue::TS_DONE;
// 			//tq->mCurrSpeed = 0.0;	//画工具栏上的速度指示图使用该值。

// 			qDebug()<<__FUNCTION__<<__LINE__<<" delete task done :"<<pTaskId;
// 		}
// 		else
// 		{
// 			//竟然没有在模型数据中找到，靠，这个是不应该出现的情况。
// 			assert (1== 2 );
// 		}
// 	}


// }

void NullGet::onDeleteTaskAll()
{
	qDebug()<<__FUNCTION__;
	QModelIndex index;
	QAbstractItemModel *model;
	int rowCount = -1;

	model = SqliteTaskModel::instance(ng::cats::downloading, this);

	for (int i = 0; i < model->rowCount(); ++ i) {
		index = model->index(i,0);
		int taskId = model->data(index).toInt();
		// this->onDeleteTask(taskId);
	}
}
void NullGet::onDeleteSegment()
{

}
void NullGet::onDeleteSegment(int pTaskId,int pSegId)
{

}

QModelIndex findCatModelIndexByCatId(QAbstractItemModel *mdl, QModelIndex parent, int catId)
{
    QModelIndex idx;
    
    if (parent.isValid()) {
        QModelIndex catIdx = mdl->index(parent.row(), ng::cats::cat_id, parent.parent());
        if (catIdx.data().toInt() == catId) {
            return parent;
        } else {
            int childRowCount = mdl->rowCount(parent);
            QModelIndex childIdx;
            for (int i = 0 ; i < childRowCount; i ++) {
                childIdx = mdl->index(i, 0, parent);
                idx = findCatModelIndexByCatId(mdl, childIdx, catId);
                if (idx.isValid()) {
                    return idx;
                }
            }
        }
    } else {
        QModelIndex topCatIdx = mdl->index(0, 0);
        return findCatModelIndexByCatId(mdl, topCatIdx, catId);
    }
    return idx;
}

void NullGet::onTaskDone(int pTaskId)
{
	qDebug()<<__FUNCTION__;

	SqliteTaskModel *mdl = SqliteTaskModel::instance(ng::cats::downloading);

	//QDateTime bTime , eTime ; 
	//bTime = QDateTime::currentDateTime();
    QModelIndexList mil = mdl->match(mdl->index(0, ng::tasks::task_id), Qt::DisplayRole,
                                     QVariant(QString("%1").arg(pTaskId)), 1, Qt::MatchExactly | Qt::MatchWrap);

    QModelIndex idx;
    int destCatId = 0;
    if (mil.count() == 1) {
        idx = mil.at(0);

        QModelIndexList moveModels;
        for (int i = 0 ; i < mdl->columnCount(); ++i) {
            moveModels << mdl->index(idx.row(), i);
        }

        destCatId = mdl->data(mdl->index(idx.row(), ng::tasks::user_cat_id)).toInt();
        mdl->moveTasks(ng::cats::downloading, destCatId, moveModels);

        // cacl the downloading model index
        QItemSelection readySelect, readDeselect;
        QModelIndex readyIdx = findCatModelIndexByCatId(this->mCatViewModel, QModelIndex(), destCatId);
        qDebug()<<"find cat recursive:"<<destCatId<<readyIdx;
        readySelect.select(this->mCatViewModel->index(readyIdx.row(), 0, readyIdx.parent()), 
                           this->mCatViewModel->index(readyIdx.row(), ng::cats::dirty, readyIdx.parent()));
        
        this->mCatView->selectionModel()->select(readySelect, QItemSelectionModel::ClearAndSelect);
    }
    
    return;
    /////////// 
    TaskQueue *tq = this->mTaskMan;
	QAbstractItemModel * from_model = 0 , * to_model = 0;
	QModelIndex index; 
	int row = -1;

	if (tq != 0) {

		//如果是RTSP、MMS的，需要合并文件操作，这个操作是否应该放在这里完成?		

		from_model = SqliteTaskModel::instance(ng::cats::downloading, this);
		to_model = SqliteTaskModel::instance(ng::cats::downloaded, this);
		int colcnt = from_model->columnCount();
		int from_rowcnt = from_model->rowCount();
		for(int i = 0; i < from_rowcnt; i ++ )
		{
			if (from_model->data(from_model->index(i,ng::tasks::task_id)).toInt() == pTaskId )
			{
				row = i;
				break;
			}
		}
		if (row >= 0 )
		{
			//在system tray 显示移动任务消息
			this->mSysTrayIcon->showMessage(tr("Task Done."),QString(tr("Move It To Done Table Now. TaskId: %1")).arg(pTaskId),QSystemTrayIcon::Information,5000);
			
			tq->onTaskListCellNeedChange(pTaskId , ng::tasks::task_status , tq->getStatusString(TaskQueue::TS_COMPLETE));

			//将线程数据移动出来。
			// for(int j = 0; j < tq->mSegmentThread.size(); ++j)
			// {
			// 	//tq->mSegmentThread[j]->bePauseing = true;
			// 	//tq->mSegmentThread[j]->terminate();
			// 	//assert(tq->mSegmentThread[j]->isRunning() == false  );
			// }
			//清除线程窗口列表
			this->mSegLogListView->setModel(0);
			this->mSegListView->setModel(0);
			this->mSegLogListView->setModel(SegmentLogModel::instance(-1,-1, this));
			this->mSegListView->setModel(SqliteSegmentModel::instance(-1, this) );

			//int idx = this->mTaskQueue.indexOf(tq);

			int torows = to_model->rowCount();
			to_model->insertRows(torows,1);
			
			for(int j = 0; j < colcnt; j ++)
			{
				index = to_model->index(torows,j);
				to_model->setData(index,from_model->data(from_model->index(row,j)) );
			}
			from_model->removeRows(row,1);
			from_model->submit();

			to_model->setData(to_model->index(torows,ng::tasks::user_cat_id),(int)ng::cats::downloaded );
			to_model->submit();

			// tq->mTaskStatus = TaskQueue::TS_COMPLETE;
			// tq->mCurrSpeed = 0.0;	//画工具栏上的速度指示图使用该值。
			
			qDebug()<<__FUNCTION__<<" cleanup task done :"<<tq;
		}	//end if (row >= 0 ) 		

	}
	
}

//////ui op

void NullGet::onNewCategory()
{
	int er;
	CatManDlg * dlg = new CatManDlg (this );
	QAbstractItemModel * aim;
	QItemSelectionModel * ism;
	QModelIndexList mil;

	er = dlg->exec();
	if (er == QDialog::Accepted) {
		QModelIndex idx;
		QString dir;
		QString catname;
		int row;
		
		dir = dlg->getNewCatDir();
		catname = dlg->getNewCatName();

		ism = dlg->getSelectionModel();

		aim = dlg->getCatModel();		
		//qDebug()<<dlg->getCatModel();
		//
		mil = ism->selectedIndexes();
		qDebug()<<mil.size();
		if (mil.size() >  0 ) {
			row = mil.at(0).model()->rowCount(mil.at(0));
			aim->insertRows(row, 1, mil.at(0));
			idx = aim->index(row, ng::cats::display_name, mil.at(0));
			aim->setData(idx, catname);
			idx = aim->index(row, ng::cats::path, mil.at(0));
			aim->setData(idx, dir);
			
			qDebug()<<"insertt "<<row<<aim->data(idx);
			
			//this->mCatView->collapse(mil.at(0));	// open the new cate
			//this->mCatView->expand(mil.at(0));			
			this->mCatView->expandAll();
			this->mCatView->resizeColumnToContents(0);
            aim->submit();
		}
	} else {

	}
	if (dlg != NULL ) delete dlg; dlg = NULL;
}
void NullGet::onShowCategoryProperty()
{
	int er;
	
	QItemSelectionModel * ism = this->mCatView->selectionModel();
	if (ism->selectedIndexes().size() == 0 ) return;
	
	CatPropDlg * dlg = new CatPropDlg (this );

	dlg->setCategoryModel(this->mCatView->selectionModel()->selectedIndexes().at(0));	

	SqliteStorage * storage = SqliteStorage::instance(this);
	int cat_id = this->mCatView->selectionModel()->selectedIndexes().at(ng::cats::cat_id).data().toInt();
	dlg->setCategoryParameters(
		storage->getSubCatCountById(cat_id),		
		storage->getTotalFileCountById(cat_id),
		storage->getDownloadedFileCountById(cat_id),
		storage->getTotalDownloadedLength(cat_id)
		);

	er = dlg->exec();	//显示先

	if (er == QDialog::Accepted )
	{
		//qDebug()<<dlg->getCatModel();
	}
	else
	{

	}
	delete dlg;	
}

void NullGet::onDeleteCategory()
{	
	QItemSelectionModel * ism;
	QModelIndexList mil;
	QModelIndex idx , parent;

	mil = this->mCatView->selectionModel()->selectedIndexes();

	if (mil.size() > 0)
	{
		idx = mil.at(0);
		if (idx == this->mCatViewModel->index(0,0)) return;	//不删除系统默认分类。
		if (idx == this->mCatViewModel->index(0,0, this->mCatViewModel->index(0,0))) return;
		if (idx == this->mCatViewModel->index(1,0, this->mCatViewModel->index(0,0))) return;
		if (idx == this->mCatViewModel->index(2,0, this->mCatViewModel->index(0,0))) return;

		if (QMessageBox::question(this,tr("Delete Category:"),tr("Delete the Category and All sub Category?") ,QMessageBox::Ok,QMessageBox::Cancel ) == QMessageBox::Cancel)
			return;

		parent = idx.parent();
		this->mCatViewModel->removeRows(idx.row(),1,parent);

		//this->mCatView->collapse(parent);
		//this->mCatView->expand(parent);

	}
}

void NullGet::onCategoryMoveTo()
{
	QItemSelectionModel * ism;
	QModelIndexList mil , milto;
	QModelIndex idx , parent;

	mil = this->mCatView->selectionModel()->selectedIndexes();

	if (mil.size() > 0)
	{

		idx = mil.at(0);
		if (idx == this->mCatViewModel->index(0,0)) return;	//不移动系统默认分类。
		if (idx == this->mCatViewModel->index(0,0, this->mCatViewModel->index(0,0))) return;
		if (idx == this->mCatViewModel->index(1,0, this->mCatViewModel->index(0,0))) return;
		if (idx == this->mCatViewModel->index(2,0, this->mCatViewModel->index(0,0))) return;

		CatManDlg * cmd = new CatManDlg(this);
		cmd ->changeMoveToState(mil);

		if (cmd->exec() == QDialog::Accepted)
		{			
			milto = cmd->getSelectionModel()->selectedIndexes();
			if (milto.size() == 0 ) return; // no cat selected
			if (milto == mil ) return;
			//if milto is child of subchild of from , then return; do this	in next line function  .

			//this->mCatViewModel->rowMoveTo(mil.at(0),milto.at(0));

			parent = idx.parent();

			this->mCatView->collapse(parent);
			this->mCatView->expand(parent);
			this->mCatView->collapse(milto.at(0));
			this->mCatView->expand(milto.at(0));
			this->mCatView->resizeColumnToContents(0);

		}
		delete cmd;

	}	
}
void NullGet::onShowColumnEditor()
{
	QDialog * dlg = new ColumnsManDlg(this);

	dlg->exec();

	delete dlg;
}


void NullGet::initXmlRpc()
{
    if (this->mAriaRpc == NULL) {
        // the default port is 6800 on best case, change to 6800+ if any exception.
        this->mAriaRpc = new MaiaXmlRpcClient(QUrl(QString("http://127.0.0.1:%1/rpc").arg(this->mAriaMan->rpcPort())));

        // get version and session
        // use aria2's multicall method. note: has moved to AriaMan
        // QVariantMap  getVersion;
        // QVariantMap getSession;
        // QVariantList gargs;
        // QVariantList args;
        // QVariantMap options;
        // QVariant payload;
        
        // getVersion["methodName"] = QString("aria2.getVersion");
        // getVersion["params"] = QVariant(options);
        // getSession["methodName"] = QString("aria2.getSessionInfo");
        // getSession["params"] = QVariant(options);

        // args.insert(0, getVersion);
        // args.insert(1, getSession);

        // gargs.insert(0, args);

        // this->mAriaRpc->call(QString("system.multicall"), gargs, payload,
        //                      this, SLOT(onAriaMultiCallVersionSessionResponse(QVariant&, QVariant&)),
        //                      this, SLOT(onAriaMultiCallVersionSessionFault(int, QString, QVariant &)));
    }
}

QMap<QString, QVariant> NullGet::taskOptionToAria2RpcOption(TaskOption *to)
{
    Q_ASSERT(to != NULL);
    QMap<QString, QVariant> aopts;

    if (!to->mReferer.isEmpty()) {
        aopts["referer"] = to->mReferer;
    } else {
        aopts["referer"] = to->mTaskUrl;
    }

    if (!to->mSavePath.isEmpty()) {
        aopts["dir"] = to->mSavePath;
    }

    if (to->mSplitCount > 0) {
        aopts["split"] = QString("%1").arg(to->mSplitCount);
    }

    if (!to->mCookies.isEmpty()) {
        QVariantList header;
        header << QString("Cookie: %1").arg(to->mCookies);
        aopts["header"] = header;
    }

    return aopts;
}

QString decodeThunderUrl(QString enUrl)
{
    // like thunder://QUFodHRwOi8vZG93bi41MnouY29tLy9kb3duL3JhbWRpc2sgdjEuOC4yMDAgZm9yIHdpbnhwIMbGveKw5i5yYXJaWg==

    QTextCodec *u8codec = QTextCodec::codecForName("GBK");
	Q_ASSERT(u8codec != NULL);

    QByteArray bUrl = QByteArray::fromBase64(enUrl.right(enUrl.length() - 10).toAscii());
    QString deUrl = (u8codec == NULL) ? bUrl : u8codec->toUnicode(bUrl);
    deUrl = deUrl.mid(2, deUrl.length() - 4);
    return deUrl;
}

QString decodeQQdlUrl(QString enUrl)
{
    // like: qqdl://aHR0cDovL3d3dy5sZXZpbC5jbg== ,  “[url]http://www.levil.cn[/url]”
    QTextCodec *u8codec = QTextCodec::codecForName("GBK");
	Q_ASSERT(u8codec != NULL);

    int pos = enUrl.indexOf("&");
    QString enArgs;
    if (pos != -1) {
        enArgs = enUrl.right(enUrl.length() - pos);
        enUrl = enUrl.left(pos);
    }
    QByteArray bUrl = QByteArray::fromBase64(enUrl.right(enUrl.length() - 7).toAscii());
	
    QString deUrl = (u8codec == NULL) ? bUrl : u8codec->toUnicode(bUrl);
    deUrl = deUrl.mid(7, deUrl.length() - 15);
    return deUrl;
    
    return QString();
}

QString decodeFlashgetUrl(QString enUrl)
{
    // like flashget://W0ZMQVNIR0VUXWZ0cDovL2R5Z29kMTpkeWdvZDFAZDAzMy5keWdvZC5vcmc6MTA1OS8lRTUlQTQlQUElRTUlQjklQjMlRTYlQjQlOEIlRTYlODglOTglRTQlQkElODklNUIxMDI0JUU1JTg4JTg2JUU4JUJFJUE4JUU3JThFJTg3LiVFNCVCOCVBRCVFOCU4QiVCMSVFNSU4RiU4QyVFNSVBRCU5NyU1RC8lNUIlRTclOTQlQjUlRTUlQkQlQjElRTUlQTQlQTklRTUlQTAlODJ3d3cuZHkyMDE4Lm5ldCU1RCVFNSVBNCVBQSVFNSVCOSVCMyVFNiVCNCU4QiVFNiU4OCU5OCVFNCVCQSU4OSVFNyVBQyVBQzA0JUU5JTlCJTg2JTVCJUU0JUI4JUFEJUU4JThCJUIxJUU1JThGJThDJUU1JUFEJTk3JTVELnJtdmJbRkxBU0hHRVRd&18859&1270484198847

    QTextCodec *u8codec = QTextCodec::codecForName("GBK");
	Q_ASSERT(u8codec != NULL);

    int pos = enUrl.indexOf("&");
    QString enArgs;
    if (pos != -1) {
        enArgs = enUrl.right(enUrl.length() - pos);
        enUrl = enUrl.left(pos);
    }
    QByteArray bUrl = QByteArray::fromBase64(enUrl.right(enUrl.length() - 11).toAscii());
	
    QString deUrl = (u8codec == NULL) ? bUrl : u8codec->toUnicode(bUrl);
    deUrl = deUrl.mid(10, deUrl.length() - 20);
    return deUrl;

}

QString decodeEncodeUrl(QString enUrl)
{
    
    if (enUrl.toLower().startsWith("thunder://")) {
        return decodeThunderUrl(enUrl);
    }
    if (enUrl.toLower().startsWith("flashget://")) {
        return decodeFlashgetUrl(enUrl);
    }
    if (enUrl.toLower().startsWith("qqdl://")) {
        return decodeQQdlUrl(enUrl);
    }
    return enUrl;
}

/////////////
/**
 *
 * 创建下载任务输入对话框，接受用户输入，并获取下载信息进行任务的下载。
 * 
 * @bugs: 在主窗口隐藏的时候如果调用该方法，能够弹出输入对话框，但无论是接受还是取消对话框，都会让主程序退出。
 */
void NullGet::showNewDownloadDialog()
{
	QString url;
	int segcnt = 7;
	TaskOption * to = 0;	//创建任务属性。

	taskinfodlg *tid = new taskinfodlg(this);
	int er = tid->exec();
	url = tid->taskUrl();
	segcnt = tid->segmentCount();	

	to = tid->getOption();	//
	delete tid;

    QString deUrl = decodeEncodeUrl(url);
	qDebug()<<segcnt<<url<<deUrl;
    if (url != deUrl) {
        to->setUrl(deUrl);
        url = deUrl;
    }

	if (er == QDialog::Accepted)	{
        int taskId = this->createTask(to);
		qDebug()<<segcnt<<url<<taskId;
        
        this->initXmlRpc();

        // payload type
        QMap<QString, QVariant> payload;
        payload["taskId"] = QString("%1").arg(taskId);
        payload["url"] = url;

        QVariantList args;
        QList<QVariant> uris;
        uris << QString(url);
        args.insert(0, uris);

        QMap<QString, QVariant> options = this->taskOptionToAria2RpcOption(to);
        // options["split"] = QString("2");
        args.insert(1, options);

        this->mAriaRpc->call(QString("aria2.addUri"), args, QVariant(payload),
                  this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
                  this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

        if (!this->mAriaUpdater.isActive()) {
            this->mAriaUpdater.setInterval(3000);
            QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
            this->mAriaUpdater.start();
        }
	} else {
		delete to; to = 0;
	}
	qDebug()<<segcnt<<url;
}

void NullGet::testResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}
void NullGet::testFault(int status, QString response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<status<<response<<payload;
}

void NullGet::onAriaAddUriResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload; // why this line cause crash?

    QMap<QString, QVariant> mPayload = payload.toMap();
    int taskId = mPayload["taskId"].toString().toInt();
    QString url = mPayload["url"].toString();
    QString cmd = mPayload["cmd"].toString();

    this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::aria_gid, response.toString());

    this->mRunningMap[taskId] = response.toString();
    // if is torrent, add to torrentMap
    if (url.toLower().endsWith(".torrent")) {
        this->mTorrentMap[taskId] = response.toString();
     }
    
}
void NullGet::onAriaAddUriFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<reason;
}

void NullGet::onAriaGetUriResponse(QVariant &response, QVariant &payload)
{
    
    qDebug()<<__FUNCTION__<<response;
}
void NullGet::onAriaGetUriFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason;
}

/*
QVariant(QVariantMap, QMap(("bitfield", QVariant(QString, "0000") ) ("completedLength" ,  QVariant(QString, "1769472") ) ("connections" ,  QVariant(QString, "2") ) ("dir" ,  QVariant(QString, "/home/gzleo/nullget-svn") ) ("downloadSpeed" ,  QVariant(QString, "35243") ) ("files" ,  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("index", QVariant(QString, "1") ) ("length" ,  QVariant(QString, "13910775") ) ("path" ,  QVariant(QString, "/home/gzleo/nullget-svn/postgresql-9.0alpha5.tar.bz2") ) ("selected" ,  QVariant(QString, "true") ) ("uris" ,  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("status", QVariant(QString, "used") ) ("uri" ,  QVariant(QString, "http://wwwmaster.postgresql.org/redir/394/h/source/9.0alpha5/postgresql-9.0alpha5.tar.bz2") ) )  ) ,  QVariant(QVariantMap, QMap(("status", QVariant(QString, "used") ) ("uri" ,  QVariant(QString, "http://wwwmaster.postgresql.org/redir/394/h/source/9.0alpha5/postgresql-9.0alpha5.tar.bz2") ) )  ) )  ) ) )  ) )  ) ) ("gid" ,  QVariant(QString, "1") ) ("numPieces" ,  QVariant(QString, "14") ) ("pieceLength" ,  QVariant(QString, "1048576") ) ("status" ,  QVariant(QString, "active") ) ("totalLength" ,  QVariant(QString, "13910775") ) ("uploadLength" ,  QVariant(QString, "0") ) ("uploadSpeed" ,  QVariant(QString, "0") ) )  )
 */


void NullGet::onAriaGetStatusResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;

    int taskId = payload.toInt();
    QVariantMap sts = response.toMap();

    this->mTaskMan->onTaskStatusNeedUpdate(taskId, sts);

    // qDebug()<<sts["files"];

    if (sts.contains("bittorrent")) {
        QVariantMap stsbt = sts.value("bittorrent").toMap();
        QVariantList stsTrackers = stsbt.value("announceList").toList();
        this->mTaskMan->setTrackers(taskId, stsTrackers);
    } 

    if (sts["status"].toString() == QString("complete")) {
        if (this->mTaskMan->isTorrentTask(taskId)) {
            this->mTorrentMap.remove(taskId);
        }
        this->mRunningMap.remove(taskId);

        this->onTaskDone(taskId);
    }

    if (sts["status"].toString() == "error") {
        // 
    }
}
void NullGet::onAriaGetStatusFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason;
}

void NullGet::onAriaUpdaterTimeout()
{
    // qDebug()<<"time out update";
    QHashIterator<int, QString> hit(this->mRunningMap);

    if (this->mAriaRpc == NULL) {
        Q_ASSERT(this->mAriaRpc != NULL);
    }
    QVariantList args;
    int taskId;
    QString ariaGid;
    
    while (hit.hasNext()) {
        hit.next();
        taskId = hit.key();
        ariaGid = hit.value();

        args<<ariaGid;
        this->mAriaRpc->call(QString("aria2.tellStatus"), args, QVariant(taskId),
                             this, SLOT(onAriaGetStatusResponse(QVariant &, QVariant &)),
                             this, SLOT(onAriaGetStatusFault(int, QString, QVariant &)));
        args.clear();

        // 
        args<<ariaGid;
        this->mAriaRpc->call(QString("aria2.getServers"), args, QVariant(taskId),
                             this, SLOT(onAriaGetServersResponse(QVariant &, QVariant &)),
                             this, SLOT(onAriaGetServersFault(int, QString, QVariant &)));
        args.clear();
    }
}

void NullGet::onAriaRemoveResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
    int taskId = payload.toInt();
    this->mRunningMap.remove(taskId);

    // tq = this->mTaskMan->findTaskById(taskId);
    this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::task_status, QString(tr("pause")));
    this->mTaskMan->onPauseTask(taskId);
}

void NullGet::onAriaRemoveFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
    // assert(1 == 2);
}

void NullGet::onAriaGlobalUpdaterTimeout()
{
    QVariantList args;
    QVariant payload;

    this->initXmlRpc();
    this->mAriaRpc->call(QString("aria2.tellActive"), args, payload,
                         this, SLOT(onAriaGetActiveResponse(QVariant&, QVariant&)),
                         this, SLOT(onAriaGetActiveFault(int, QString, QVariant &)));
}

void NullGet::onAriaGetActiveResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    QVariantList lsts = response.toList();
    QVariantMap  sts;
    int speed = 0;
    quint64 totalLength = 0;
    quint64 gotLength = 0;

    for (int i = 0; i < lsts.size(); i++) {
        sts = lsts.at(i).toMap();
        speed += sts["downloadSpeed"].toInt();
        totalLength += sts["totalLength"].toULongLong();
        gotLength += sts["completedLength"].toULongLong();
    }

    qDebug()<<"TSpeed:"<<speed<<" TLen:"<<totalLength<<" GLen:"<<gotLength;
    
	double sumSpeed =speed*1.0/1000;
	if (sumSpeed >= 0.0) {
		this->mSpeedTotalLable->setText(QString("%1 KB/s").arg(sumSpeed));
		this->mSpeedProgressBar->setValue((int)sumSpeed);
	} else {
		this->mSpeedTotalLable->setText(QString("%1 B/s").arg(speed));
		this->mSpeedProgressBar->setValue((int)sumSpeed);
    }

	this->mISHW->updateSpeedHistogram(sumSpeed);
}

void NullGet::onAriaGetActiveFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void NullGet::onAriaGetServersResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;

    int taskId = payload.toInt();
    TaskServerModel *serverModel = this->mTaskMan->taskServerModel(taskId);
    if (serverModel) {
        QVariantList servers = response.toList();
        serverModel->setData(servers);
        // QTreeView::rowsInserted internal representation of the model has been corrupted, resetting. 
    } else {
        qDebug()<<"server model not found";
    }
}

void NullGet::onAriaGetServersFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void NullGet::onAriaGetTorrentPeersResponse(QVariant &response, QVariant &payload)
{
    int taskId = payload.toInt();

    QVariantList peers = response.toList();
    this->mTaskMan->setPeers(taskId, peers);
    // qDebug()<<__FUNCTION__<<response<<payload;
}
void NullGet::onAriaGetTorrentPeersFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void NullGet::onAriaParseTorrentFileResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
    /*
      onAriaGetTorrentFilesResponse QVariant(QString, "1") QVariant(QVariantMap, QMap(("cmd", QVariant(QString, "torrent_get_files") ) ("taskId" ,  QVariant(int, 42) ) ("url" ,  QVariant(QString, "file:///home/gzleo/NGDownload/1CA79A2AD33A0C89157E2BE71A0AF8A1A735A83A.torrent") ) )  ) 
     */
    QString ariaGid = response.toString();
    QMap<QString, QVariant> mPayload = payload.toMap();
    int taskId = mPayload["taskId"].toInt();
    QString url = mPayload["url"].toString();
    mPayload["ariaGid"] = ariaGid;
    // QString fileName = url.right(url.length() - 7);
    // TaskOption *to = new TaskOption(); // TODO get option from GlobalOption
    // to->setDefaultValue();
    // to->setUrl("file://" + url); //转换成本地文件协议

    // int taskId = this->createTask(to);
    qDebug()<<url<<taskId;
        
    this->initXmlRpc();

    // payload["taskId"] = taskId;
    // payload["url"] = "file://" + url;
    // payload["cmd"] = QString("torrent_get_files");

    // QFile torrentFile();
    // torrentFile.open(QIODevice::ReadOnly);
    // QByteArray torrentConntent = torrentFile.readAll();
    // torrentFile.close();

    QVariantList args;
    args.insert(0, ariaGid);

    // now use tellStatus to get files and torrent file info
    this->mAriaRpc->call(QString("aria2.tellStatus"), args, QVariant(mPayload),
                         this, SLOT(onAriaGetTorrentFilesResponse(QVariant &, QVariant &)),
                         this, SLOT(onAriaGetTorrentFilesFault(int, QString, QVariant &)));

    // this->mAriaRpc->call(QString("aria2.getFiles"), args, QVariant(mPayload),
    //                      this, SLOT(onAriaGetTorrentFilesResponse(QVariant &, QVariant &)),
    //                      this, SLOT(onAriaGetTorrentFilesFault(int, QString, QVariant &)));
    
}

void NullGet::onAriaParseTorrentFileFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
    // onAriaParseTorrentFileFault 1 "We encountered a problem while processing the option '--select-file'."
    /*
      2010-04-21 10:44:24.049116 ERROR - Exception caught
      Exception: [RequestGroup.cc:324] File /home/gzleo/karia2-svn/Kansas - Monolith [1979] exists, but a control file(*.aria2) does not exist. Download was canceled in order to prevent your file from being truncated to 0. If you are sure to download the file all over again, then delete it or add --allow-overwrite=true option and restart aria2.
     */
}

void NullGet::onAriaGetTorrentFilesResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    QMap<QString, QVariant> mPayload = payload.toMap();
    int taskId = payload.toMap().value("taskId").toInt();
    QMap<QString, QVariant> statusMap = response.toMap();
    QVariantList files = statusMap["files"].toList(); // response.toList();

    SeedFilesDialog *fileDlg = new SeedFilesDialog();
    fileDlg->setFiles(files, true);
    fileDlg->setTorrentInfo(statusMap, statusMap.value("bittorrent").toMap());
    int rv = fileDlg->exec();
    
    if (rv == QDialog::Accepted) {
        //remove the unused aria2 task
        mPayload["indexes"] = fileDlg->getSelectedFileIndexes();
        mPayload["removeConfirm"] = "no";
        QVariantList args;
        args << payload.toMap().value("ariaGid");

        this->mAriaRpc->call(QString("aria2.remove"), args, QVariant(mPayload),
                             this, SLOT(onAriaRemoveTorrentParseFileTaskResponse(QVariant &, QVariant&)),
                             this, SLOT(onAriaRemoveTorrentParseFileTaskFault(int, QString, QVariant&)));

        // args.clear();
        // this->mAriaRpc->call(QString("aria2.purgeDownloadResult"), args, QVariant(taskId),
        //                      this, SLOT(onAriaGetVersionResponse(QVariant &, QVariant &)),
        //                      this, SLOT(onAriaGetVersionFault(int, QString, QVariant&)));

        
    }
}

void NullGet::onAriaGetTorrentFilesFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void NullGet::onTorrentRemoveConfirmTimeout()
{
    QTimer *timer = (QTimer*)(sender());  
    QVariant vtimer = QVariant(qVariantFromValue((QObject*)timer));
    timer = (QTimer*)(vtimer.value<QObject*>());
    QVariant payload = this->mTorrentWaitRemoveConfirm[timer];
    QMap<QString, QVariant> mPayload = payload.toMap();

    QString ariaGid = mPayload["ariaGid"].toString();

    QVariantList args;
    args << ariaGid;

    this->mAriaRpc->call(QString("aria2.tellStatus"), args, payload,
                         this, SLOT(onAriaRemoveGetTorrentFilesConfirmResponse(QVariant&, QVariant&)),
                         this, SLOT(onAriaRemoveGetTorrentFilesConfirmFault(int, QString, QVariant &)));
}

void NullGet::onAriaRemoveGetTorrentFilesConfirmResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    QVariantMap msts = response.toMap();
    QVariantMap mPayload = payload.toMap();

    if (msts.value("status").toString() == "removed") {
        mPayload["removeConfirm"] = "yes";
        QVariant aPayload = QVariant(mPayload);
        this->onAriaRemoveTorrentParseFileTaskResponse(response, aPayload);
        // delete no used timer and temporary data
        QTimer *timer = (QTimer*)(mPayload.value("confirmTimer").value<QObject*>());
        QVariant tPayload = this->mTorrentWaitRemoveConfirm[timer];
        this->mTorrentWaitRemoveConfirm.remove(timer);
        delete timer;
    } else {
        QTimer *timer = (QTimer*)(mPayload.value("confirmTimer").value<QObject*>());
        timer->start();
    }
}

void NullGet::onAriaRemoveGetTorrentFilesConfirmFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void NullGet::onAriaRemoveTorrentParseFileTaskResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;

    // insert new torrent task
    QMap<QString, QVariant> mPayload = payload.toMap();
    QString indexList = mPayload["indexes"].toString();
    QString url = mPayload["url"].toString();
    QString removeConfirm = mPayload["removeConfirm"].toString();

    if (removeConfirm != "yes") {
        QTimer *timer = new QTimer(); timer->setSingleShot(true); timer->setInterval(500);
        QObject::connect(timer, SIGNAL(timeout()), 
                         this, SLOT(onTorrentRemoveConfirmTimeout()));
        mPayload["confirmTimer"] = qVariantFromValue((QObject*)timer);
        this->mTorrentWaitRemoveConfirm[timer] = QVariant(mPayload);
        timer->start();
        return;
    }

    this->initXmlRpc();

    QFile torrentFile(url.right(url.length() - 7));
    torrentFile.open(QIODevice::ReadOnly);
    QByteArray torrentConntent = torrentFile.readAll();
    torrentFile.close();

    QVariantList args;
    QList<QVariant> uris;

    args.insert(0, torrentConntent);
    args.insert(1, uris);

    QMap<QString, QVariant> options;
    // options["split"] = QString("1");
    options["select-file"] = indexList;
    args.insert(2, options);
    args.insert(3, QVariant(0));

    this->mAriaRpc->call(QString("aria2.addTorrent"), args, QVariant(payload),
                         this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
                         this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

    if (!this->mAriaUpdater.isActive()) {
        this->mAriaUpdater.setInterval(3000);
        QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
        this->mAriaUpdater.start();
    }
        
    if (!this->mAriaTorrentUpdater.isActive()) {
        this->mAriaTorrentUpdater.setInterval(4000);
        QObject::connect(&this->mAriaTorrentUpdater, SIGNAL(timeout()), this, SLOT(onAriaTorrentUpdaterTimeout()));
        this->mAriaTorrentUpdater.start();
    }

    // set seed file model data
    int taskId = mPayload["taskId"].toString().toInt();
    QVariantList seedFiles = response.toMap().value("files").toList();
    this->mTaskMan->setSeedFiles(taskId, seedFiles);
}

void NullGet::onAriaRemoveTorrentParseFileTaskFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void NullGet::onAriaTorrentUpdaterTimeout()
{
    this->initXmlRpc();
    QVariantList args;
    QHashIterator<int, QString> mit(this->mTorrentMap);
    while(mit.hasNext()) {
        mit.next();
        
        args<<mit.value();
        this->mAriaRpc->call(QString("aria2.getPeers"), args, QVariant(mit.key()),
                             this, SLOT(onAriaGetTorrentPeersResponse(QVariant &, QVariant&)),
                             this, SLOT(onAriaGetTorrentPeersFault(int, QString, QVariant&)));
    }
}

void NullGet::onAriaGetVersionResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}
void NullGet::onAriaGetVersionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}
void NullGet::onAriaGetSessionInfoResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}
void NullGet::onAriaGetSessionInfoFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void NullGet::onAriaMultiCallVersionSessionResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}

void NullGet::onAriaMultiCallVersionSessionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void NullGet::onAriaChangeGlobalOptionResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    QString which = payload.toString();
    if (which == "max-overall-download-limit") {

    } else if (which == "max-concurrent-downloads") {

    } else if (which == "max-overall-upload-limit") {
        
    } else {
        Q_ASSERT(1 == 2);
    }

    // for debug, see the change's response result. no use now
    // QVariantList args;
    // this->mAriaRpc->call("aria2.getGlobalOption", args, payload,
    //                      this, SLOT(onAriaGetGlobalOptionResponse(QVariant &, QVariant &)),
    //                      this, SLOT(onAriaGetGlobalOptionFault(int, QString, QVariant &)));
}

void NullGet::onAriaChangeGlobalOptionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void NullGet::onAriaGetGlobalOptionResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}

void NullGet::onAriaGetGlobalOptionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}


void NullGet::showNewBittorrentFileDialog()
{
    QString url;
    
    url = QFileDialog::getOpenFileName(this, tr("Open a .torrent file..."), QString(),
                                       tr("Torrent File (*.torrent)"), NULL, 0);

    if (url == QString::null) {
        // cancel
    } else if (url.isEmpty()) {
        // select 0 file?
    } else {
        TaskOption *to = new TaskOption(); // TODO get option from GlobalOption
        to->setDefaultValue();
        to->mCatId = ng::cats::downloading;
        to->setUrl("file://" + url); //转换成本地文件协议

        int taskId = this->createTask(to);
		qDebug()<<__FUNCTION__<<url<<taskId;
        
        this->initXmlRpc();

        QMap<QString, QVariant> payload;
        payload["taskId"] = taskId;
        payload["url"] = "file://" + url;
        payload["cmd"] = QString("torrent_get_files");

        QFile torrentFile(url);
        torrentFile.open(QIODevice::ReadOnly);
        QByteArray torrentConntent = torrentFile.readAll();
        torrentFile.close();

        QVariantList args;
        QList<QVariant> uris;

        args.insert(0, torrentConntent);
        args.insert(1, uris);

        QMap<QString, QVariant> options;
        // options["split"] = QString("1");
        options["bt-max-peers"] = QString("1");
        options["all-proxy"] = QString("127.0.0.1:65532"); // use a no usable proxy, let aria2 stop quick
        options["select-file"] = QString("1");
        args.insert(2, options);

        this->mAriaRpc->call(QString("aria2.addTorrent"), args, QVariant(payload),
                  this, SLOT(onAriaParseTorrentFileResponse(QVariant &, QVariant &)),
                  this, SLOT(onAriaParseTorrentFileFault(int, QString, QVariant &)));

        // this->mAriaRpc->call(QString("aria2.addTorrent"), args, QVariant(payload),
        //           this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
        //           this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

        // if (!this->mAriaUpdater.isActive()) {
        //     this->mAriaUpdater.setInterval(3000);
        //     QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
        //     this->mAriaUpdater.start();
        // }
        
        // if (!this->mAriaTorrentUpdater.isActive()) {
        //     this->mAriaTorrentUpdater.setInterval(4000);
        //     QObject::connect(&this->mAriaTorrentUpdater, SIGNAL(timeout()), this, SLOT(onAriaTorrentUpdaterTimeout()));
        //     this->mAriaTorrentUpdater.start();
        // }
    }
}

void NullGet::showNewMetalinkFileDialog()
{
    QString url;
    
    url = QFileDialog::getOpenFileName(this, tr("Open a .metalink file..."), QString(),
                                       tr("Metalink File (*.metalink)"), NULL, 0);

    if (url == QString::null) {
        // cancel
    } else if (url.isEmpty()) {
        // select 0 file?
    } else {
        TaskOption *to = new TaskOption(); // TODO get option from GlobalOption
        to->setDefaultValue();
        to->setUrl("file://" + url); //转换成本地文件协议

        int taskId = this->createTask(to);
		qDebug()<<url<<taskId;
        
        this->initXmlRpc();

        QMap<QString, QVariant> payload;
        payload["taskId"] = taskId;
        payload["url"] = "file://" + url;

        QFile metalinkFile(url);
        metalinkFile.open(QIODevice::ReadOnly);
        QByteArray metalinkConntent = metalinkFile.readAll();
        metalinkFile.close();

        QVariantList args;

        args.insert(0, metalinkConntent);

        QMap<QString, QVariant> options;
        options["split"] = QString("5");
        args.insert(1, options);

        this->mAriaRpc->call(QString("aria2.addMetalink"), args, QVariant(payload),
                  this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
                  this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

        if (!this->mAriaUpdater.isActive()) {
            this->mAriaUpdater.setInterval(3000);
            QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
            this->mAriaUpdater.start();
        }
        
        if (!this->mAriaTorrentUpdater.isActive()) {
            this->mAriaTorrentUpdater.setInterval(4000);
            QObject::connect(&this->mAriaTorrentUpdater, SIGNAL(timeout()), this, SLOT(onAriaTorrentUpdaterTimeout()));
            this->mAriaTorrentUpdater.start();
        }
    }
}

void NullGet::showBatchDownloadDialog()
{
	QString url;
	int segcnt = 7;
	TaskOption * to = 0;	//创建任务属性。
	QStringList sl;
	
	BatchJobManDlg *bjd = new BatchJobManDlg(this);
	int er = bjd->exec();
	sl = bjd->getUrlList();

	delete bjd;

	if (er == QDialog::Accepted)
	{		
		qDebug()<<segcnt<<url;
		taskinfodlg *tid = new taskinfodlg(this);
		for(int i = 0; i < sl.count(); ++i)
		{
			tid->setTaskUrl(sl.at(i));
			to = tid->getOption();
			this->createTask(to );
		}
		delete tid;	
	}
	else
	{
		delete to; to = 0;
	}	
}

void NullGet::showProcessWebPageInputDiglog()	//处理WEB页面，取其中链接并下载
{
	QStringList urlList , srcList ,resultList;
	QString url;
	QUrl u;
	QString htmlcode;
	int er = QDialog::Rejected;
	int ulcount;
	int linkCount = 0;
	char ** linkList = 0;

	WebPageUrlInputDlg * wpid = 0;
	WebPageLinkDlg * wpld  = 0;

	wpid = new WebPageUrlInputDlg(this);
	er = wpid->exec();
	if (er == QDialog::Accepted )
	{
		url = wpid->getLinkList();
		urlList = url.split("\"+\"");
		ulcount = urlList.size();
		for(int i = 0; i < ulcount; ++i)
		{
			if (i == 0 && urlList.at(0).startsWith("\""))
			{
				urlList[0] = urlList.at(0).right(urlList.at(0).length()-1);
			}
			if (i == ulcount-1 && urlList.at(i).endsWith("\""))
			{
				urlList[i] = urlList.at(i).left(urlList.at(i).length()-1);
			}
		}
		qDebug()<<urlList;

		if (urlList.size() > 0 )
		{
			for(int i = 0; i < ulcount; ++i)
			{
				u = urlList.at(i);
				if (u.scheme().toUpper().compare("HTTP") == 0
					|| u.scheme().toUpper().compare("HTTPS") == 0 )
				{
					//是否需要我们自己从HTTP服务器读取文件内容呢。
					//windows 不需要.UNIX的各资源管理器还不清楚是否支持打开HTTP的功能。

				}
				else if (u.scheme().toUpper().length() == 1 )	// C D E and so on	for win , / for unix
				{
					QFile f(urlList.at(0));
					f.open(QIODevice::ReadOnly);
					htmlcode = QString(f.readAll().data());
					f.close();
				}
				else
				{
					//not supported page file access way
				}
				//test
				linkCount = 0;
				linkList = html_parse_get_all_link(htmlcode.toAscii().data() , &linkCount );
				qDebug()<<linkCount;
				for(int j = 0; j < linkCount; j ++ )
				{
					srcList.append(QString(linkList[j]));
				}
				html_parse_free_link_mt(linkList,linkCount);
			}	//end for(int i = 0; i < ulcount; ++i)
		}

		//////
		if (srcList.size() > 0 )
		{
			wpld = new WebPageLinkDlg(this);
			wpld->setSourceUrlList(srcList);
			if (wpld->exec() == QDialog::Accepted)
			{
				resultList = wpld->getResultUrlList();
				qDebug()<<resultList;

			}
			delete wpld; wpld = 0;
		}
		else // no url found in user input webpage file
		{
			QMessageBox::warning(this,tr("Process Weg Page :"),tr("No URL(s) Found In the Weg Page File/Given URL"));
		}
		
		///建立新任务。
		if (resultList.size() > 0 )
		{
			////建立任务
			QString taskUrl = resultList.at(0);

			TaskOption * to = 0;	//创建任务属性。

			taskinfodlg *tid = new taskinfodlg(this);
			tid->setTaskUrl(taskUrl);
			int er = tid->exec();
			//taskUrl = tid->taskUrl();
			//segcnt = tid->segmentCount();	
			//to = tid->getOption();	//			

			if (er == QDialog::Accepted)
			{				
				//this->createTask(url,segcnt);
				//这里先假设用户使用相同的设置。
				//delete to; to = 0;
				for(int t = 0; t < resultList.size(); ++ t )
				{
					taskUrl = resultList.at(t);
					qDebug()<<taskUrl;
					tid->setTaskUrl(taskUrl );
					to = tid->getOption();
					this->createTask(to );
				}
			}
			else
			{
				//delete to; to = 0;
			}

			delete tid;
		}	//end if (resultList.size() > 0 )
		else
		{
			QMessageBox::warning(this,tr("Process Weg Page :"),tr("No URL(s) Selected"));
		}	//end if (resultList.size() > 0 ) else 
	}

	if (wpid != 0 ) delete wpid;
	if (wpld != 0 ) delete wpld;
}

void NullGet::onShowOptions()
{
	//OptionDlg *dlg = new OptionDlg(this);

	//dlg->exec();

	//delete dlg;
	QDialog * dlg = new PreferencesDialog(this);

	dlg->exec();

	delete dlg;

}
void NullGet::onShowConnectOption()
{
	OptionDlg *dlg = new OptionDlg(this);

	dlg->exec();

	delete dlg;
}

// void NullGet::onShowDefaultDownloadProperty()
// {
// 	taskinfodlg *tid = new taskinfodlg(0,0);		
	
// 	//tid->setRename(fname);			
// 	int er = tid->exec();	
	
// 	delete tid;	
// }

void NullGet::onShowTaskProperty()
{
	qDebug()<<__FUNCTION__;
	QItemSelectionModel * sim;
	TaskQueue *tq = NULL;
	QModelIndexList mil;

	//在运行队列中查找。
	mil  = this->mTaskListView->selectionModel()->selectedIndexes();
	if (this->mTaskListView->model() != 0 &&
		mil.size() == this->mTaskListView->model()->columnCount()) {
		//only selected one ,可以处理，其他多选的不予处理。
		int taskId = mil.at(0).data().toInt();
		// tq = this->mTaskMan->findTaskById(taskId);
		if (tq == 0) {
			// tq = this->mTaskMan->findTaskById(taskId);
		}
	}

	//根据不同的位置显示不同的任务属性。
    if (tq != 0) {
        // QString url = tq->mTaskUrl;
        // int segcnt = tq->mTotalSegmentCount;
        QString url;
        int segcnt;
        QString fname;//= tq->mTaskOption->mSaveName;
		
        taskinfodlg *tid = new taskinfodlg(0, this);
        tid->setTaskUrl(url);
        tid->setSegmentCount(segcnt);
        tid->setRename(fname);
		
        int er = tid->exec();			
		
        delete tid;
    }
}
void NullGet::onShowTaskProperty(int pTaskId)
{

}

void NullGet::onShowTaskPropertyDigest(const QModelIndex & index )
{
	//qDebug()<<__FUNCTION__ << index.data();
	int taskId;
	this->mSwapPoint = QCursor::pos();
	this->mSwapModelIndex = index;
	QTimer::singleShot(1000, this, SLOT(onShowTaskPropertyDigest()));
}

void NullGet::onShowTaskPropertyDigest( )
{
	//qDebug()<<__FUNCTION__;
	QString tips = tr("<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:宋体; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\"><table  width=\"100%\"  height=\"100%\" border=\"1\">  <tr>    <td width=\"97\">&nbsp;<img name=\"\" src=\"%1\" width=\"80\" height=\"80\" alt=\"\"></td>    <td  height=\"100%\" ><b>%2</b><br>-------------------------------<br>File Size: %3<br>File Type: .%4<br>Completed: %5<br>-------------------------------<br>Save Postion: %6<br>URL: %7<br>Refferer: %8<br>Comment: %9<br>-------------------------------<br>Create Time: %10<br>------------------------------- </td>  </tr></table></body></html>");
	QPoint np = this->mainUI.mui_tv_task_list->viewport()->mapFromGlobal(QCursor::pos());
	QModelIndex nidx = this->mainUI.mui_tv_task_list->indexAt(np);
	QModelIndex tidx;
	TaskQueue * tq = 0;
	int catId = -1;
	
	if (this->mainUI.mui_tv_task_list->viewport()->underMouse()&&
		this->mSwapModelIndex.isValid() && nidx.isValid() && nidx == this->mSwapModelIndex ) {
		int row = nidx.row();
		QString id , url , ftype, name , path , gotlength , totallength , refferer, comment ,createtime  , fimg;
		tidx = nidx.model()->index(row,0);
		id = tidx.data().toString();
		//tq = TaskQueue::findTaskById(id.toInt());
		//if (tq != 0   )
		//{
		//	//path = tq->mTaskOption->mSavePath;
		//}
		//else
		//{
		//	path = ".";
		//}
		tidx = nidx.model()->index(row,ng::tasks::user_cat_id);
		path = SqliteStorage::instance(this)->getSavePathByCatId(tidx.data().toInt());

		tidx = nidx.model()->index(row, ng::tasks::org_url);
		url = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::file_name);
		name = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::abtained_length);
		gotlength = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::file_size);
		totallength = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::referer);
		refferer = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::comment);
		comment = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::create_time);
		createtime = tidx.data().toString();

		//ftype = "unknown";
		//fimg = "cubelogo.png";
		QPair<QString , QString> guessedFileType = this->getFileTypeByFileName(name);
		ftype = guessedFileType.first;
		fimg = guessedFileType.second;

		//qDebug()<<"show digest"<<this->mSwapModelIndex.data();
		//QToolTip * tt = new QToolTip();
		tips = tips.arg(fimg);
		tips = tips.arg(name);
		tips = tips.arg(totallength);
		tips = tips.arg(ftype);		//get file type by file name , or get file type by file content , later 
		tips = tips.arg(gotlength);
		tips = tips.arg(path);
		tips = tips.arg(url);
		tips = tips.arg(refferer);
		tips = tips.arg(comment);
		tips = tips.arg(createtime);
		QToolTip::showText(QCursor::pos(),tips );
		//delete tt;
	}
	this->mSwapPoint = QPoint(0,0);
	this->mSwapModelIndex = QModelIndex();	
}

QPair<QString, QString> NullGet::getFileTypeByFileName(QString fileName)
{
	QPair<QString,QString> ftype("unknown", "unknown.png");
	
#if defined(Q_OS_WIN)
	QString path = "icons/crystalsvg/128x128/mimetypes/";
#elif defined(Q_OS_MAC)
    QString path = "icons/crystalsvg/128x128/mimetypes/";
#else // *nix
    QString path = QString("/usr/share/icons/%1/128x128/mimetypes/").arg(QIcon::themeName());
#endif
	QStringList fileParts = fileName.toLower().split(".");
    QString fileExtName = fileParts.at(fileParts.count() - 1);
    
    ftype.first = fileExtName;
    if (gMimeHash.contains(fileExtName)) {
        ftype.second = path + gMimeHash.value(fileExtName) + ".png";
    } else {
        ftype.second = path + ftype.second;
    }

    // ("/usr/share/icons", "/usr/local/share/icons", "/usr/share/icons", ":/home/gzleo/.kde4/share/icons", ":/icons")
    // qDebug()<<QIcon::themeSearchPaths();
    
    return ftype;

    // raw seach
	if (QFile::exists(path+fileParts.at(fileParts.count()-1) + ".png" ) ) {
		ftype.first = fileParts.at(fileParts.count()-1);
		ftype.second = path+fileParts.at(fileParts.count()-1) + ".png";
	}
	if (fileName.toLower().endsWith(".wmv",Qt::CaseSensitive) 
		|| fileName.toLower().endsWith(".asf",Qt::CaseSensitive)
		|| fileName.toLower().endsWith(".rm",Qt::CaseSensitive) 
		|| fileName.toLower().endsWith(".rmvb",Qt::CaseSensitive) ) {
		ftype.first = "video";
		ftype.second = path + "video.png";		
	} else if (fileName.toLower().endsWith(".gz", Qt::CaseSensitive) 
		|| fileName.toLower().endsWith(".tgz", Qt::CaseSensitive)
		|| fileName.toLower().endsWith(".rar", Qt::CaseSensitive) ) {
		ftype.first = "tar";
		ftype.second = path + "tar.png";
	} else if (fileName.toLower().endsWith(".iso", Qt::CaseInsensitive)) {
        ftype.first = "iso";
        ftype.second = path + "cd-image.png";
    } else if (fileName.endsWith(".bz2", Qt::CaseInsensitive)) {
        ftype.first = "bz2";
        ftype.second = path + "bzip.png";
    } else if (fileName.endsWith(".torrent", Qt::CaseInsensitive)) {
        ftype.second = path + "bittorrent.png";
    }

    // qDebug()<<"FN:"<<fileName<<ftype;
	return ftype;
}

void NullGet::onEditSelectAll()
{
	qDebug()<<__FUNCTION__;
	QWidget * wg = focusWidget ();

	if (wg == this->mainUI.mui_tv_task_list) {
		this->mainUI.mui_tv_task_list->selectAll();
	} else if (wg == this->mainUI.mui_tv_seg_log_list ) {
		this->mainUI.mui_tv_seg_log_list->selectAll();
	}
	qDebug()<<wg;

}
void NullGet::onEditInvertSelect()
{
	qDebug()<<__FUNCTION__;
	QWidget * wg = focusWidget ();
	QModelIndex idx;
	QTreeView *tv;
	QItemSelectionModel *sm;
	if (wg == this->mainUI.mui_tv_task_list)
	{
		tv = this->mainUI.mui_tv_task_list;
	}
	else if (wg == this->mainUI.mui_tv_seg_log_list )
	{
		tv = this->mainUI.mui_tv_seg_log_list;
	}
	else
	{
		return;
	}
	sm = tv->selectionModel();
	QList<int> subrows;

	int rows = tv->model()->rowCount();
	int cols = tv->model()->columnCount();
	for(int i = 0;i < rows;  i ++ )
	{
		idx = tv->model()->index(i,0);
		if (sm->isRowSelected(i,QModelIndex()) )
		{
		}
		else
		{
			subrows.append(i);
		}
	}
	sm->clear();
	for(int i = 0; i < subrows.size();i ++)
	{
		sm->select(tv->model()->index(subrows[i],0),QItemSelectionModel::Select|QItemSelectionModel::Rows);
	}

	qDebug()<<wg;

}

void NullGet::onShowToolbarText(bool show) 
{
	if (show )
	{
		this->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	}
	else
	{
		this->setToolButtonStyle(Qt::ToolButtonIconOnly);
	}
}

void NullGet::onTaskListMenuPopup(/* const QPoint & pos */) 
{
	//qDebug()<<__FUNCTION__;

	//将菜单项进行使合适的变灰
	QModelIndex idx;
	QString assumeCategory = "Download";	
	if (this->mCatView->selectionModel()->selectedIndexes().size() == 0 )
	{
	}
	else if (this->mCatView->selectionModel()->selectedIndexes().size() > 0 )
	{
		idx = this->mCatView->selectionModel()->selectedIndexes().at(0);
		if (idx.data().toString().compare("Download")!=0
			&& idx.data().toString().compare("Downloaded")!=0
			&& idx.data().toString().compare("Deleted")!=0	)
		{
			
		}
		else
		{
			assumeCategory = idx.data().toString();
		}
	}
	if (assumeCategory.compare("Downloaded") == 0 )
	{
		this->mainUI.action_Start->setEnabled(false);
		this->mainUI.action_Pause->setEnabled(false);
		this->mainUI.action_Schedule->setEnabled(false);
		this->mainUI.action_Delete_task->setEnabled(false);
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0)
		{
			//this->mainUI.action_Start->setEnabled(true);
			//this->mainUI.action_Pause->setEnabled(true);
			//this->mainUI.action_Schedule->setEnabled(true);
			//this->mainUI.action_Delete_task->setEnabled(true);
			this->mainUI.action_Properties->setEnabled(true);
			this->mainUI.action_Comment->setEnabled(true);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI.action_Browse_Referer->setEnabled(true);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI.actionMove_Down->setEnabled(true);
			this->mainUI.actionMove_Up->setEnabled(true);
			this->mainUI.action_Move_bottom->setEnabled(true);
			this->mainUI.action_Move_top->setEnabled(true);
			this->mainUI.action_Site_Properties->setEnabled(true);
		}
		else
		{
			//this->mainUI.action_Start->setEnabled(false);
			//this->mainUI.action_Pause->setEnabled(false);
			//this->mainUI.action_Schedule->setEnabled(false);
			//this->mainUI.action_Delete_task->setEnabled(false);
			this->mainUI.action_Properties->setEnabled(false);
			this->mainUI.action_Comment->setEnabled(false);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI.action_Browse_Referer->setEnabled(false);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI.actionMove_Down->setEnabled(false);
			this->mainUI.actionMove_Up->setEnabled(false);
			this->mainUI.action_Move_bottom->setEnabled(false);
			this->mainUI.action_Move_top->setEnabled(false);
			this->mainUI.action_Site_Properties->setEnabled(false);
		}
	}
	else if (assumeCategory.compare("Deleted") == 0 )
	{
		this->mainUI.action_Start->setEnabled(false);
		this->mainUI.action_Pause->setEnabled(false);
		this->mainUI.action_Schedule->setEnabled(false);
		this->mainUI.action_Delete_task->setEnabled(false);
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0)
		{
			//this->mainUI.action_Start->setEnabled(true);
			//this->mainUI.action_Pause->setEnabled(true);
			//this->mainUI.action_Schedule->setEnabled(true);
			//this->mainUI.action_Delete_task->setEnabled(true);
			this->mainUI.action_Properties->setEnabled(true);
			this->mainUI.action_Comment->setEnabled(true);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI.action_Browse_Referer->setEnabled(true);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI.actionMove_Down->setEnabled(true);
			this->mainUI.actionMove_Up->setEnabled(true);
			this->mainUI.action_Move_bottom->setEnabled(true);
			this->mainUI.action_Move_top->setEnabled(true);
			this->mainUI.action_Site_Properties->setEnabled(true);
		}
		else
		{
			//this->mainUI.action_Start->setEnabled(false);
			//this->mainUI.action_Pause->setEnabled(false);
			//this->mainUI.action_Schedule->setEnabled(false);
			//this->mainUI.action_Delete_task->setEnabled(false);
			this->mainUI.action_Properties->setEnabled(false);
			this->mainUI.action_Comment->setEnabled(false);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI.action_Browse_Referer->setEnabled(false);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI.actionMove_Down->setEnabled(false);
			this->mainUI.actionMove_Up->setEnabled(false);
			this->mainUI.action_Move_bottom->setEnabled(false);
			this->mainUI.action_Move_top->setEnabled(false);
			this->mainUI.action_Site_Properties->setEnabled(false);
		}
	}
	else //if (assumeCategory.compare("Download") == 0 )
	{
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0)
		{
			this->mainUI.action_Start->setEnabled(true);
			this->mainUI.action_Pause->setEnabled(true);
			this->mainUI.action_Schedule->setEnabled(true);
			this->mainUI.action_Delete_task->setEnabled(true);
			this->mainUI.action_Properties->setEnabled(true);
			this->mainUI.action_Comment->setEnabled(true);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI.action_Browse_Referer->setEnabled(true);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI.actionMove_Down->setEnabled(true);
			this->mainUI.actionMove_Up->setEnabled(true);
			this->mainUI.action_Move_bottom->setEnabled(true);
			this->mainUI.action_Move_top->setEnabled(true);
			this->mainUI.action_Site_Properties->setEnabled(true);
		}
		else
		{
			this->mainUI.action_Start->setEnabled(false);
			this->mainUI.action_Pause->setEnabled(false);
			this->mainUI.action_Schedule->setEnabled(false);
			this->mainUI.action_Delete_task->setEnabled(false);
			this->mainUI.action_Properties->setEnabled(false);
			this->mainUI.action_Comment->setEnabled(false);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI.action_Browse_Referer->setEnabled(false);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI.actionMove_Down->setEnabled(false);
			this->mainUI.actionMove_Up->setEnabled(false);
			this->mainUI.action_Move_bottom->setEnabled(false);
			this->mainUI.action_Move_top->setEnabled(false);
			this->mainUI.action_Site_Properties->setEnabled(false);
		}
	}
	
	if (this->mTaskListView == sender() ) {
		this->mTaskPopupMenu->popup(QCursor::pos());
	} else {
		//不是任务视图控件发送来的。
	}
}
void NullGet::onUpdateJobMenuEnableProperty() 
{
	qDebug()<<__FUNCTION__;

	//将菜单项进行使合适的变灰
	QModelIndex idx;
	// QString assumeCategory = "Download";
    int catId = ng::cats::downloading;
	
	if (this->mCatView->selectionModel()->selectedIndexes().size() == 0) {

	} else if (this->mCatView->selectionModel()->selectedIndexes().size() > 0) {
		idx = this->mCatView->selectionModel()->selectedIndexes().at(ng::cats::cat_id);
        catId = idx.data().toInt();
		// if (idx.data().toString().compare("Download")!=0
		// 	&& idx.data().toString().compare("Downloaded")!=0
		// 	&& idx.data().toString().compare("Deleted")!=0	) {
			
		// } else {
		// 	assumeCategory = idx.data().toString();
		// }
	}

	if (catId == ng::cats::downloaded) { // (assumeCategory.compare("Downloaded") == 0 ) {
		this->mainUI.action_Start->setEnabled(false);
		this->mainUI.action_Pause->setEnabled(false);
		this->mainUI.action_Schedule->setEnabled(false);
		this->mainUI.action_Delete_task->setEnabled(false);
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0) {
			//this->mainUI.action_Start->setEnabled(true);
			//this->mainUI.action_Pause->setEnabled(true);
			//this->mainUI.action_Schedule->setEnabled(true);
			this->mainUI.action_Delete_task->setEnabled(true);
			this->mainUI.action_Properties->setEnabled(true);
			this->mainUI.action_Comment->setEnabled(true);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI.action_Browse_Referer->setEnabled(true);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI.actionMove_Down->setEnabled(true);
			this->mainUI.actionMove_Up->setEnabled(true);
			this->mainUI.action_Move_bottom->setEnabled(true);
			this->mainUI.action_Move_top->setEnabled(true);
			this->mainUI.action_Site_Properties->setEnabled(true);
		} else {
			//this->mainUI.action_Start->setEnabled(false);
			//this->mainUI.action_Pause->setEnabled(false);
			//this->mainUI.action_Schedule->setEnabled(false);
			//this->mainUI.action_Delete_task->setEnabled(false);
			this->mainUI.action_Properties->setEnabled(false);
			this->mainUI.action_Comment->setEnabled(false);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI.action_Browse_Referer->setEnabled(false);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI.actionMove_Down->setEnabled(false);
			this->mainUI.actionMove_Up->setEnabled(false);
			this->mainUI.action_Move_bottom->setEnabled(false);
			this->mainUI.action_Move_top->setEnabled(false);
			this->mainUI.action_Site_Properties->setEnabled(false);
		}
	} else if (catId == ng::cats::deleted) { // (assumeCategory.compare("Deleted") == 0 )
		this->mainUI.action_Start->setEnabled(false);
		this->mainUI.action_Pause->setEnabled(false);
		this->mainUI.action_Schedule->setEnabled(false);
		this->mainUI.action_Delete_task->setEnabled(false);
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0) {
			//this->mainUI.action_Start->setEnabled(true);
			//this->mainUI.action_Pause->setEnabled(true);
			//this->mainUI.action_Schedule->setEnabled(true);
			//this->mainUI.action_Delete_task->setEnabled(true);
			this->mainUI.action_Properties->setEnabled(true);
			this->mainUI.action_Comment->setEnabled(true);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI.action_Browse_Referer->setEnabled(true);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI.actionMove_Down->setEnabled(true);
			this->mainUI.actionMove_Up->setEnabled(true);
			this->mainUI.action_Move_bottom->setEnabled(true);
			this->mainUI.action_Move_top->setEnabled(true);
			this->mainUI.action_Site_Properties->setEnabled(true);
		} else {
			//this->mainUI.action_Start->setEnabled(false);
			//this->mainUI.action_Pause->setEnabled(false);
			//this->mainUI.action_Schedule->setEnabled(false);
			//this->mainUI.action_Delete_task->setEnabled(false);
			this->mainUI.action_Properties->setEnabled(false);
			this->mainUI.action_Comment->setEnabled(false);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI.action_Browse_Referer->setEnabled(false);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI.actionMove_Down->setEnabled(false);
			this->mainUI.actionMove_Up->setEnabled(false);
			this->mainUI.action_Move_bottom->setEnabled(false);
			this->mainUI.action_Move_top->setEnabled(false);
			this->mainUI.action_Site_Properties->setEnabled(false);
		}
	}
	else //if (assumeCategory.compare("Downloading") == 0 )
	{
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0) {
			this->mainUI.action_Start->setEnabled(true);
			this->mainUI.action_Pause->setEnabled(true);
			this->mainUI.action_Schedule->setEnabled(true);
			this->mainUI.action_Delete_task->setEnabled(true);
			this->mainUI.action_Properties->setEnabled(true);
			this->mainUI.action_Comment->setEnabled(true);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI.action_Browse_Referer->setEnabled(true);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI.actionMove_Down->setEnabled(true);
			this->mainUI.actionMove_Up->setEnabled(true);
			this->mainUI.action_Move_bottom->setEnabled(true);
			this->mainUI.action_Move_top->setEnabled(true);
			this->mainUI.action_Site_Properties->setEnabled(true);
		} else {
			this->mainUI.action_Start->setEnabled(false);
			this->mainUI.action_Pause->setEnabled(false);
			this->mainUI.action_Schedule->setEnabled(false);
			this->mainUI.action_Delete_task->setEnabled(false);
			this->mainUI.action_Properties->setEnabled(false);
			this->mainUI.action_Comment->setEnabled(false);
			this->mainUI.action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI.action_Browse_Referer->setEnabled(false);
			this->mainUI.action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI.actionMove_Down->setEnabled(false);
			this->mainUI.actionMove_Up->setEnabled(false);
			this->mainUI.action_Move_bottom->setEnabled(false);
			this->mainUI.action_Move_top->setEnabled(false);
			this->mainUI.action_Site_Properties->setEnabled(false);
		}
	}
	
}

void NullGet::onLogListMenuPopup(const QPoint & pos ) 
{
	this->mLogPopupMenu->popup(QCursor::pos());
}
void NullGet::onSegListMenuPopup(const QPoint & pos) 
{
	// this->mSegmentPopupMenu->popup(QCursor::pos());
}
void NullGet::onCateMenuPopup(const QPoint & pos)
{
	this->mCatPopupMenu->popup(QCursor::pos());
}

void NullGet::onCopyUrlToClipboard()
{
	qDebug()<<__FUNCTION__;

	QItemSelectionModel * ism = 0;
	QModelIndexList mil;

	ism = this->mTaskListView->selectionModel();
	if (ism != 0) {
		mil = ism->selectedIndexes();
		QString url = mil.at(ng::tasks::real_url).data().toString();
		QClipboard * cb = QApplication::clipboard();
		cb->setText(url);

	}
}

////////////
void NullGet::onCopySelectSegLog()
{
	qDebug()<<__FUNCTION__;
	
	QModelIndex idx;
	QTreeView *tv;
	QItemSelectionModel *sm;	
	QString text;
	QModelIndexList mil;
	int cols = 0;

	if (this->mSegLogListView->model() != 0 )
	{
		cols = this->mSegLogListView->model()->columnCount();
		sm = this->mSegLogListView->selectionModel();
		mil = sm->selectedIndexes();
		for(int r = 0; r < mil.size()/cols; r++ )
		{
			for(int c = 0; c < cols; ++c )
			{
				text +=   mil.value(r+c*mil.size()/cols).data().toString();
				text += '\t';
			}
			text += "\r\n";
		}
		if (text.endsWith("\r\n"))
			text = text.left(text.length()-2);
		//
		QClipboard * cb = QApplication::clipboard();
		cb->setText(text);		
	}


	qDebug()<<cols<<text <<mil.size();

}
void NullGet::onSaveSegLog()
{
	QModelIndex idx;
	QTreeView *tv;
	QItemSelectionModel *sm;	
	QString text;
	QModelIndexList mil;
	QAbstractItemModel * model = 0;
	int cols = 0;

	model = this->mSegLogListView->model();
	if (model != 0 )
	{

		cols = model->columnCount();
		
		for(int r = 0; r < model->rowCount(); ++r)
		{
			for(int c = 0; c < cols; ++ c)
			{
				text += model->data(model->index(r,c)).toString();
				text += '\t';
			}
			text += "\r\n";
		}
		
		if (text.endsWith("\r\n"))
			text = text.left(text.length()-2);
		//
		QString fname = QFileDialog::getSaveFileName(this);
		if (! fname.isNull() )
		{
			QFile fi(fname);
			fi.open(QIODevice::WriteOnly|QIODevice::Unbuffered);			
			fi.write(text.toAscii());
			//fi.waitForBytesWritten();
			fi.close();
		}
	}
}
/**
 * 	清除线程日志视图中的LOG信息。
 */
void NullGet::onClearSegLog() 	//
{
	QAbstractItemModel * aim = 0;
	int row , col;

	aim = this->mSegLogListView->model();
	if (aim == 0 ) return;	//没有日志，直接返回
	row = aim->rowCount();
	for(int i = row -1; i >=0; i --)
	{
		aim->removeRows(i,1);
	}
	// replace or short stmt
	row = aim->rowCount();
	if (row > 0 )
		aim->removeRows(0,row);
}


/////language	
/**
 * Qt 的这种语言处理方式不能在程序运行时动态修改，需要另一种比较麻烦的方法实现，详见Qt 手册。
 * 已经修改为支持动态语言切换了
 */
void NullGet::onSwitchLanguage(QAction* action)
{
	QString lang = "en_US";	//默认
	//if (action == this->mainUI.action_Chinese_simple)
	//{
	//	qDebug()<<"switch lang: zh_CN";
	//	lang = "zh_CN";
	//}

	//if (action == this->mainUI.action_English)
	//{
	//	qDebug()<<"switch lang: en_US";
	//	lang = "en_US";
	//}
	lang = action->data().toString();
	if (lang.isEmpty() )
		 lang = "en_US";	//默认
	
	//QTranslator translator;
	//translator.load(QString("nullget_")+lang);
	//qApp->installTranslator(&translator);

	QString langFile  = QString("nullget_")+lang;
	appTranslator.load(langFile , qmPath );
	//qDebug()<<"Loading file :"<<langFile;
	langFile = "qt_"+lang;
	qtTranslator.load(langFile,qmPath);
	//qDebug()<<"Loading file :"<<langFile;

	if (! action->isChecked()) {
		action->setChecked(true);
	}

	this->retranslateUi();
	
}

void NullGet::onSwitchSkinType(QAction* action)
{

}

////////////style
//"windows", "motif", "cde", "plastique", "windowsxp", or "macintosh"
// ("Bespin", "Oxygen", "Windows", "Motif", "CDE", "Plastique", "GTK+", "Cleanlooks")

void NullGet::onSwitchWindowStyle(QAction * action )
{
	qDebug()<<__FUNCTION__<<":"<<__LINE__<<" typeL "<< action->data().toString()
		<< this->sender();
    // QStringList styleKeys = QStyleFactory::keys();
    // qDebug()<<styleKeys;

	if (action->data().toString() == "norwegianwood") {
		//qDebug()<<"NorwegianWood style";
		QStyle * nw = new NorwegianWoodStyle();
		QApplication::setStyle(nw);
	} else {
		QApplication::setStyle(action->data().toString());
	}

	if (mainUI.action_Default_Palette->isChecked()) {
		QApplication::setPalette(this->orginalPalette);
	} else {
		QApplication::setPalette(QApplication::style()->standardPalette());
	}
	if (! action->isChecked()) {
		action->setChecked(true);
	}
}

void NullGet::onSwitchSpeedMode(QAction *action)
{
	qDebug()<<__FUNCTION__;
	if (action == mainUI.action_Unlimited) {
		this->mSpeedBarSlider->hide();
		this->mSpeedProgressBar->hide();
		this->mSpeedManualLabel->hide();
		GlobalOption::instance()->mIsLimitSpeed = 0;
        this->onManualSpeedChanged(0);  // set no limit
	} else if (action == mainUI.action_Manual) {
		this->mSpeedBarSlider->show();
		this->mSpeedProgressBar->hide();
		this->mSpeedManualLabel->show();
		GlobalOption::instance()->mIsLimitSpeed = 1;
	} else if (action == mainUI.action_Automatic ) {
		this->mSpeedBarSlider->hide();
		this->mSpeedProgressBar->show();
		this->mSpeedManualLabel->show();
		GlobalOption::instance()->mIsLimitSpeed = 0;
        this->onManualSpeedChanged(0); // no limit now 
	}
}

// TODO call many time when drap the speed slider bar
void NullGet::onManualSpeedChanged(int value) 
{
	//qDebug()<<__FUNCTION__;
	this->mSpeedManualLabel->setText(QString("%1 KB/s").arg(value));
	//this->mSpeedTotalLable->setText(QString("%1 KB/s").arg(value*this->mTaskQueue.size()));
	GlobalOption::instance()->mMaxLimitSpeed = value * 1024;	//the value is KB in unit

    QVariant payload;
    QVariantList args;
    QVariantMap options;
    options["max-overall-download-limit"] = QString("%1K").arg(value); // .arg(value * 1024);
    payload = QString("max-overall-download-limit");

    args.insert(0, options);

    this->mAriaRpc->call("aria2.changeGlobalOption", args, payload,
                         this, SLOT(onAriaChangeGlobalOptionResponse(QVariant &, QVariant &)),
                         this, SLOT(onAriaChangeGlobalOptionFault(int, QString, QVariant &)));
        
}
////////////

void NullGet::showAboutDialog()
{
	this->mpAboutDialog = new AboutDialog(this);

	this->mpAboutDialog->exec();

	delete this->mpAboutDialog;

}
void NullGet::onGotoHomePage()
{
	QString homepage = "http://www.qtchina.net/nullget.html";
	QDesktopServices::openUrl(homepage);
}

/**
 * 隐藏或者显示主窗口。
 * 这是一个翻转开关，是显示还是隐藏依赖于主窗口当前是否显示。
 */
void NullGet::onDropZoneDoubleClicked()
{
	qDebug()<<__FUNCTION__;
	if (this->isHidden () )
	{
		this->setHidden(false);
		this->showNormal();
		this->setFocus();
		this->raise();
		this->activateWindow();
	}
	else
	{
		this->showMinimized();
		this->setHidden(true);
	}
}
void NullGet::onDropZoneCustomMenu(const QPoint & pos)
{
	this->mDropZonePopupMenu->popup(QCursor::pos());
}


void NullGet::onOpenDistDirector()
{
	qDebug()<<__FUNCTION__;
	QString openner;
	QString tmp;
	QString dir;
	int taskId = -1;
	int catId = -1; 
	
	QItemSelectionModel * ism = 0;
	QModelIndexList mil;
	QString durl;

	ism = this->mTaskListView->selectionModel();
	mil = ism->selectedIndexes();
	if (mil.size() > 0) {
		taskId = mil.at(ng::tasks::task_id).data().toString().toInt();	
        // taskId = this->mTaskListViewModel->data(mil.at(0)).toInt();
		catId = mil.at(ng::tasks::user_cat_id).data().toString().toInt();

		// dir = SqliteStorage::instance(this)->getSavePathByCatId(catId);
        dir = mil.at(ng::tasks::save_path).data().toString();
        if (dir.startsWith("~")) {
            dir = QDir::homePath() + dir.right(dir.length() - 1);
        }
		durl = dir;

		bool opened = QDesktopServices::openUrl(durl);	//only qt >= 4.2
        if (!opened) {
            opened = QDesktopServices::openUrl(QUrl("file://" + durl));
        }
	} else {
		QMessageBox::warning(this, tr("No Task Selected"), tr("Please Select a Task For Operation"));
	}
}

void NullGet::onOpenExecDownloadedFile()
{
	qDebug()<<__FUNCTION__;
	
	QString tmp;
	QString dir, fname;
	int taskId = -1;
	int catId = -1; 

	QItemSelectionModel * ism = 0;
	QModelIndexList mil;
	QString durl;

	ism = this->mTaskListView->selectionModel();
	mil = ism->selectedIndexes();
	if (mil.size() > 0) {
		taskId = mil.at(ng::tasks::task_id).data().toString().toInt();	
        //this->mTaskListViewModel->data(mil.at(0)).toInt();	
		catId = mil.at(ng::tasks::user_cat_id).data().toString().toInt();
		fname = mil.at(ng::tasks::file_name).data().toString();

		// dir = SqliteStorage::instance(this)->getSavePathByCatId(catId);
        dir = mil.at(ng::tasks::save_path).data().toString();

        if (dir.startsWith("~")) {
            dir = QDir::homePath() + dir.right(dir.length() - 1);
        }
		durl = dir + QString("/") + fname;
        QString fullUrl = QDir().absoluteFilePath(durl);
		if (fname .length() == 0 || ! QDir().exists(fullUrl)) {
			QMessageBox::warning(this, tr("Notice:"),
                                 QString(tr("File <b>%1</b> not found, has it downloaded already?")).arg(fullUrl));
		} else {
			//QProcess::execute(openner);
            bool opened = QDesktopServices::openUrl(fullUrl);	//only qt >= 4.2
            if (!opened) {
                opened = QDesktopServices::openUrl(QUrl("file://" + fullUrl));
            }
		}
	} else {
		QMessageBox::warning(this, tr("No Task Selected"), tr("Please Select a Task For Operation"));
	}
}


void NullGet::onOpenRefererUrl()
{
	QString tmp;
	QString dir, fname, referer;
	int taskId = -1;
	int catId = -1; 

	QItemSelectionModel * ism = 0;
	QModelIndexList mil;
	QString durl;

	ism = this->mTaskListView->selectionModel();
	mil = ism->selectedIndexes();
	if (mil.size() > 0) {
		taskId = mil.at(ng::tasks::task_id).data().toString().toInt();	
		catId = mil.at(ng::tasks::user_cat_id).data().toString().toInt();
		fname = mil.at(ng::tasks::file_name).data().toString();
        referer = mil.at(ng::tasks::referer).data().toString();

        QDesktopServices::openUrl(QUrl(referer));
		// dir = SqliteStorage::instance(this)->getSavePathByCatId(catId);
        // dir = mil.at(ng::tasks::save_path).data().toString();

        // if (dir.startsWith("~")) {
        //     dir = QDir::homePath() + dir.right(dir.length() - 1);
        // }
		// durl = dir + QString("/") + fname;
        // QString fullUrl = QDir().absoluteFilePath(durl);
		// if (fname .length() == 0 || ! QDir().exists(fullUrl)) {
		// 	QMessageBox::warning(this, tr("Notice:"),
        //                          QString(tr("File <b>%1</b> not found, has it downloaded already?")).arg(fullUrl));
		// } else {
		// 	//QProcess::execute(openner);
        //     bool opened = QDesktopServices::openUrl(fullUrl);	//only qt >= 4.2
        //     if (!opened) {
        //         opened = QDesktopServices::openUrl(QUrl("file://" + fullUrl));
        //     }
		// }
	} else {
		QMessageBox::warning(this, tr("No Task Selected"), tr("Please Select a Task For Operation"));
	}
    
}

//////////private

//
void NullGet::onClipBoardDataChanged()
{
	
	QClipboard *cb = QApplication::clipboard();	
	QString text = cb->text();

	qDebug()<<cb->mimeData()->formats();
	
	if (text.length() > 0) {
		QUrl uu(text);
		if (uu.scheme().toUpper().compare("HTTP") == 0 
            || uu.scheme().toUpper().compare("HTTPS") == 0
            || uu.scheme().toUpper().compare("FTP") == 0
            || text.startsWith("magnet:?", Qt::CaseInsensitive)
            || text.startsWith("flashget", Qt::CaseInsensitive)
            || text.startsWith("qqdl:", Qt::CaseInsensitive)
            || text.startsWith("thunder:", Qt::CaseInsensitive)
            || text.startsWith("nullget://", Qt::CaseInsensitive)) {
			// this->showNewDownloadDialog(); // Open sometime later
		}
        if (text.startsWith("nullget://", Qt::CaseInsensitive)) {
            this->showNewDownloadDialog(); // this should be nullget passed from browser, force handle it
        }
	}
	
	qDebug()<<text;
}
void NullGet::caclAllTaskAverageSpeed() 
{
	// double sumSpeed = 0.0;
	// if (sumSpeed >= 0.0)
	// {
	// 	this->mSpeedTotalLable->setText(QString("%1 KB/s").arg(sumSpeed));
	// 	this->mSpeedProgressBar->setValue((int)sumSpeed);
	// }
	// this->mISHW->updateSpeedHistogram(sumSpeed);

	// this->statusBar()->showMessage(rc);
}

void NullGet::onAllocateDiskFileSpace(quint64 fileLength , QString fileName )
{
	//做磁盘预分配
	QProgressDialog pd (QString(tr("Allocating File Space: %1")).arg(fileName),"Cancel",0,100, this);
	pd.setWindowModality(Qt::WindowModal);
	pd.setMinimumDuration(0);
	pd.setFixedSize(pd.size().width()*1.82,pd.size().height());
	pd.setWindowFlags(Qt::Tool| Qt::WindowTitleHint);
	int px = (this->pos().x()+this->size().width()/2-pd.width()/2);
	int py = (this->pos().y()+this->size().height()/2-pd.height()/2);
	pd.move(px , py );
	pd.setCancelButton(0);
	pd.setWindowTitle(tr("Waiting ..."));
	

	int step = 100;
	int ps = 0;
	QFile tf(fileName);
	tf.open(QIODevice::ReadWrite|QIODevice::Unbuffered);
	while(step > 0 )
	{
		int sl = fileLength/100;
		if (ps + sl >= fileLength )
		{
			ps = fileLength;
		}
		else
		{
			ps += sl;
		}
		tf.seek(ps);
		tf.write("\0",1);
		pd.setValue(100-step);
		qApp->processEvents();
		step --;
	}
	tf.close();
	//QFile::remove("haha.wmv");
}

void NullGet::paintEvent (QPaintEvent * event )
{
    Q_UNUSED(event);
	QPainter painter(this);
	QPoint p(0, 0);
	if (this->image.isNull()) {
		this->image =QImage("4422b46f9b7e5389963077_zVzR8Bc2kwIf.jpg");
		//qDebug()<<this->mCatView->windowOpacity();
		//this->mCatView->setWindowOpacity(0.5);
		//qDebug()<<this->mCatView->windowOpacity();
		this->mainUI.splitter_4->setWindowOpacity(0.3);
		this->mainUI.splitter_3->setWindowOpacity(0.5);
		this->mainUI.splitter_2->setWindowOpacity(0.8);
		this->mainUI.splitter->setWindowOpacity(0.6);
		//this->setWindowOpacity(0.7);
		// QPixmap pixmap("4422b46f9b7e5389963077_zVzR8Bc2kwIf.jpg");
		 //this->mCatView->setPixmap(pixmap);
	}
	//painter.drawImage(p, this->image );	
	//this->setWindowOpacity(0.6);	//ok
}
/**
 *
 * 提示用户是否真的退出。可能是用户的误操作
 * 清理数据，保存状态信息。
 */
void NullGet::closeEvent (QCloseEvent * event )
{
	qDebug()<<__FUNCTION__ << this->sender();
	//qDebug()<< static_cast<QApplication*>(QApplication::instance())->quitOnLastWindowClosed ();
	if (this->sender() == 0) { // 点击右上角的X号,将该行为转成窗口最小化，隐藏到系统托盘区域
		this->mainUI.action_Show_Hide_Main_Widow->trigger();
		event->setAccepted(false);
	} else {//通过点击退出菜单，可认为用户是想退出的。
		//if (QMessageBox::question(this,"Are you sure?","Exit NullGet Now.",QMessageBox::Ok,QMessageBox::Cancel) == QMessageBox::Cancel )
		{
		//	event->setAccepted(false);	//不再传递了
		//	return;
		}
		if (this->mWalkSiteWnd != 0) { //the other main window , close it here
			this->mWalkSiteWnd->close();
			delete this->mWalkSiteWnd;
			this->mWalkSiteWnd = 0;
		}
		if (this->mHWalkSiteWndEx != 0) { //the other main window , close it here
			this->mHWalkSiteWndEx->close();
			delete this->mHWalkSiteWndEx;
			this->mHWalkSiteWndEx = 0;
		}

		//清理托盘图标
		delete this->mSysTrayIcon;this->mSysTrayIcon = 0;

		QApplication::setQuitOnLastWindowClosed(true);
		event->setAccepted(true );
		QMainWindow::closeEvent(event);
	}
}

void NullGet::showEvent (QShowEvent * event ) 
{
	QWidget::showEvent(event);

	//qDebug()<<__FUNCTION__<<__LINE__<<rand();
	if (firstShowEvent == true) {	
		firstShowEvent = false;
		//添加首次显示要处理的工作
		qDebug()<<__FUNCTION__<<__LINE__<<"first show";
		//this->firstShowHandler();
		QTimer::singleShot(30, this, SLOT(firstShowHandler()));
	}
}

#ifdef Q_OS_WIN
bool NullGet::winEvent (MSG * msg, long * result )
{
	//qDebug()<<__FUNCTION__<<__LINE__<<rand();
	//whereis MOD_CONTROL ???
    // shutcut: CTRL+SHIFT+C
	if (msg->message == WM_HOTKEY) {
		switch(msg->wParam) {
		case 'C':
			//做想做的事。我想用它抓图,哈哈，完全可以啊。
			this->shootScreen();
			qDebug()<<msg->message <<"xxx"<<(int)('G')<<":"<<msg->wParam<<" lp:"<< msg->lParam;
			break;
		default:
			break;
		}
	}
	
	return QMainWindow::winEvent(msg, result);
}
#else
#ifdef Q_OS_MAC
bool NullGet::macEvent (EventHandlerCallRef caller, EventRef event )
{
    return QMainWindow::macEvent(caller, event);
}
#else
bool NullGet::x11Event (XEvent * event )
{
    //qDebug()<<"XEvent->type:"<< event->type;
    if (event->type == PropertyNotify) {
        //qDebug()<<"dont push button";
    }
    return QMainWindow::x11Event(event);
}
void NullGet::keyReleaseEvent (QKeyEvent * event )
{
    // shortcut: CTRL+G
    if (event->modifiers() == Qt::ControlModifier) {
        //qDebug()<<"with ctrl key ";
        if (event->key() == Qt::Key_G) {
            //qDebug()<<"key released: " << event->text();
            this->shootScreen();
        }
    }
    
    return QMainWindow::keyReleaseEvent(event);
}
#endif
#endif

void NullGet::shootScreen() 
{
	QPixmap screenSnapShot = QPixmap::grabWindow(QApplication::desktop()->winId());
	QString format = "png";

	QString fileName =	QString("abcd" ) + "."+format;
	
	if (! QFile::exists(fileName)) {
		QFile ff(fileName);
		ff.open(QIODevice::ReadWrite|QIODevice::Unbuffered);
		ff.close();
	}
	if (! screenSnapShot.save(fileName, format.toAscii())) {
		qDebug()<< "save screen snap shot error:"<<fileName;
	}
}

//system tray slot handler
void NullGet::onActiveTrayIcon(QSystemTrayIcon::ActivationReason index )
{
	qDebug()<<__FUNCTION__ << index;
	if (index == QSystemTrayIcon::DoubleClick	)
	{		
		this->mainUI.action_Show_Hide_Main_Widow->trigger();
		this->raise();
	}
	else if (index == QSystemTrayIcon::Context )
	{

	}
	else if (index == QSystemTrayIcon::Trigger )
	{

	}
	else if (index == QSystemTrayIcon::Unknown )
	{
	}
}
/**
 * 系统托盘弹出的泡泡状消息窗口被点击时处理函数
 */
void NullGet::onBallonClicked()
{
	qDebug()<<__FUNCTION__;
}

void NullGet::onShowWalkSiteWindow()
{
	//将搜索窗口集成到主窗口中，不再需要显示这个窗口了。将下面到hide几行删除，release编译出来的程序会小60多K。	
	//if (this->mWalkSiteWnd == 0 )
	//{
	//	this->mWalkSiteWnd = new WalkSiteWnd(this);
	//}
	//this->mWalkSiteWnd->showMinimized();
	//this->mWalkSiteWnd->hide();

	if (this->mHWalkSiteWndEx == 0 )
	{
		this->mHWalkSiteWndEx = new WalkSiteWndEx(this);		
		//本模块接收音乐下载的信号
	}
	this->mHWalkSiteWndEx->showMinimized();
	this->mHWalkSiteWndEx->hide();

	//////////停靠窗口重叠属性为可以重叠
	if (! this->isDockNestingEnabled() )
	{
		this->setDockNestingEnabled(true);
	}
	//先将窗口最大化，如果不是的话。
	if (! this->isMaximized() )
	{
		//this->showMaximized();
	}
	///next 集成到主窗口的依靠窗口的搜索窗口初始化/显示。
	if (this->mWalkSiteDockWidget == 0 )
	{
		//this->mWalkSiteDockWidget = new QDockWidget(tr(" Site Crawler "), this);	
		this->mWalkSiteDockWidget = new QDockWidget(this->mHWalkSiteWndEx->windowTitle(), this);	
		this->mWalkSiteDockWidget->setAllowedAreas(Qt::BottomDockWidgetArea);
		this->mWalkSiteDockWidget->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
		this->mWalkSiteDockWidget->setFloating(false);
		this->addDockWidget(Qt::BottomDockWidgetArea, this->mWalkSiteDockWidget,Qt::Horizontal);
		this->mWalkSiteDockWidget->showMinimized();
		this->mWalkSiteDockWidget->showNormal();
		//如果检测到有另一个窗口存在，可以浮动起来该窗口，再检测一个合适的位置停靠，估计能实现那种带TAB的浮动窗口效果。
	}

	if (this->mWalkSiteDockWidget->widget() != this->mHWalkSiteWndEx )
	{		
		this->mWalkSiteDockWidget->setWidget(this->mHWalkSiteWndEx);
	}
	this->mHWalkSiteWndEx->showMinimized();
	qDebug()<< this->mWalkSiteDockWidget->isHidden();
	if (this->mWalkSiteDockWidget->isHidden() )
		this->mWalkSiteDockWidget->showMinimized();

}

void NullGet::onAriaProcError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        this->mAriaGlobalUpdater.stop();
        QMessageBox::warning(this, tr("Aria2 backend error :"), 
                             tr("Can not start aria2. Are you already installed it properly?"));
    }
}

void NullGet::onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
}


void NullGet::handleArguments()
{
    int argc = qApp->argc();
    char **argv = qApp->argv();

    this->handleArguments(argc, argv);
}

#include "getopt_pp_standalone.h"

void NullGet::handleArguments(int argc, char **argv)
{
    for (int i = 0 ; i < argc ; i ++) {
        qDebug()<<"Arg no: "<<i<<argv[i];
    }

    int rargc = argc;
    char **rargv = argv;
    char *targv[100] = {0};
    /* opera for win send this format arguments:
       no:  0 Z:\cross\karia2-svn\release\NullGet.exe
       no:  1 --uri http://down.sandai.net/Thunder5.9.19.1390.exe --refer http://dl.xunlei.com/index.htm?tag=1
       NullGet::handleArguments No uri specified.
     */
    // maybe opera
#if defined(Q_OS_WIN)
    if (argc == 2) {
        rargc = 0;
        rargv = targv;

        qDebug()<<"Reformat arguments for handle properly. "; // ktyytc11
        reformatArgs = 1;
        char *farg = argv[1];
        char *ptr = 0;

        rargc += 1;
        targv[0] = argv[0];

        while (*farg == ' ') { farg++; } ; // ltrim
        while (true && farg != NULL) {
            ptr = strchr(farg, ' ');
            qDebug()<<"here:"<<ptr;
            if (ptr == NULL) {
                targv[rargc++] = farg;
                break;
            } else {
                targv[rargc++] = farg;
                farg = ptr + 1;
                *ptr = '\0';
            }
        }

        for (int i = 0 ; i < rargc ; i ++) {
            qDebug()<<"rArg no: "<<i<<rargv[i];
        }
    }
#endif

    std::string std_uri, std_refer, std_metafile, std_cookies, std_agent;

    // GetOpt::GetOpt_pp args(argc, argv);
    GetOpt::GetOpt_pp args(rargc, rargv);
    args >> GetOpt::Option('u', "uri", std_uri);
    args >> GetOpt::Option('r', "refer", std_refer);
    args >> GetOpt::Option('c', "cookie", std_cookies);
    args >> GetOpt::Option('m', "metafile", std_metafile);
    args >> GetOpt::Option('a', "agent", std_agent);

    QString uri, refer, metafile, cookies, agent;
    uri = QString::fromStdString(std_uri);
    refer = QString::fromStdString(std_refer);
    cookies = QString::fromStdString(std_cookies);
    metafile = QString::fromStdString(std_metafile);
    agent = QString::fromStdString(std_agent);

    if (metafile.length() > 0) {
        QFile file(metafile);
        file.open(QIODevice::ReadOnly);
        QByteArray metaData = file.readAll().trimmed();
        QList<QByteArray> metaInfo = metaData.split('\n');
        
        uri = metaInfo.at(1);
        refer = metaInfo.at(2);
        cookies = metaInfo.at(3);
        agent = metaInfo.at(4);
    }

    if (uri.length() == 0) {
        qDebug()<<__FUNCTION__<<"No uri specified.";
        return;
    }

    // fix for opera %l or %u give the error url
    if (uri.startsWith("http:/", Qt::CaseInsensitive)
        && !uri.startsWith("http://", Qt::CaseInsensitive)) {
        uri = uri.replace("http:/", "http://");
    } else if (uri.startsWith("ftp:/", Qt::CaseInsensitive)
               && !uri.startsWith("ftp://", Qt::CaseInsensitive)) {
        uri = uri.replace("ftp:/", "ftp://");
    }

    // fix for opera %l or %u give the error url
    if (refer.startsWith("http:/", Qt::CaseInsensitive)
        && !refer.startsWith("http://", Qt::CaseInsensitive)) {
        refer = refer.replace("http:/", "http://");
    } else if (refer.startsWith("ftp:/", Qt::CaseInsensitive)
               && !refer.startsWith("ftp://", Qt::CaseInsensitive)) {
        refer = refer.replace("ftp:/", "ftp://");
    }

    // convect to TaskOption raw data formats, for passive more data
    TaskOption options;
    options.mTaskUrl = uri;
    options.mReferer = refer;
    options.mCookies = cookies;

    QString ngetUri = "nullget://" + options.toBase64Data();
    QClipboard *cb = QApplication::clipboard();
    cb->setText(ngetUri);
    qDebug()<<__FUNCTION__<<"uri:"<<uri<<"cbtext:"<<cb->text()<<ngetUri;

    // this->mainUI.action_New_Download->trigger();
    qApp->setActiveWindow(this);
    this->setFocus(Qt::MouseFocusReason);

}

void NullGet::handleArguments(QStringList args)
{
    QString uri, uri2;
    QString refer, refer2;
    QString shortArgName;
    QString longArgName;
    QString defaultArgValue = QString::null;
    
    // args.takeFirst(); // it is app name

    int argc = args.count();
    char *argv[100] = {0};
    
    for (int i = 0; i < argc; i ++) {
        argv[i] = strdup(args.at(i).toAscii().data());
    }
    this->handleArguments(argc, argv);

    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    // 
    return;

    // GetOpt4  go(args);
    
    // shortArgName = "u";
    // longArgName = "uri";
    // go.addOptionalOption('u', longArgName, &uri, defaultArgValue); // deal with case: --uri=abcd.zip -uabcd.zip
    // // go.addOptionalOption(longArgName, &uri, defaultArgValue); // maybe case: --uri abcd.zip

    // QString referShortArgName = "r";
    // QString referLongArgName = "refer";
    // go.addOptionalOption('r', referLongArgName, &refer, defaultArgValue);
    // // go.addOptionalOption(referLongArgName, &refer, defaultArgValue);

    // if (!go.parse()) {
    //     // qDebug()<<__FUNCTION__<<"Arg parse faild.";
    //     return;
    // }

    // qDebug()<<uri<<uri2;
    // qDebug()<<refer<<refer2;

    // if (uri == QString::null && uri2 == QString::null) {
    //     qDebug()<<__FUNCTION__<<"No uri specified.";
    //     return;
    // }

    // uri = (uri == QString::null) ? uri2 : uri;

    // // fix for opera %l or %u give the error url
    // if (uri.startsWith("http:/", Qt::CaseInsensitive)
    //     && !uri.startsWith("http://", Qt::CaseInsensitive)) {
    //     uri = uri.replace("http:/", "http://");
    // } else if (uri.startsWith("ftp:/", Qt::CaseInsensitive)
    //            && !uri.startsWith("ftp://", Qt::CaseInsensitive)) {
    //     uri = uri.replace("ftp:/", "ftp://");
    // }

    // refer = (refer == QString::null) ? refer2 : refer;
    // // fix for opera %l or %u give the error url
    // if (refer.startsWith("http:/", Qt::CaseInsensitive)
    //     && !refer.startsWith("http://", Qt::CaseInsensitive)) {
    //     refer = refer.replace("http:/", "http://");
    // } else if (refer.startsWith("ftp:/", Qt::CaseInsensitive)
    //            && !refer.startsWith("ftp://", Qt::CaseInsensitive)) {
    //     refer = refer.replace("ftp:/", "ftp://");
    // }

    // // convect to TaskOption raw data formats, for passive more data
    // TaskOption options;
    // options.mTaskUrl = uri;
    // options.mReferer = refer;

    // QString ngetUri = "nget://" + options.toBase64Data();
    // QClipboard *cb = QApplication::clipboard();
    // cb->setText(ngetUri);
    // qDebug()<<__FUNCTION__<<"uri:"<<uri<<"cbtext:"<<cb->text()<<ngetUri;

    // this->mainUI.action_New_Download->trigger();
    // qApp->setActiveWindow(this);
    // this->setFocus(Qt::MouseFocusReason);
}

void NullGet::onOtherKaria2MessageRecived(const QString &msg)
{
    QStringList args;
    
    if (msg.startsWith("sayhello:")) {
        qDebug()<<__FUNCTION__<<"You says: "<<msg;
    } else if (msg.startsWith("cmdline:")) {
        args = msg.right(msg.length() - 8).split(" ");
        this->handleArguments(args);
    } else {
        qDebug()<<__FUNCTION__<<"Unknown message type: "<<msg;
    }
}


//dynamic language switch
void NullGet::retranslateUi()
{
	this->mainUI.retranslateUi(this);	//不调用它还不行，不知道为什么呢。
	//还有一个问题，怎么把程序中所有的字符串都放在这个函数中呢。
}

void NullGet::onObjectDestroyed(QObject * obj )
{
	qDebug()<<__FUNCTION__<<__LINE__<<" "<< obj->objectName();
	obj->dumpObjectInfo ();
	obj->dumpObjectTree ();
}


//QAXFACTORY_DEFAULT(NullGet,
//	   "{074AA25F-F544-401E-8A2A-5C81F01264EF}",
//	   "{4351FA96-A922-4D76-B4AD-A0A4CF0ED8AA}",
//	   "{DBEF3F59-305C-4A58-9491-F7E56ADBB0B0}",
//	   "{9D6E015B-02EF-4FF6-A862-4B26250FCF57}",
//	   "{E0D9ECBF-2E40-4E94-A37B-0E4FB1ADBBB9}")


//////////end of nullget.cpp


