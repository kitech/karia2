// karia2.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 22:27:02 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "ui_karia2.h"

#include "utility.h"
#include "karia2.h"
#include "asynctask.h"
#include "taskui.h"
#include "optionui.h"
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

extern QHash<QString, QString> gMimeHash;

////////////////////////////////////////////////
//main window 
////////////////////////////////////////////////
Karia2::Karia2(int argc, char **argv, QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , mainUI(new Ui::Karia2())
    , mAtask(NULL)
    , mTaskMan(NULL)
    // , mAriaMan(NULL), mAriaRpc(NULL)
    , mEAria2Man(NULL)
    , m_argc(argc), m_argv(argv)
//    , mSkypeTracer(NULL)
{
    //    QDir().setCurrent(qApp->applicationDirPath()); // why do this?
	mainUI->setupUi(this);	
	firstShowEvent = true;
    this->mNorStyle = NULL;

	/////////////
	orginalPalette = QApplication::palette();
	//dynamic language switch
	qmLocale = QLocale::system().name();
	qmPath = qApp->applicationDirPath() + QString("/translations");
	qLogx()<<"Switch Langague to: "<<qmLocale;
	qApp->installTranslator(&appTranslator);
	qApp->installTranslator(&qtTranslator);
	appTranslator.load(QString("karia2_") + qmLocale , qmPath );
	qtTranslator.load(QString("qt_") + qmLocale , qmPath );
	this->retranslateUi();
	if (qmLocale.startsWith("zh_CN")) {
		this->mainUI->action_Chinese_simple->setChecked(true);
	} else if (qmLocale.startsWith("zh_TW")) {
		this->mainUI->action_Chinese_trad->setChecked(true);
	} else {
		this->mainUI->action_English->setChecked(true);
	}
	// this->mDropZone = new DropZone();	// little float window
    qLogx()<<"";

    // ***ui init
    this->taskUi = new TaskUi(this);
    this->taskUi->init();

}
/**
 * first show adjust window layout
 * TODO 改进，主线程加载界面组件，异步任务线程加载数据组件，初始化非gui对象。
 */
void Karia2::firstShowHandler()
{
    qLogx()<<"";
    QMenu *addTaskMenuList = new QMenu(this);
    addTaskMenuList->addAction(this->mainUI->action_New_Download);
    addTaskMenuList->addAction(this->mainUI->action_New_Bittorrent);
    addTaskMenuList->addAction(this->mainUI->action_New_Metalink);
    addTaskMenuList->addAction(this->mainUI->actionAdd_batch_download); // batch
    this->mAddOtherTaskButton = new QToolButton(this);
    this->mAddOtherTaskButton->setPopupMode(QToolButton::MenuButtonPopup); // new, nice
    this->mAddOtherTaskButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->mainUI->toolBar->insertWidget(this->mainUI->action_Delete_task, this->mAddOtherTaskButton);
    this->mAddOtherTaskButton->setMenu(addTaskMenuList);
    this->mAddOtherTaskButton->setDefaultAction(this->mainUI->action_New_Download);

	this->mISHW = new InstantSpeedHistogramWnd(this->mainUI->speed_histogram_toolBar);
	this->mainUI->speed_histogram_toolBar->addWidget(this->mISHW);

	QScrollArea *sa = new QScrollArea(this->mainUI->tab_4);
	this->mainUI->tab_4->setLayout(new QVBoxLayout());
	this->mainUI->tab_4->layout()->addWidget(sa);

    // database inited here???
	TaskBallMapWidget *wg = TaskBallMapWidget::instance(sa);
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

	this->update();

	///////	
	// init base storage db 
    // this->mStorage = SqliteStorage::instance(this);
    // QObject::connect(this->mStorage, SIGNAL(opened()), this, SLOT(onStorageOpened()));
	// this->mStorage->open();

    //
    this->mCatView = this->mainUI->mui_tv_category;

    ////
    this->mTaskListView = this->mainUI->mui_tv_task_list;

    this->mSegListView = this->mainUI->mui_tv_seg_list;


    this->mSegLogListView = this->mainUI->mui_tv_seg_log_list;


    // 初始化任务队列管理实例
    // this->mTaskMan = TaskQueue::instance();

	this->update();

    // select the downloading cat Automaticly
    

    this->mSeedFileDelegate = new SeedFileItemDelegate();
    this->mSeedFileView = this->mainUI->treeView_2;
    this->mSeedFileView->setItemDelegate(this->mSeedFileDelegate);

	///////////////一些lazy模式实例化的类指针初始化。
	this->mWalkSiteWnd = 0;
	this->mWalkSiteDockWidget = 0;
	this->mHWalkSiteWndEx = 0;

	//
	this->connectAllSignalAndSlog();

    this->mainUI->action_Pause->setEnabled(false);
    this->mainUI->action_Start->setEnabled(false);
    this->mainUI->action_Delete_task->setEnabled(false);
    // this->onUpdateJobMenuEnableProperty();
	
	//this->showMaximized();

    // start backend
//    this->mAriaMan = new AriaMan();
//    this->mAriaMan->start();
//    this->mAriaGlobalUpdater.setInterval(5*1000);
//    QObject::connect(&this->mAriaGlobalUpdater, SIGNAL(timeout()),
//                     this, SLOT(onAriaGlobalUpdaterTimeout()));
//    this->mAriaGlobalUpdater.start();
//    // QObject::connect(this->mAriaMan, SIGNAL(taskLogReady(QString, QString, QString)),
//    //                  this->mTaskMan, SLOT(onTaskLogArrived(QString, QString, QString)));
//    QObject::connect(this->mAriaMan, SIGNAL(error(QProcess::ProcessError)),
//                     this, SLOT(onAriaProcError(QProcess::ProcessError)));
//    QObject::connect(this->mAriaMan, SIGNAL(finished(int, QProcess::ExitStatus)),
//                     this, SLOT(onAriaProcFinished(int, QProcess::ExitStatus)));
//    QObject::connect(this->mAriaMan, SIGNAL(taskLogReady(QString)),
//                     this, SLOT(onTaskLogArrived(QString)));

    //// start embeded backed
    // qRegisterMetaType<QMap<int, QVariant> >("QMAP_INT_QVARIANT");
    //    this->mEAria2Man = EAria2Man::instance();
    // QObject::connect(this->mEAria2Man, &EAria2Man::taskStatChanged, this->mTaskMan, &TaskQueue::onTaskStatusNeedUpdate2);
    // QObject::connect(this->mEAria2Man, &EAria2Man::taskFinished, this, &Karia2::onTaskDone);

	///////
	this->hideUnimplementUiElement();
	this->hideUnneededUiElement();


    // this->initUserOptionSetting();
    // process arguments 
    this->handleArguments();

    qLogx()<<"";

    // this->mSkype = new Skype("karia2");
    // QObject::connect(this->mSkype, SIGNAL(skypeError(int, QString)),
    //                  this, SLOT(onSkypeError(int, QString)));
    // bool cok = this->mSkype->connectToSkype();
    // qLogx()<<"skype connect status:"<<cok;

    // QObject::connect(this->mainUI->actionSkype_Tracer_2, SIGNAL(triggered(bool)),
    //                  this, SLOT(onShowSkypeTracer(bool)));
    // QObject::connect(this->mainUI->pushButton, SIGNAL(clicked()),
    //                  this, SLOT(onChatWithSkype()));
    // QObject::connect(this->mainUI->pushButton_2, SIGNAL(clicked()),
    //                  this, SLOT(onSendPackage()));
    // QObject::connect(this->mainUI->pushButton_4, SIGNAL(clicked()),
    //                  this, SLOT(onCallSkype()));
    // SkypeTunnel *tun = new SkypeTunnel(this);
    // tun->setSkype(this->mSkype);


}

void Karia2::asyncFirstShowHandler()
{
	// init base storage db 
    this->mStorage = SqliteStorage::instance(this);
    QObject::connect(this->mStorage, SIGNAL(opened()), this, SLOT(onStorageOpened()));
	this->mStorage->open();

    // 初始化任务队列管理实例
    this->mTaskMan = TaskQueue::instance();

    //// start embeded backed
    qRegisterMetaType<QMap<int, QVariant> >("QMap<int, QVariant>");
    this->mEAria2Man = EAria2Man::instance();
    QObject::connect(this->mEAria2Man, &EAria2Man::taskStatChanged, this->mTaskMan, &TaskQueue::onTaskStatusNeedUpdate2);
    QObject::connect(this->mEAria2Man, &EAria2Man::taskFinished, this, &Karia2::onTaskDone);

    this->initUserOptionSetting();


	//test area 　---------begin-----------------
	LabSpace *labspace = new LabSpace(this);
	//labspace->show();
    Q_UNUSED(labspace);

#ifdef Q_OS_WIN32
	// register system hot key 
	if (!::RegisterHotKey(this->winId(), 'C', MOD_CONTROL|MOD_SHIFT, 'C')) {
		qLogx()<<"::RegisterHotKey faild";
	}
#else	
#endif

	//test area 　---------end-----------------
}

Karia2::~Karia2()
{
//    this->mAriaMan->stop();
//    delete this->mAriaMan;
}
/**
 * init main window size, split size 
 */
void Karia2::initialMainWindow()
{
	//qLogx()<<__FUNCTION__;
	//set split on humanable postion , but not middle
	QList<int> splitSize;
	splitSize = this->mainUI->splitter_4->sizes();	
	splitSize[0] -= 70;
	splitSize[1] += 70;	
	this->mainUI->splitter_4->setSizes(splitSize);

	splitSize = this->mainUI->splitter_3->sizes();	
	splitSize[0] -= 100;
	splitSize[1] += 100;	
	this->mainUI->splitter_3->setSizes(splitSize);

	splitSize = this->mainUI->splitter_2->sizes();
	splitSize[0] -= 50;
	splitSize[1] += 50;	
	//this->mainUI->splitter_2->setSizes(splitSize);


	splitSize = this->mainUI->splitter->sizes();
	//qLogx()<<splitSize;
	splitSize[0] += 80;
    // splitSize[1] -= 80; // no log section now
	this->mainUI->splitter->setSizes(splitSize);

	//this->onSwitchToWindowsXPStyle(true);	
}

void Karia2::moveEvent (QMoveEvent * event )
{
	QPoint cp = event->pos();
	QPoint op = event->oldPos();

	
	QSize winSize = this->size();
	QRect winRect = this->rect();

	QPoint pos = this->pos();

	int dtwidth = QApplication::desktop()->width();
	int dtheight = QApplication::desktop()->height();
	//qLogx()<<event->pos()<<event->oldPos()<<"W: "<<dtwidth<<" H:"<<dtheight;

	//top 

	QMainWindow::moveEvent(event);

}

void Karia2::initPopupMenus()
{
	//action groups 
	QActionGroup *group = new QActionGroup(this);
	QObject::connect(group, SIGNAL(triggered(QAction *)), this, SLOT(onSwitchSpeedMode(QAction*)));
	group->addAction(mainUI->action_Unlimited);
	group->addAction(mainUI->action_Manual);
	group->addAction(mainUI->action_Automatic);

    // should dynamic check available styles
	//window style : "windows", "motif", "cde", "plastique", "windowsxp", or "macintosh" , "cleanlooks" 
	//NorwegianWood
	group = new QActionGroup(this);
	mainUI->action_NorwegianWood->setData("norwegianwood");
	group->addAction(mainUI->action_NorwegianWood);
    // automatic check supported style
    QStringList styleKeys = QStyleFactory::keys();
    QStyle *defaultStyle = QApplication::style();
    // qLogx()<<QApplication::style()<<styleKeys;
    for (int i = 0; i < styleKeys.count(); ++i) {
        QAction *styleAction = new QAction(styleKeys.at(i), this->mainUI->menuStyle);
        styleAction->setData(styleKeys.at(i));
        styleAction->setCheckable(true);
        if (styleKeys.at(i).toLower() == defaultStyle->objectName()) {
            styleAction->setChecked(true);
        }
        this->mainUI->menuStyle->addAction(styleAction);
        group->addAction(styleAction);
    }
	QObject::connect(group, SIGNAL(triggered(QAction *)), this, SLOT(onSwitchWindowStyle(QAction*)));

	//supported languages: en_US , zh_CN , zh_TW
	group = new QActionGroup(this);
	mainUI->action_Chinese_simple->setData("zh_CN");
	mainUI->action_Chinese_trad->setData("zh_TW");
	mainUI->action_English->setData("en_US");
	group->addAction(mainUI->action_Chinese_simple);
	group->addAction(mainUI->action_Chinese_trad);
	group->addAction(mainUI->action_English);
	QObject::connect(group, SIGNAL(triggered(QAction *)), this, SLOT(onSwitchLanguage(QAction*)));
	//mainUI->action_English->setChecked(true);

	/////skin
	group = new QActionGroup(this);
	group->addAction(mainUI->actionNone);
	group->addAction(mainUI->actionNormal);
	group->addAction(mainUI->actionXP_Luna);
	group->addAction(mainUI->actionXP_Luna_Gradient);
	group->addAction(mainUI->actionSkype_Gradient);
	group->addAction(mainUI->actionCustom_Background);
	group->addAction(mainUI->actionImageBk);
	QObject::connect(group, SIGNAL(triggered(QAction *)), this, SLOT(onSwitchSkinType(QAction*)));


	/////// all popups
	this->mTaskPopupMenu = new QMenu("&Task", this);
		
	QList<QAction*> actionList;
	actionList.append(this->mainUI->action_Start);
	actionList.append(this->mainUI->action_Pause);
	actionList.append(this->mainUI->action_Start_All);
	actionList.append(this->mainUI->action_Pause_All);
	actionList.append(this->mainUI->action_Schedule);
	this->mTaskPopupMenu->addActions(actionList);
	this->mTaskPopupMenu->addSeparator();
	actionList.clear();
	this->mTaskPopupMenu->addAction(this->mainUI->action_Move_top);
	this->mTaskPopupMenu->addAction(this->mainUI->action_Move_bottom);
	this->mTaskPopupMenu->addSeparator();
	this->mTaskPopupMenu->addAction(this->mainUI->action_Delete_task);
	this->mTaskPopupMenu->addSeparator();
	this->mTaskPopupMenu->addAction(this->mainUI->action_Properties);
	this->mTaskPopupMenu->addAction(this->mainUI->action_Site_Properties);
	this->mTaskPopupMenu->addAction(this->mainUI->action_Comment);
	this->mTaskPopupMenu->addAction(this->mainUI->action_Browse_Referer );
	this->mTaskPopupMenu->addAction(this->mainUI->action_Browse_With_Site_Explorer );
	this->mTaskPopupMenu->addSeparator();
	this->mTaskPopupMenu->addAction(this->mainUI->action_Download_Again );
	this->mTaskPopupMenu->addSeparator();
	this->mTaskPopupMenu->addAction(this->mainUI->action_Copy_URL_To_ClipBoard );

	/////////////
	this->mTaskPopupMenu->addActions(actionList);

	actionList.clear();
	actionList.append(this->mainUI->action_Seg_Log_Copy);
	actionList.append(this->mainUI->actionSelect_All);
	actionList.append(this->mainUI->action_Seg_Log_Save_To_File);
	actionList.append(this->mainUI->action_Clear_Seg_Log);

	this->mLogPopupMenu = new QMenu("&SegLog", this);
	this->mLogPopupMenu->addActions(actionList);

	actionList.clear();
	actionList.append(this->mainUI->action_Seg_List_Start);
	actionList.append(this->mainUI->action_Seg_List_Stop);
	actionList.append(this->mainUI->action_Seg_List_Restart);
	actionList.append(this->mainUI->action_Increase_split_parts);
	actionList.append(this->mainUI->action_Decrease_split_parts);

	this->mSegmentPopupMenu = new QMenu("&SegList", this);
	this->mSegmentPopupMenu->addActions(actionList);	

	this->mCatPopupMenu = new QMenu("&Category", this);
	actionList.clear();
	actionList.append(this->mainUI->actionNew_Cagegory);
	actionList.append(this->mainUI->action_Delete_cat);
	actionList.append(this->mainUI->action_cat_Move_To);
	actionList.append(this->mainUI->action_Cat_Property);
	this->mCatPopupMenu->addActions(actionList);


	//drop zone
	this->mDropZonePopupMenu = new QMenu("&DropZone", this);
	this->mDropZonePopupMenu->addAction(this->mainUI->action_Show_Hide_Main_Widow);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI->action_Monitor_Clipboard);
	this->mDropZonePopupMenu->addAction(this->mainUI->action_Disable_Browser_Monitor);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI->action_Start_All);
	this->mDropZonePopupMenu->addAction(this->mainUI->action_Pause_All);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI->action_New_Download);
	this->mDropZonePopupMenu->addAction(this->mainUI->actionAdd_batch_download);
	this->mDropZonePopupMenu->addAction(this->mainUI->actionPaste_URL);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addMenu(this->mainUI->menuSpeed_Limit_Mode);
	this->mDropZonePopupMenu->addSeparator ();	
	this->mDropZonePopupMenu->addMenu(this->mainUI->menu_Search);
	
	this->mDropZonePopupMenu->addAction(this->mainUI->action_Site_Explorer);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI->action_Options);
	this->mDropZonePopupMenu->addAction(this->mainUI->action_About_Karia2);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI->actionQuit);

	//mSysTrayMenu
	this->mSysTrayMenu = new QMenu("NullTray", this);
	this->mSysTrayMenu->addAction(this->mainUI->action_Show_Hide_Main_Widow);
	this->mSysTrayMenu->addSeparator();
	this->mSysTrayMenu->addAction(this->mainUI->actionQuit);
	this->mSysTrayMenu->addAction(this->mainUI->actionAbout_Qt);

}

void Karia2::initStatusBar()
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
 *
 */
void Karia2::initSystemTray()
{
	//init system tray icon
	this->mSysTrayIcon = new QSystemTrayIcon(this);
	this->mSysTrayIcon->setToolTip(tr("karia2 Icon Tray Control"));
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
        // iconname = QString(qApp->applicationDirPath() + "/Resources/karia2-1.png");
        iconname = QString(qApp->applicationDirPath() + "/" +"Resources/karia2.png");
        break;
    }

    QPixmap pix(iconname);
	this->mSysTrayIcon->setIcon(QIcon(pix));

	this->mSysTrayIcon->show();

	//this->setWindowIcon(QIcon(qApp->applicationDirPath()+"/"+"resources/icon_16x16.png"));	
    // this->setWindowIcon(QIcon(qApp->applicationDirPath()+"/Resources/karia2-1.png"));	
    this->setWindowIcon(QIcon(qApp->applicationDirPath()+"/Resources/karia2.png"));	
}

void Karia2::initAppIcons()
{
    QString dir = qApp->applicationDirPath() + "/icons";
    this->mainUI->action_Start->setIcon(QIcon(dir + "/media-playback-start.png"));
    this->mainUI->action_Pause->setIcon(QIcon(dir + "/media-playback-pause.png"));
    this->mainUI->action_New_Download->setIcon(QIcon(dir + "/list-add.png"));
    this->mainUI->action_Delete_task->setIcon(QIcon(dir + "/list-remove.png"));
    this->mainUI->action_Properties->setIcon(QIcon(dir + "/document-properties.png"));
    this->mainUI->actionMove_Up->setIcon(QIcon(dir + "/arrow-up.png"));
    this->mainUI->actionMove_Down->setIcon(QIcon(dir + "/arrow-down.png"));
    this->mainUI->actionOpen_Exec_download_file->setIcon(QIcon(dir + "/system-run.png"));
    this->mainUI->actionOpen_de_stination_directory->setIcon(QIcon(dir + "/document-open-folder.png"));
    this->mainUI->actionFind->setIcon(QIcon(dir + "/edit-find.png"));
    this->mainUI->actionFind_next->setIcon(QIcon(dir + "/go-down-search.png"));
    // this->mainUI->actionDefault_Download_Properties->setIcon(QIcon(dir + "/configure.png"));
    this->mainUI->action_Options->setIcon(QIcon(dir + "/preferences-system.png"));
    this->mainUI->actionQuit->setIcon(QIcon(dir + "/system-shutdown.png"));
    this->mainUI->label->setPixmap(QPixmap(dir + "/status/unknown.png"));
}

void Karia2::connectAllSignalAndSlog()
{
    return;
	QObject::connect(this->mainUI->actionDrop_zone, SIGNAL(triggered(bool)), this->mDropZone,SLOT(setVisible(bool)));
	
	//file action
	//QObject::connect(this->mainUI->action_New_Database, SIGNAL(triggered()), this, SLOT(onNewDatabase()));	
	//QObject::connect(this->mainUI->action_Open_Database, SIGNAL(triggered()), this, SLOT(onOpenDatabase()));	
	//QObject::connect(this->mainUI->action_Save_Database, SIGNAL(triggered()), this, SLOT(onSaveAllTask()));	
	//QObject::connect(this->mainUI->actionSave_As, SIGNAL(triggered()), this, SLOT(onSaveAllTaskAs()));

	QObject::connect(this->mainUI->actionProcess_Web_Page_File, SIGNAL(triggered()), this, SLOT(showProcessWebPageInputDiglog()));

	//editr
	QObject::connect(this->mainUI->actionPaste_URL , SIGNAL(triggered()), this, SLOT(showNewDownloadDialog()));
	QObject::connect(this->mainUI->actionSelect_All, SIGNAL(triggered()), this, SLOT(onEditSelectAll()));
	QObject::connect(this->mainUI->actionInvert_Select, SIGNAL(triggered()), this, SLOT(onEditInvertSelect()));
	
	//cat action
	QObject::connect(this->mainUI->actionNew_Cagegory , SIGNAL(triggered()), this, SLOT(onNewCategory()));
	QObject::connect(this->mainUI->action_Cat_Property , SIGNAL(triggered()), this, SLOT(onShowCategoryProperty()));
	QObject::connect(this->mainUI->action_Delete_cat, SIGNAL(triggered()), this, SLOT(onDeleteCategory()));
	QObject::connect(this->mainUI->action_cat_Move_To , SIGNAL(triggered()), this, SLOT(onCategoryMoveTo()));

	//view
	QObject::connect(this->mainUI->action_Show_Columns_Editor, SIGNAL(triggered()), this, SLOT(onShowColumnEditor()));
	QObject::connect(this->mainUI->actionShow_Text, SIGNAL(triggered(bool)), this, SLOT(onShowToolbarText(bool) ) );
	//job action 
	QObject::connect(this->mainUI->menu_Jobs, SIGNAL(aboutToShow()), this, SLOT(onTaskListMenuPopup()));
	QObject::connect(this->mainUI->action_New_Download, SIGNAL(triggered()), this, SLOT(showNewDownloadDialog()));
    QObject::connect(this->mainUI->action_New_Bittorrent, SIGNAL(triggered()), this, SLOT(showNewBittorrentFileDialog())); 
    QObject::connect(this->mainUI->action_New_Metalink, SIGNAL(triggered()), this, SLOT(showNewMetalinkFileDialog()));
	QObject::connect(this->mainUI->actionAdd_batch_download, SIGNAL(triggered()), this, SLOT(showBatchDownloadDialog()));
	
	QObject::connect(this->mainUI->action_Start , SIGNAL(triggered()), this, SLOT(onStartTask()));
	QObject::connect(this->mainUI->action_Pause , SIGNAL(triggered()), this, SLOT(onPauseTask()));
	QObject::connect(this->mainUI->action_Start_All , SIGNAL(triggered()), this, SLOT(onStartTaskAll()));
	QObject::connect(this->mainUI->action_Pause_All , SIGNAL(triggered()), this, SLOT(onPauseTaskAll()));

	QObject::connect(this->mainUI->action_Delete_task, SIGNAL(triggered()), this, SLOT(onDeleteTask()));

	QObject::connect(this->mainUI->action_Properties, SIGNAL(triggered()), this, SLOT(onShowTaskProperty()));

	QObject::connect(this->mainUI->actionOpen_de_stination_directory, SIGNAL(triggered()), this, SLOT(onOpenDistDirector()));
	QObject::connect(this->mainUI->actionOpen_Exec_download_file, SIGNAL(triggered()), this, SLOT(onOpenExecDownloadedFile()));
	QObject::connect(this->mainUI->action_Browse_Referer, SIGNAL(triggered()), this, SLOT(onOpenRefererUrl()));
	
	//tools 
	QObject::connect(this->mainUI->action_Options, SIGNAL(triggered()), this, SLOT(onShowOptions()));
	
	// QObject::connect(this->mainUI->actionDefault_Download_Properties , SIGNAL(triggered()), this, SLOT(onShowDefaultDownloadProperty()));

	//statusbar
	QObject::connect(this->mSpeedBarSlider, SIGNAL(valueChanged(int)), this, SLOT(onManualSpeedChanged(int)));
    QObject::connect(this->mainUI->action_Remember, SIGNAL(triggered(bool)),
                     this, SLOT(onRememberSpeedLimitSetting(bool)));
    
	//help action 
	this->connect(this->mainUI->action_Go_to_Karia2_Home_Page, SIGNAL(triggered()), this, SLOT(onGotoHomePage()));
	this->connect(this->mainUI->action_About_Karia2, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
	QObject::connect(this->mainUI->actionAbout_Qt, SIGNAL(triggered()), this, SLOT(onAboutQt()));

	//centrol 
	QObject::connect(this->mainUI->mui_tv_task_list, SIGNAL(customContextMenuRequested (const QPoint &  )),
		this, SLOT(onTaskListMenuPopup(/*const QPoint &*/)));
	QObject::connect(this->mainUI->mui_tv_task_list, SIGNAL(entered(const QModelIndex & )) ,
		this, SLOT(onShowTaskPropertyDigest(const  QModelIndex & ) ) );

	QObject::connect(this->mainUI->mui_tv_seg_log_list, SIGNAL(customContextMenuRequested (const QPoint &  )),
		this, SLOT(onLogListMenuPopup(const QPoint &)));
	QObject::connect(this->mainUI->mui_tv_seg_list, SIGNAL(customContextMenuRequested (const QPoint &  )),
		this, SLOT(onSegListMenuPopup(const QPoint &)));
	QObject::connect(this->mainUI->mui_tv_category, SIGNAL(customContextMenuRequested (const QPoint &  )),
		this, SLOT(onCateMenuPopup(const QPoint &)));


	QObject::connect(this->mDropZone, SIGNAL(doubleclicked()), this, SLOT(onDropZoneDoubleClicked()));
	QObject::connect(this->mainUI->action_Show_Hide_Main_Widow, SIGNAL(triggered()), this, SLOT(onDropZoneDoubleClicked()));
	QObject::connect(this->mDropZone, SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(onDropZoneCustomMenu(const QPoint &)));
	
	//seg log menu
	QObject::connect(this->mainUI->action_Seg_Log_Copy, SIGNAL(triggered()), this, SLOT(onCopySelectSegLog()));
	QObject::connect(this->mainUI->action_Seg_Log_Save_To_File, SIGNAL(triggered()), this, SLOT(onSaveSegLog()));
	QObject::connect(this->mainUI->action_Clear_Seg_Log, SIGNAL(triggered()), this, SLOT(onClearSegLog()));


	//toolbar	//在UI设计器中将信号传递到标准菜单中。
	//QObject::connect(this->mainUI->mui_tb_properties, SIGNAL(triggered()), this, SLOT(onShowTaskProperty()));
	//QObject::connect(this->mainUI->mui_tb_open_dir, SIGNAL(triggered()), this, SLOT(onOpenDistDirector()));
	//QObject::connect(this->mainUI->mui_tb_exec_file, SIGNAL(triggered()), this, SLOT(onOpenExecDownloadedFile()));

	//other
	QObject::connect(this->mainUI->action_Copy_URL_To_ClipBoard, SIGNAL(triggered()), this, SLOT(onCopyUrlToClipboard()));

	QObject::connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(onClipBoardDataChanged()));

	QObject::connect(this->mainUI->actionWalk_Site, SIGNAL(triggered()), this, SLOT(onShowWalkSiteWindow()));
	QObject::connect(this->mSysTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(onActiveTrayIcon(QSystemTrayIcon::ActivationReason)));
	QObject::connect(this->mSysTrayIcon, SIGNAL(messageClicked()),  this, SLOT(onBallonClicked()));

	//test it
	//this->connect(this->mainUI->pushButton_3, SIGNAL(clicked()), this, SLOT(testFunc()));
	//QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit () ), this, SLOT(testFunc2()));

}
//temporary 临时用于隐藏没有实现功能用户界面的代码。
void Karia2::hideUnimplementUiElement()
{
    this->mainUI->menu_Search->menuAction()->setVisible(false);
    this->mainUI->menu_Plugins->menuAction()->setVisible(false);

    // action
	this->mainUI->action_New_Database->setVisible(false);
	this->mainUI->action_Open_Database->setVisible(false);
	this->mainUI->action_Merge_Database->setVisible(false);
	this->mainUI->action_Save_Database->setVisible(false);
	this->mainUI->actionSave_As->setVisible(false);

	this->mainUI->actionImport_Previous_File->setVisible(false);
	this->mainUI->actionImport_Previous_Batch_File->setVisible(false);
	this->mainUI->action_Export_Information->setVisible(false);
	this->mainUI->action_Export->setVisible(false);
	this->mainUI->actionImport_Broken_D_ownloads->setVisible(false); 
	this->mainUI->actionImport_list->setVisible(false);
	this->mainUI->action_Export_list->setVisible(false);

	this->mainUI->action_Recently_Downloaded_Files->setVisible(false);
    this->mainUI->actionProcess_Web_Page_File->setVisible(false);

	//cat
	this->mainUI->action_cat_Move_To->setVisible(false);

	//edit
	this->mainUI->actionFind->setVisible(false);
	this->mainUI->actionFind_next->setVisible(false);

	//view
	this->mainUI->actionDetail->setVisible(false);
	this->mainUI->actionGrid->setVisible(false);
    this->mainUI->menuSkin->menuAction()->setVisible(false);
	this->mainUI->actionButtons->setVisible(false);	

	//task
	this->mainUI->action_Move_bottom->setVisible(false);
	this->mainUI->action_Move_top->setVisible(false);
	this->mainUI->action_task_Move_To->setVisible(false);
	this->mainUI->actionMove_Down->setVisible(false);
	this->mainUI->actionMove_Up->setVisible(false);
	this->mainUI->action_Check_for_update->setVisible(false);
	
	//tool
	this->mainUI->action_Browse_With_Site_Explorer->setVisible(false);
    this->mainUI->action_Site_Explorer->setVisible(false);
    this->mainUI->actionWalk_Site->setVisible(false);
	this->mainUI->action_Download_Rules->setVisible(false);
	// this->mainUI->action_Save_as_default->setVisible(false);

	//help
	this->mainUI->action_User_Manual_in_Internet->setVisible(false);

	this->mainUI->action_Manual_Karia2->setVisible(false);
	this->mainUI->actionC_heck_for_a_new_version->setVisible(false);
	this->mainUI->actionFAQ_in_Internet->setVisible(false);

	//other
	this->mainUI->action_Seg_List_Restart->setVisible(false);
}

void Karia2::onStorageOpened()
{
    qLogx()<<"";
    //
    this->mCatView = this->mainUI->mui_tv_category;
    this->mCatView->setSelectionMode(QAbstractItemView::SingleSelection );
    this->mCatViewModel = SqliteCategoryModel::instance(0);
    this->mCatView->setModel(this->mCatViewModel);
    this->mCatView->setRootIndex(this->mCatViewModel->index(0, 0)); // root is the topest node
    this->mCatView->expandAll();

    this->update();

    ////
    this->mTaskListView = this->mainUI->mui_tv_task_list;
    this->mTaskItemDelegate = new TaskItemDelegate();
    this->mTaskListView->setItemDelegate(this->mTaskItemDelegate);
    this->mTaskTreeViewModel = SqliteTaskModel::instance(ng::cats::downloading, this);
    this->mTaskListView->setModel(this->mTaskTreeViewModel);
    this->mTaskListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->mTaskListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->mTaskListView->setAlternatingRowColors(true);
    // QObject::connect(this->mTaskListView->selectionModel(),
    //                  SIGNAL(selectionChanged (const QItemSelection & , const QItemSelection &   )),
    //                  this, SLOT(onTaskListSelectChange(const QItemSelection & , const QItemSelection &   ) ) );

    this->mSegListView = this->mainUI->mui_tv_seg_list;
    this->mSegListView->setSelectionMode(QAbstractItemView::SingleSelection );
    this->mSegListView->setAlternatingRowColors(true);
    //this->mSegListView ->setModel(mdl);

    this->mSegLogListView = this->mainUI->mui_tv_seg_log_list;
    this->mSegLogListView->setSelectionMode(QAbstractItemView::ExtendedSelection );

    //cat view
    QObject::connect(this->mCatView->selectionModel(), SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
        this, SLOT(onCatListSelectChange(const QItemSelection & , const QItemSelection &   ) ) );

    // select the default cat model
    {
        // cacl the downloading model index
        QItemSelection readySelect, readDeselect;
        QModelIndex topCatIdx = this->mCatViewModel->index(0, 0);
        qLogx()<<topCatIdx.data();
        int l2RowCount = this->mCatViewModel->rowCount(topCatIdx);
        qLogx()<<l2RowCount;
        for (int r = 0 ; r < l2RowCount ; r ++) {
            QModelIndex currCatIdx = this->mCatViewModel->index(r, ng::cats::cat_id, topCatIdx);
            qLogx()<<currCatIdx;
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
}

void Karia2::hideUnneededUiElement()
{

}

void Karia2::initUserOptionSetting()
{
    OptionManager *om = NULL;

    om = OptionManager::instance();

    //////
    QStringList colList;
    QString taskShowColumns = om->getTaskShowColumns();
    // qLogx()<<__FUNCTION__<<taskShowColumns<<(ng::tasks::aria_gid);
    if (taskShowColumns != "") {
        colList = taskShowColumns.split(',');
        for (int i = ng::tasks::task_id ; i <= ng::tasks::aria_gid; ++i) {
            // qLogx()<<"show col:"<<colList.contains(QString::number(i));
            if (!colList.contains(QString::number(i))) {
                this->mainUI->mui_tv_task_list->setColumnHidden(i, true);
            }
        }
        if (this->mCustomTaskShowColumns != taskShowColumns) {
            this->mCustomTaskShowColumns = taskShowColumns;
        }
    }

    //////
    QString rememberSpeedLimit = om->getRememberSpeedLimit();
    if (rememberSpeedLimit == "true") {
        this->mainUI->action_Remember->setChecked(true);
        QString speedLimitType = om->getSpeedLimitType();
        if (speedLimitType == "unlimited") {
        } else if (speedLimitType == "manual") {
            this->mainUI->action_Manual->setChecked(true);
            QString speedLImitSpeed = om->getSpeedLimitSpeed();
            this->onManualSpeedChanged(speedLImitSpeed.toInt()/1000);
            this->mSpeedBarSlider->setValue(speedLImitSpeed.toInt()/1000);
            this->onSwitchSpeedMode(this->mainUI->action_Manual);
        } else if (speedLimitType == "auto") {
            this->mainUI->action_Automatic->setChecked(true);
            this->onSwitchSpeedMode(this->mainUI->action_Automatic);
        }
    }
}


void Karia2::testFunc()
{
	qLogx()<<__FUNCTION__;
	// int count = 7;	
	this->mTaskPopupMenu->popup(QCursor::pos ());
	return;

	this->onStartTask();
	return;
	
	return;

	QString url = "http://localhost/mtv.wmv";
	int nRet = 0;
	qLogx() <<nRet;

}

void Karia2::testFunc2()
{
	qLogx()<<__FUNCTION__;

	return;

	// QStandardItemModel *model = 	mConfigDatabase->createCatModel();	
	// this->mCatView->setModel( model );
	// this->mCatView->expand(model->index(0,0));

	return;

	this->onPauseTask();

}


// TODO, handle multi row select case
void Karia2::onStartTask()
{
	qLogx()<<__FUNCTION__<<__LINE__;	
	QAbstractItemView *view = this->mTaskListView;
    QAbstractItemModel *model = view->model();
    int step = model->columnCount();
    
	int taskId = -1;
    QString url;
	QModelIndexList smil = view->selectionModel()->selectedIndexes();
	//qLogx()<<smil.size();
    if (smil.size() == 0) {
        return;
    }

    // this->initXmlRpc();

    for (int row = 0; row < smil.count() ; row += step) {
        TaskOption *taskOptions = TaskOption::fromModelRow(model, smil.at(row).row());
        taskId = smil.value(row + ng::tasks::task_id).data().toInt();
        url = smil.value(row + ng::tasks::org_url).data().toString();

        // if running
//        if (this->mRunningMap.contains(taskId)) {
//            continue;
//        }

        QMap<QString, QVariant> payload;
        QVariantList args;
        QList<QVariant> uris;
        QMap<QString, QVariant> options;
        QString aria2RpcMethod;

        {
            // create task object first
            payload["taskId"] = QString("%1").arg(taskId);
            payload["url"] = url;
            this->taskUi->createTask(taskId, taskOptions);
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

        this->mEAria2Man->addUri(taskId, url, taskOptions);

//        this->mAriaRpc->call(aria2RpcMethod, args, QVariant(payload),
//                             this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
//                             this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

//        if (!this->mAriaUpdater.isActive()) {
//            this->mAriaUpdater.setInterval(3000);
//            QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
//            this->mAriaUpdater.start();
//        }
//        if (!this->mAriaTorrentUpdater.isActive()) {
//            this->mAriaTorrentUpdater.setInterval(4000);
//            QObject::connect(&this->mAriaTorrentUpdater, SIGNAL(timeout()), this, SLOT(onAriaTorrentUpdaterTimeout()));
//            this->mAriaTorrentUpdater.start();
//        }

        delete taskOptions; taskOptions = NULL;

    }
}

void Karia2::onStartTaskAll()
{
	qLogx()<<__FUNCTION__;

	TaskQueue * hTask = 0;
	// int taskCount;
	// int retrCount;

	//taskCount = this->mTaskQueue.size();
	//for (int i = 0; i < taskCount; i ++)
	{
		//hTask = this->mTaskQueue[i];
		if (hTask != 0 )
		{
			//this->onStartTask(hTask->mTaskId);
			//this->onStartTask(hTask );
		}
	}
}


void Karia2::onPauseTask()
{
	qLogx()<<__FUNCTION__;
	
	QAbstractItemView *view = this->mTaskListView;
	QAbstractItemModel *model = view->model(); // SqliteTaskModel::instance(ng::cats::downloading, this);
	int step = model->columnCount();
	int taskId = -1;
    QString ariaGid;
	QModelIndexList smil = view->selectionModel()->selectedIndexes();
	qLogx()<<__FUNCTION__<<"selectedIndexes count:"<<smil.size();

	if (smil.size() == 0) {
        return;
    }
    
//    this->initXmlRpc();
//    Q_ASSERT(this->mAriaRpc != NULL);

//    for (int i = 0; i < smil.size(); i += step) {
//        taskId = smil.value(i).data().toInt();
//        ariaGid = smil.value(i + ng::tasks::aria_gid).data().toString();
//        qLogx()<<__FUNCTION__<<smil.value(i)<<taskId;

//        // check if running
//        if (!this->mRunningMap.contains(taskId)) {
//            continue;
//        }

//        QVariantList args;

//        args << ariaGid;
//        this->mAriaRpc->call(QString("aria2.remove"), args, QVariant(taskId),
//                             this, SLOT(onAriaRemoveResponse(QVariant &, QVariant&)),
//                             this, SLOT(onAriaRemoveFault(int, QString, QVariant&)));
//    }
}
void Karia2::onPauseTask(int pTaskId )
{
	qLogx()<<__FUNCTION__;
    Q_UNUSED(pTaskId);

	// BaseRetriver * hRetr = 0;
	// int retrCount;
	TaskQueue * hTask = this->mTaskMan;
	if (hTask == 0)	{
		return;
	} else {
		//this->onPauseTask(hTask);
	}

	return;
}

void Karia2::onPauseTaskAll()
{
	qLogx()<<__FUNCTION__;
	TaskQueue * hTask = 0;
	// int taskCount;

	//taskCount = this->mTaskQueue.size();
	//for (int i = 0; i < taskCount; i ++)
	{
	//	hTask = this->mTaskQueue[i];
		if (hTask != 0 )
		{
		//	this->onPauseTask(hTask->mTaskId);
		}
	}

}

/////delete option
/**
 * 将任务数据移动到删除队列
 */
void Karia2::onDeleteTask()
{
	qLogx()<<__FUNCTION__;
	
	SqliteTaskModel * from_model = 0 , * to_model = 0;
	QModelIndex idx;
	QItemSelectionModel * sim = 0;
	QModelIndexList mil;
	// int rowCount = 0;
	// int row = -1;
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
        // qLogx()<<"prepare delete ROW:"<<mrow<<" All ROW:"<<rowcnt;
		// int taskId = from_model->data(mil.value(row * colcnt + ng::tasks::task_id)).toInt();
		QString ariaGid = from_model->data(mil.value(row * colcnt + ng::tasks::aria_gid)).toString();
        // int srcCatId = from_model->data(mil.value(row * colcnt + ng::tasks::sys_cat_id)).toInt();

        // qLogx()<<"DDDD:"<<taskId<<ariaGid<<srcCatId;
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
        qLogx()<<"DDDD:"<<mrow<<taskId<<ariaGid<<srcCatId;
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
            if (rv <= 0) {
            }
			//在system tray 显示移动任务消息
			this->mSysTrayIcon->showMessage(tr("Move Task ."), QString(tr("Move To Trash Now. TaskId: %1")).arg(taskId),
                                            QSystemTrayIcon::Information, 5000);
        }
        
    }

    QApplication::restoreOverrideCursor();
}

void Karia2::onDeleteTaskAll()
{
	qLogx()<<__FUNCTION__;
	QModelIndex index;
	QAbstractItemModel *model;
	// int rowCount = -1;

	model = SqliteTaskModel::instance(ng::cats::downloading, this);

	for (int i = 0; i < model->rowCount(); ++ i) {
		index = model->index(i,0);
		int taskId = model->data(index).toInt();
		// this->onDeleteTask(taskId);
        Q_UNUSED(taskId); 
	}
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

void Karia2::onTaskDone(int pTaskId, int code)
{
	qLogx()<<__FUNCTION__;

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
        qLogx()<<"find cat recursive:"<<destCatId<<readyIdx;
        readySelect.select(this->mCatViewModel->index(readyIdx.row(), 0, readyIdx.parent()), 
                           this->mCatViewModel->index(readyIdx.row(), ng::cats::dirty, readyIdx.parent()));
        
        this->mCatView->selectionModel()->select(readySelect, QItemSelectionModel::ClearAndSelect);
    }
    
    return;
}

void Karia2::onShutdown()
{
    qLogx()<<__FUNCTION__<<" shutdown OS now";
}

//////ui op

void Karia2::onNewCategory()
{
	CatManDlg * dlg = new CatManDlg (this );
	QAbstractItemModel * aim;
	QItemSelectionModel * ism;
	QModelIndexList mil;

	int rv = dlg->exec();
	if (rv == QDialog::Accepted) {
		QModelIndex idx;
		QString dir;
		QString catname;
		int row;
		
		dir = dlg->getNewCatDir();
		catname = dlg->getNewCatName();

		ism = dlg->getSelectionModel();

		aim = dlg->getCatModel();		
		//qLogx()<<dlg->getCatModel();
		//
		mil = ism->selectedIndexes();
		qLogx()<<__FUNCTION__<<mil.size();
		if (mil.size() >  0 ) {
			row = mil.at(0).model()->rowCount(mil.at(0));
			aim->insertRows(row, 1, mil.at(0));
			idx = aim->index(row, ng::cats::display_name, mil.at(0));
			aim->setData(idx, catname);
			idx = aim->index(row, ng::cats::path, mil.at(0));
			aim->setData(idx, dir);
			
			qLogx()<<"insertt "<<row<<aim->data(idx);
			
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

void Karia2::onShowCategoryProperty()
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

	er = dlg->exec();	//show it

	if (er == QDialog::Accepted) {
		//qLogx()<<dlg->getCatModel();
	} else {

	}
	delete dlg;	
}

void Karia2::onDeleteCategory()
{	
	// QItemSelectionModel * ism;
	QModelIndexList mil;
	QModelIndex idx , parent;

	mil = this->mCatView->selectionModel()->selectedIndexes();

	if (mil.size() > 0) {
		idx = mil.at(0);
		if (idx == this->mCatViewModel->index(0,0)) return;	//can not delete the default category
		if (idx == this->mCatViewModel->index(0,0, this->mCatViewModel->index(0,0))) return;
		if (idx == this->mCatViewModel->index(1,0, this->mCatViewModel->index(0,0))) return;
		if (idx == this->mCatViewModel->index(2,0, this->mCatViewModel->index(0,0))) return;

		if (QMessageBox::question(this, tr("Delete Category:"),
                                  tr("Delete the Category and All sub Category?"),
                                  QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel) {
			return;
        }

		parent = idx.parent();
		this->mCatViewModel->removeRows(idx.row(),1,parent);

		//this->mCatView->collapse(parent);
		//this->mCatView->expand(parent);
	}
}

void Karia2::onCategoryMoveTo()
{
	// QItemSelectionModel * ism;
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
void Karia2::onShowColumnEditor()
{
	QDialog *dlg = new ColumnsManDlg(this);
    QObject::connect(dlg, SIGNAL(taskShowColumnsChanged(QString)),
                     this, SLOT(onTaskShowColumnsChanged(QString)));

	dlg->exec();

	delete dlg;
}


//void Karia2::initXmlRpc()
//{
//    if (this->mAriaRpc == NULL) {
//        // the default port is 6800 on best case, change to 6800+ if any exception.
//        this->mAriaRpc = new MaiaXmlRpcClient(QUrl(QString("http://127.0.0.1:%1/rpc").arg(this->mAriaMan->rpcPort())));

//        // get version and session
//        // use aria2's multicall method. note: has moved to AriaMan
//        // QVariantMap  getVersion;
//        // QVariantMap getSession;
//        // QVariantList gargs;
//        // QVariantList args;
//        // QVariantMap options;
//        // QVariant payload;
        
//        // getVersion["methodName"] = QString("aria2.getVersion");
//        // getVersion["params"] = QVariant(options);
//        // getSession["methodName"] = QString("aria2.getSessionInfo");
//        // getSession["params"] = QVariant(options);

//        // args.insert(0, getVersion);
//        // args.insert(1, getSession);

//        // gargs.insert(0, args);

//        // this->mAriaRpc->call(QString("system.multicall"), gargs, payload,
//        //                      this, SLOT(onAriaMultiCallVersionSessionResponse(QVariant&, QVariant&)),
//        //                      this, SLOT(onAriaMultiCallVersionSessionFault(int, QString, QVariant &)));
//    }
//}

//QMap<QString, QVariant> Karia2::taskOptionToAria2RpcOption(TaskOption *to)
//{
//    Q_ASSERT(to != NULL);
//    QMap<QString, QVariant> aopts;

//    if (!to->mReferer.isEmpty()) {
//        aopts["referer"] = to->mReferer;
//    } else {
//        aopts["referer"] = to->mTaskUrl;
//    }

//    if (!to->mSavePath.isEmpty()) {
//        aopts["dir"] = to->mSavePath;
//    }

//    if (to->mSplitCount > 0) {
//        aopts["split"] = QString("%1").arg(to->mSplitCount);
//    }

//    if (!to->mCookies.isEmpty()) {
//        QVariantList header;
//        header << QString("Cookie: %1").arg(to->mCookies);
//        aopts["header"] = header;
//        if (!to->mReferer.isEmpty()) {
//          header << QString("Referer: %1").arg(to->mReferer);
//        }
//        if (!to->mAgent.isEmpty()) {
//          header << QString("User-Agent: %1").arg(to->mAgent);
//        }
//    }

//    return aopts;
//}


/////////////
void Karia2::onShowOptions()
{
	QDialog *dlg = new PreferencesDialog(this);

	dlg->exec();

	delete dlg;

}

// void Karia2::onShowConnectOption()
// {
// 	OptionDlg *dlg = new OptionDlg(this);

// 	dlg->exec();

// 	delete dlg;
// }

// void Karia2::onShowDefaultDownloadProperty()
// {
// 	taskinfodlg *tid = new taskinfodlg(0,0);		
	
// 	//tid->setRename(fname);			
// 	int er = tid->exec();	
	
// 	delete tid;	
// }

void Karia2::onEditSelectAll()
{
	qLogx()<<__FUNCTION__;
	QWidget * wg = focusWidget ();

	if (wg == this->mainUI->mui_tv_task_list) {
		this->mainUI->mui_tv_task_list->selectAll();
	} else if (wg == this->mainUI->mui_tv_seg_log_list ) {
		this->mainUI->mui_tv_seg_log_list->selectAll();
	}
	qLogx()<<wg;

}
void Karia2::onEditInvertSelect()
{
	qLogx()<<__FUNCTION__;
	QWidget * wg = focusWidget ();
	QModelIndex idx;
	QTreeView *tv;
	QItemSelectionModel *sm;
	if (wg == this->mainUI->mui_tv_task_list)
	{
		tv = this->mainUI->mui_tv_task_list;
	}
	else if (wg == this->mainUI->mui_tv_seg_log_list )
	{
		tv = this->mainUI->mui_tv_seg_log_list;
	}
	else
	{
		return;
	}
	sm = tv->selectionModel();
	QList<int> subrows;

	int rows = tv->model()->rowCount();
	// int cols = tv->model()->columnCount();
	for (int i = 0;i < rows;  i ++ )
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
	for (int i = 0; i < subrows.size();i ++)
	{
		sm->select(tv->model()->index(subrows[i],0),QItemSelectionModel::Select|QItemSelectionModel::Rows);
	}

	qLogx()<<wg;

}

void Karia2::onShowToolbarText(bool show) 
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

void Karia2::onCopyUrlToClipboard()
{
	qLogx()<<__FUNCTION__;

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
void Karia2::onCopySelectSegLog()
{
	qLogx()<<__FUNCTION__;
	
	QModelIndex idx;
	// QTreeView *tv;
	QItemSelectionModel *sm;	
	QString text;
	QModelIndexList mil;
	int cols = 0;

	if (this->mSegLogListView->model() != 0 )
	{
		cols = this->mSegLogListView->model()->columnCount();
		sm = this->mSegLogListView->selectionModel();
		mil = sm->selectedIndexes();
		for (int r = 0; r < mil.size()/cols; r++ )
		{
			for (int c = 0; c < cols; ++c )
			{
				text +=   mil.value(r+c*mil.size()/cols).data().toString();
				text += '\t';
			}
			text += "\r\n";
		}
		if (text.endsWith("\r\n"))
			text = text.left(text.length()-2);
		//
		QClipboard *cb = QApplication::clipboard();
		cb->setText(text);		
	}


	qLogx()<<cols<<text <<mil.size();

}
void Karia2::onSaveSegLog()
{
	QModelIndex idx;
	// QTreeView *tv;
	// QItemSelectionModel *sm;	
	QString text;
	QModelIndexList mil;
	QAbstractItemModel * model = 0;
	int cols = 0;

	model = this->mSegLogListView->model();
	if (model != 0 )
	{

		cols = model->columnCount();
		
		for (int r = 0; r < model->rowCount(); ++r)
		{
			for (int c = 0; c < cols; ++ c)
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
			fi.write(text.toLatin1());
			//fi.waitForBytesWritten();
			fi.close();
		}
	}
}
/**
 * 	清除线程日志视图中的LOG信息。
 */
void Karia2::onClearSegLog() 	//
{
	QAbstractItemModel * aim = 0;
	int row;

	aim = this->mSegLogListView->model();
	if (aim == 0 ) return;	//没有日志，直接返回
	row = aim->rowCount();
	for (int i = row - 1; i >=0; i --)
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
void Karia2::onSwitchLanguage(QAction* action)
{
	QString lang = "en_US";	//默认
	//if (action == this->mainUI->action_Chinese_simple)
	//{
	//	qLogx()<<"switch lang: zh_CN";
	//	lang = "zh_CN";
	//}

	//if (action == this->mainUI->action_English)
	//{
	//	qLogx()<<"switch lang: en_US";
	//	lang = "en_US";
	//}
	lang = action->data().toString();
	if (lang.isEmpty() )
		 lang = "en_US";	//默认
	
	//QTranslator translator;
	//translator.load(QString("karia2_")+lang);
	//qApp->installTranslator(&translator);

	QString langFile  = QString("karia2_")+lang;
	appTranslator.load(langFile , qmPath );
	//qLogx()<<"Loading file :"<<langFile;
	langFile = "qt_"+lang;
	qtTranslator.load(langFile,qmPath);
	//qLogx()<<"Loading file :"<<langFile;

	if (! action->isChecked()) {
		action->setChecked(true);
	}

	this->retranslateUi();
	
}

void Karia2::onSwitchSkinType(QAction* action)
{
    Q_UNUSED(action);
}

////////////style
//"windows", "motif", "cde", "plastique", "windowsxp", or "macintosh"
// ("Bespin", "Oxygen", "Windows", "Motif", "CDE", "Plastique", "GTK+", "Cleanlooks")

void Karia2::onSwitchWindowStyle(QAction * action )
{
	qLogx()<<__FUNCTION__<<":"<<__LINE__<<" typeL "<< action->data().toString()
		<< this->sender();
    // QStringList styleKeys = QStyleFactory::keys();
    // qLogx()<<styleKeys;

	if (action->data().toString() == "norwegianwood") {
		//qLogx()<<"NorwegianWood style";
        if (this->mNorStyle == NULL) {
            // this->mNorStyle = new NorwegianWoodStyle();
        }
		// QApplication::setStyle(this->mNorStyle);
	} else {
		QApplication::setStyle(action->data().toString());
	}

	if (mainUI->action_Default_Palette->isChecked()) {
		QApplication::setPalette(this->orginalPalette);
	} else {
		QApplication::setPalette(QApplication::style()->standardPalette());
	}
	if (! action->isChecked()) {
		action->setChecked(true);
	}
}

void Karia2::onSwitchSpeedMode(QAction *action)
{
	qLogx()<<__FUNCTION__;
    QString speedLimitType = "unlimited";
	if (action == mainUI->action_Unlimited) {
		this->mSpeedBarSlider->hide();
		this->mSpeedProgressBar->hide();
		this->mSpeedManualLabel->hide();
		// GlobalOption::instance()->mIsLimitSpeed = 0;
        this->onManualSpeedChanged(0);  // set no limit
	} else if (action == mainUI->action_Manual) {
		this->mSpeedBarSlider->show();
		this->mSpeedProgressBar->hide();
		this->mSpeedManualLabel->show();
		// GlobalOption::instance()->mIsLimitSpeed = 1;
        speedLimitType = "manual";
	} else if (action == mainUI->action_Automatic ) {
		this->mSpeedBarSlider->hide();
		this->mSpeedProgressBar->show();
		this->mSpeedManualLabel->show();
		// GlobalOption::instance()->mIsLimitSpeed = 0;
        this->onManualSpeedChanged(0); // no limit now 
        speedLimitType = "auto";
	}

    if (this->mainUI->action_Remember->isChecked()) {
        OptionManager::instance()->saveSpeedLimitType(speedLimitType);
    }
}

void Karia2::onRememberSpeedLimitSetting(bool checked)
{
    OptionManager *om = NULL;
    QString value;
    
    om = OptionManager::instance();
    value = om->getRememberSpeedLimit();
    if (checked && value == "false") {
        om->saveRememberSpeedLimit("true");

        if (this->mainUI->action_Unlimited->isChecked()) {
            om->saveSpeedLimitType("unlimited");
        } else if (this->mainUI->action_Manual->isChecked()) {
            om->saveSpeedLimitType("manual");
            om->saveSpeedLimitSpeed(QString::number(this->mSpeedBarSlider->value()));
        } else if (this->mainUI->action_Automatic->isChecked()) {
            om->saveSpeedLimitType("auto");
        }
    }

    if (!checked && value == "true") {
        om->saveRememberSpeedLimit("false");
    }
}

// TODO call many time when drap the speed slider bar
// need a accelerate slider
void Karia2::onManualSpeedChanged(int value) 
{
	//qLogx()<<__FUNCTION__;
	this->mSpeedManualLabel->setText(QString("%1 KB/s").arg(value));
	//this->mSpeedTotalLable->setText(QString("%1 KB/s").arg(value*this->mTaskQueue.size()));
	// GlobalOption::instance()->mMaxLimitSpeed = value * 1024;	//the value is KB in unit

    //this->initXmlRpc();

    QVariant payload;
    QVariantList args;
    QVariantMap options;
    options["max-overall-download-limit"] = QString("%1K").arg(value); // .arg(value * 1024);
    payload = QString("max-overall-download-limit");

    args.insert(0, options);

//    this->mAriaRpc->call("aria2.changeGlobalOption", args, payload,
//                         this, SLOT(onAriaChangeGlobalOptionResponse(QVariant &, QVariant &)),
//                         this, SLOT(onAriaChangeGlobalOptionFault(int, QString, QVariant &)));

    if (this->mainUI->action_Remember->isChecked()) {
        OptionManager::instance()->saveSpeedLimitSpeed(QString::number(value * 1000));
    }
}
////////////

void Karia2::showAboutDialog()
{
	AboutDialog *pAboutDialog = new AboutDialog(this);
	pAboutDialog->exec();
	delete pAboutDialog;
}

void Karia2::onGotoHomePage()
{
	QString homepage = "http://www.qtchina.net/";
	QDesktopServices::openUrl(homepage);
}

/**
 * 隐藏或者显示主窗口。
 * 这是一个翻转开关，是显示还是隐藏依赖于主窗口当前是否显示。
 */
void Karia2::onDropZoneDoubleClicked()
{
	qLogx()<<__FUNCTION__;
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
void Karia2::onDropZoneCustomMenu(const QPoint & pos)
{
    Q_UNUSED(pos);
	this->mDropZonePopupMenu->popup(QCursor::pos());
}


void Karia2::onOpenDistDirector()
{
	qLogx()<<__FUNCTION__;
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
		catId = mil.at(ng::tasks::user_cat_id).data().toString().toInt();

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

void Karia2::onOpenExecDownloadedFile()
{
	qLogx()<<__FUNCTION__;
	
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


void Karia2::onOpenRefererUrl()
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
	} else {
		QMessageBox::warning(this, tr("No Task Selected"), tr("Please Select a Task For Operation"));
	}
    
}

//////////private

//
void Karia2::onClipBoardDataChanged()
{
	
	QClipboard *cb = QApplication::clipboard();	
	QString text = cb->text();

	qLogx()<<cb->mimeData()->formats();
	
	if (text.length() > 0) {
		QUrl uu(text);
		if (uu.scheme().toUpper().compare("HTTP") == 0 
            || uu.scheme().toUpper().compare("HTTPS") == 0
            || uu.scheme().toUpper().compare("FTP") == 0
            || text.startsWith("magnet:?", Qt::CaseInsensitive)
            || text.startsWith("flashget", Qt::CaseInsensitive)
            || text.startsWith("qqdl:", Qt::CaseInsensitive)
            || text.startsWith("thunder:", Qt::CaseInsensitive)
            || text.startsWith("karia2://", Qt::CaseInsensitive)) {
			// this->showNewDownloadDialog(); // Open sometime later
		}
        if (text.startsWith("karia2://", Qt::CaseInsensitive)) {
            this->taskUi->showNewDownloadDialog(); // this should be karia2 passed from browser, force handle it
        }
	}
	
	qLogx()<<text;
}

void Karia2::paintEvent (QPaintEvent * event )
{
    Q_UNUSED(event);
	QPainter painter(this);
	QPoint p(0, 0);
	if (this->image.isNull()) {
		this->image =QImage("4422b46f9b7e5389963077_zVzR8Bc2kwIf.jpg");
		//qLogx()<<this->mCatView->windowOpacity();
		//this->mCatView->setWindowOpacity(0.5);
		//qLogx()<<this->mCatView->windowOpacity();
		this->mainUI->splitter_4->setWindowOpacity(0.3);
		this->mainUI->splitter_3->setWindowOpacity(0.5);
		this->mainUI->splitter_2->setWindowOpacity(0.8);
		this->mainUI->splitter->setWindowOpacity(0.6);
		//this->setWindowOpacity(0.7);
		// QPixmap pixmap("4422b46f9b7e5389963077_zVzR8Bc2kwIf.jpg");
		 //this->mCatView->setPixmap(pixmap);
	}
	//painter.drawImage(p, this->image );	
	//this->setWindowOpacity(0.6);	//ok
}
/**
 *
 * show user exit.
 * clean data, save state
 */
void Karia2::closeEvent (QCloseEvent * event )
{
    qLogx()<<this->sender();
	//qLogx()<< static_cast<QApplication*>(QApplication::instance())->quitOnLastWindowClosed ();
    if (this->sender() == 0) {
        // 点击右上角的X号, 或者Alt+F4，将该行为转成窗口最小化，隐藏到系统托盘区域
		this->mainUI->action_Show_Hide_Main_Widow->trigger();
		event->setAccepted(false);
    } else if (this->sender()->objectName() == "actionQuit") {
        event->setAccepted(true);	//不再传递了
        qApp->quit();
        return;        
	} else {//通过点击退出菜单，可认为用户是想退出的。
		//if (QMessageBox::question(this,"Are you sure?","Exit Karia2 Now.",QMessageBox::Ok,QMessageBox::Cancel) == QMessageBox::Cancel )
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

void Karia2::showEvent (QShowEvent * event ) 
{
	QWidget::showEvent(event);
    qLogx()<<__FUNCTION__<<__LINE__<<"first show";
	//qLogx()<<__FUNCTION__<<__LINE__<<rand();
	if (firstShowEvent == true) {	
		firstShowEvent = false;
		//添加首次显示要处理的工作
		qLogx()<<__FUNCTION__<<__LINE__<<"first show";
		//this->firstShowHandler();
		// QTimer::singleShot(30, this, SLOT(firstShowHandler()));
        this->mAtask = new AsyncTask(this);
        this->mAtask->start();
	}
}

/*
#if defined(Q_OS_WIN)
bool Karia2::winEvent (MSG * msg, long * result )
{
	//qLogx()<<__FUNCTION__<<__LINE__<<rand();
	//whereis MOD_CONTROL ???
    // shutcut: CTRL+SHIFT+C
	if (msg->message == WM_HOTKEY) {
		switch(msg->wParam) {
		case 'C':
			//做想做的事。我想用它抓图,哈哈，完全可以啊。
			this->shootScreen();
			qLogx()<<msg->message <<"xxx"<<(int)('G')<<":"<<msg->wParam<<" lp:"<< msg->lParam;
			break;
		default:
			break;
		}
	}
	
	return QMainWindow::winEvent(msg, result);
}
#elif defined(Q_OS_MAC)
bool Karia2::macEvent (EventHandlerCallRef caller, EventRef event )
{
    return QMainWindow::macEvent(caller, event);
}
#else
bool Karia2::x11Event (XEvent * event )
{
    //qLogx()<<"XEvent->type:"<< event->type;
    if (event->type == PropertyNotify) {
        //qLogx()<<"dont push button";
    }
    return QMainWindow::x11Event(event);
}
void Karia2::keyReleaseEvent (QKeyEvent * event )
{
    // shortcut: CTRL+G
    if (event->modifiers() == Qt::ControlModifier) {
        //qLogx()<<"with ctrl key ";
        if (event->key() == Qt::Key_G) {
            //qLogx()<<"key released: " << event->text();
            this->shootScreen();
        }
    }
    
    return QMainWindow::keyReleaseEvent(event);
}
#endif
*/
bool Karia2::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    return QMainWindow::nativeEvent(eventType, message, result);
}


void Karia2::shootScreen() 
{
	QPixmap screenSnapShot = QPixmap::grabWindow(QApplication::desktop()->winId());
	QString format = "png";

	QString fileName =	QString("abcd" ) + "."+format;
	
	if (! QFile::exists(fileName)) {
		QFile ff(fileName);
		ff.open(QIODevice::ReadWrite|QIODevice::Unbuffered);
		ff.close();
	}
	if (! screenSnapShot.save(fileName, format.toLatin1())) {
		qLogx()<< "save screen snap shot error:"<<fileName;
	}
}

//system tray slot handler
void Karia2::onActiveTrayIcon(QSystemTrayIcon::ActivationReason index )
{
	qLogx()<<__FUNCTION__ << index;
	if (index == QSystemTrayIcon::DoubleClick	)
	{		
		this->mainUI->action_Show_Hide_Main_Widow->trigger();
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
void Karia2::onBallonClicked()
{
	qLogx()<<__FUNCTION__;
}

void Karia2::onShowWalkSiteWindow()
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
	qLogx()<< this->mWalkSiteDockWidget->isHidden();
	if (this->mWalkSiteDockWidget->isHidden() )
		this->mWalkSiteDockWidget->showMinimized();

}

void Karia2::onTaskLogArrived(QString log)
{
    if (log.length() <= 2) {
        return;
    }
    // this->mainUI->plainTextEdit->appendPlainText(log.trimmed());
}

void Karia2::onTaskShowColumnsChanged(QString columns)
{
    //////
    QStringList colList;
    QString taskShowColumns = columns;
    // qLogx()<<__FUNCTION__<<taskShowColumns<<(ng::tasks::aria_gid);
    if (taskShowColumns != "") {
        colList = taskShowColumns.split(',');
        for (int i = ng::tasks::task_id ; i <= ng::tasks::aria_gid; ++i) {
            // qLogx()<<"show col:"<<colList.contains(QString::number(i));
            if (!colList.contains(QString::number(i))) {
                this->mainUI->mui_tv_task_list->setColumnHidden(i, true);
            } else {
                this->mainUI->mui_tv_task_list->setColumnHidden(i, false);
            }
        }
        if (this->mCustomTaskShowColumns != columns) {
            this->mCustomTaskShowColumns = columns;
        }
    }
}

void Karia2::onReselectFile(int row, bool selected)
{
    SeedFileModel *model = static_cast<SeedFileModel*>(this->mSeedFileView->model());
    QToolButton *btn = static_cast<QToolButton*>(this->mSeedFileView->cornerWidget());
    btn->setText("*");
    btn->setToolTip(tr("Click to apply reselected files"));
    QObject::connect(btn, SIGNAL(clicked()), this, SLOT(applyReselectFile()));
}

void Karia2::applyReselectFile()
{
    SeedFileModel *model = static_cast<SeedFileModel*>(this->mSeedFileView->model());
    QString reselectedIndexes = model->clearReselectFiles();
    q_debug()<<reselectedIndexes;
    QToolButton *btn = static_cast<QToolButton*>(this->mSeedFileView->cornerWidget());
    QObject::disconnect(btn, SIGNAL(clicked()), this, SLOT(applyReselectFile()));
    btn->setText("");
    btn->setToolTip(tr("No Change"));

    // now tell aria2 back  reselected files 
}

void Karia2::handleArguments()
{
    int argc = this->m_argc;
    char **argv = this->m_argv; // qApp->argv();

    this->handleArguments(argc, argv);
}

QString Karia2::showCommandLineArguments()
{
    QString cmdLine;

    cmdLine = 
        "Karia2 command line arguments:\n"
        "    -u --uri url\n"
        "    -r --refer url\n"
        "    -c --cookie cookie_str\n"
        "    -m --metafile file_path\n"
        "    -a --agent   host:port\n"
        "\n";
        ;

    qLogx()<<cmdLine;

    return cmdLine;
}

#include "getopt_pp_standalone.h"

void Karia2::handleArguments(int argc, char **argv)
{
    for (int i = 0 ; i < argc ; i ++) {
        qLogx()<<"Arg no: "<<i<<argv[i];
    }

    int rargc = argc;
    char **rargv = argv;
    char *targv[100] = {0};
    std::string noprefix_metafile;
    /*
      opera for win send this format arguments:
      no:  0 Z:\cross\karia2-svn\release\Karia2.exe
      no:  1 --uri http://down.sandai.net/Thunder5.9.19.1390.exe --refer http://dl.xunlei.com/index.htm?tag=1
      Karia2::handleArguments No uri specified.
    */
    // maybe opera
#if defined(Q_OS_WIN)
    if (argc == 2 && (argv[1][0] == argv[1][1] && argv[1][1] == '-')) {
        rargc = 0;
        rargv = targv;

        qLogx()<<"Reformat arguments for handle properly. "; // ktyytc11
        char *farg = argv[1];
        char *ptr = 0;

        rargc += 1;
        targv[0] = argv[0];

        while (*farg == ' ') { farg++; } ; // ltrim
        while (true && farg != NULL) {
            ptr = strchr(farg, ' ');
            qLogx()<<"here:"<<ptr;
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
            qLogx()<<"rArg no: "<<i<<rargv[i];
        }
    } else if (argc == 2) {
        // windows metafile arguments, no --metafile prefix
        GetOpt::GetOpt_pp args(rargc, rargv);
        args >> GetOpt::Option(GetOpt::GetOpt_pp::EMPTY_OPTION, noprefix_metafile);
    }
#elif defined(Q_OS_MAC)
    Q_UNUSED(targv);
#else
    Q_UNUSED(targv);
    // Fixed opera 10.54 for linux/unix change
    if (argc == 2) {
        // linux/unix metafile arguments, no --metafile prefix
        GetOpt::GetOpt_pp args(rargc, rargv);
        args >> GetOpt::Option(GetOpt::GetOpt_pp::EMPTY_OPTION, noprefix_metafile);
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

    if (noprefix_metafile.length() > 0) {
        std_metafile = noprefix_metafile;
    }

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
        if (metaInfo.count() != 5) {
            qLogx()<<__FUNCTION__<<"Not a valid metfile.";
            return;
        }
        uri = metaInfo.at(1);
        refer = metaInfo.at(2);
        cookies = metaInfo.at(3);
        agent = metaInfo.at(4);
    }

    if (uri.length() == 0) {
        qLogx()<<__FUNCTION__<<"No uri specified.";
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
    options.mAgent = agent;

    QString ngetUri = "karia2://" + options.toBase64Data();
    QClipboard *cb = QApplication::clipboard();
    cb->setText(ngetUri);
    qLogx()<<__FUNCTION__<<"uri:"<<uri<<"cbtext:"<<cb->text()<<ngetUri;

    // this->mainUI->action_New_Download->trigger();
    qApp->setActiveWindow(this);
    this->setFocus(Qt::MouseFocusReason);

}

void Karia2::handleArguments(QStringList args)
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
        argv[i] = strdup(args.at(i).toLatin1().data());
    }
    this->handleArguments(argc, argv);

    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    // 
    return;
}

void Karia2::onOtherKaria2MessageRecived(const QString &msg)
{
    QStringList args;
    
    if (msg.startsWith("sayhello:")) {
        qLogx()<<__FUNCTION__<<"You says: "<<msg;
    } else if (msg.startsWith("cmdline:")) {
        args = msg.right(msg.length() - 8).split(" ");
        if (!args.at(1).startsWith("--")) {
            QString arg2;
            for (int i = 1; i < args.count(); ++i) {
                arg2 += args.at(i) + " ";
            }
            args.erase(++args.begin(), args.end());
            args << arg2.trimmed();
            Q_ASSERT(args.count() == 2);
        }
        this->handleArguments(args);
    } else {
        qLogx()<<__FUNCTION__<<"Unknown message type: "<<msg;
    }
}


//dynamic language switch
void Karia2::retranslateUi()
{
	this->mainUI->retranslateUi(this);	//不调用它还不行，不知道为什么呢。
	//还有一个问题，怎么把程序中所有的字符串都放在这个函数中呢。
}

void Karia2::onObjectDestroyed(QObject *obj)
{
	qLogx()<<__FUNCTION__<<__LINE__<<" "<< obj->objectName();
	obj->dumpObjectInfo ();
	obj->dumpObjectTree ();
}

//void Karia2::onSkypeError(int errNo, QString msg)
//{
//    qLogx()<<errNo<<msg;
//}

//void Karia2::onShowSkypeTracer(bool checked)
//{
//    // if (this->mSkypeTracer == NULL) {
//    //     this->mSkypeTracer = new SkypeTracer(this);
//    //     QObject::connect(this->mSkype, SIGNAL(commandRequest(QString)),
//    //                      this->mSkypeTracer, SLOT(onCommandRequest(QString)));
//    //     QObject::connect(this->mSkype, SIGNAL(commandResponse(QString)),
//    //                      this->mSkypeTracer, SLOT(onCommandResponse(QString)));
//    //     QObject::connect(this->mSkypeTracer, SIGNAL(commandRequest(QString)),
//    //                      this->mSkype, SLOT(onCommandRequest(QString)));
//    // }

//    // this->mSkypeTracer->setVisible(!this->mSkypeTracer->isVisible());
//}

//void Karia2::onChatWithSkype()
//{
//    // QString skypeName = this->mainUI->lineEdit->text();
    
//    // QStringList contacts = this->mSkype->getContacts();
//    // qLogx()<<skypeName<<contacts;

//    // this->mSkype->newStream(skypeName);
//    // this->mSkype->newStream("drswinghead");
//}

//void Karia2::onSendPackage()
//{
//    MetaUri mu;
//    mu.url = "http://sourceforge.net/projects/nullfxp/files/nullfxp/nullfxp-2.0.2/nullfxp-2.0.2.tar.bz2/download";
//    mu.nameMd5 = "aceo732lksdf93sf32983rwe";
//    mu.contentMd5 = "is832rj9dvsdklfwwewfwf8sd2";
//    mu.fileSize = 12342432812LL;
//    mu.owner = "liuguangzhao";
//    mu.valid = true;
//    mu.mtime = 1234556789;

//    // QString muStr = mu.toString();
    
//    // SkypePackage sp;
//    // sp.seq = SkypeCommand::nextID().toInt();
//    // sp.type = SkypePackage::SPT_MU_ADD;
//    // sp.data = muStr;
    
//    // QString spStr = sp.toString();
    
//    // qLogx()<<muStr<<spStr;

//    // this->mSkype->sendPackage(this->mainUI->lineEdit->text(), spStr);
//}

//void Karia2::onCallSkype()
//{
//    // QString num = this->mainUI->lineEdit_2->text();
    
//    // SkypePackage sp;
//    // sp.seq = SkypeCommand::nextID().toInt();
//    // sp.type = SkypePackage::SPT_GW_SELECT;
//    // sp.data = num;

//    // QString spStr = sp.toString();
//    // qLogx()<<spStr;
//    // this->mSkype->sendPackage(this->mainUI->lineEdit->text(), spStr);
//}


//QAXFACTORY_DEFAULT(Karia2,
//	   "{074AA25F-F544-401E-8A2A-5C81F01264EF}",
//	   "{4351FA96-A922-4D76-B4AD-A0A4CF0ED8AA}",
//	   "{DBEF3F59-305C-4A58-9491-F7E56ADBB0B0}",
//	   "{9D6E015B-02EF-4FF6-A862-4B26250FCF57}",
//	   "{E0D9ECBF-2E40-4E94-A37B-0E4FB1ADBBB9}")


//////////end of karia2.cpp


