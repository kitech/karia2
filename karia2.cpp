// karia2.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 22:27:02 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

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
// #include "optiondlg.h"
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

#include "skype.h"
#include "skypetracer.h"

extern QHash<QString, QString> gMimeHash;

////////////////////////////////////////////////
//main window 
////////////////////////////////////////////////
Karia2::Karia2(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
    , mTaskMan(NULL)
    , mAriaMan(NULL), mAriaRpc(NULL)
    , mSkypeTracer(NULL)
{
    //    QDir().setCurrent(qApp->applicationDirPath()); // why do this?
	mainUI.setupUi(this);	
	firstShowEvent = true;
    this->mNorStyle = NULL;

	/////////////
	orginalPalette = QApplication::palette();
	//dynamic language switch
	qmLocale = QLocale::system().name();
	qmPath = qApp->applicationDirPath() + QString("/translations");
	qDebug()<<"Switch Langague to: "<<qmLocale;
	qApp->installTranslator(&appTranslator);
	qApp->installTranslator(&qtTranslator);
	appTranslator.load(QString("karia2_") + qmLocale , qmPath );
	qtTranslator.load(QString("qt_") + qmLocale , qmPath );
	this->retranslateUi();
	if (qmLocale.startsWith("zh_CN")) {
		this->mainUI.action_Chinese_simple->setChecked(true);
	} else if (qmLocale.startsWith("zh_TW")) {
		this->mainUI.action_Chinese_trad->setChecked(true);
	} else {
		this->mainUI.action_English->setChecked(true);
	}
	this->mDropZone = new DropZone();	// little float window
}
/**
 * first show adjust window layout
 */
void Karia2::firstShowHandler()
{
    QMenu *addTaskMenuList = new QMenu(this);
    addTaskMenuList->addAction(this->mainUI.action_New_Download);
    addTaskMenuList->addAction(this->mainUI.action_New_Bittorrent);
    addTaskMenuList->addAction(this->mainUI.action_New_Metalink);
    addTaskMenuList->addAction(this->mainUI.actionAdd_batch_download); // batch
    this->mAddOtherTaskButton = new QToolButton(this);
    this->mAddOtherTaskButton->setPopupMode(QToolButton::MenuButtonPopup); // new, nice
    this->mAddOtherTaskButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->mainUI.toolBar->insertWidget(this->mainUI.action_Delete_task, this->mAddOtherTaskButton);
    this->mAddOtherTaskButton->setMenu(addTaskMenuList);
    this->mAddOtherTaskButton->setDefaultAction(this->mainUI.action_New_Download);

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

	this->update();

	///////	
	// init base storage db 
    this->mStorage = SqliteStorage::instance(this);
	this->mStorage->open();

    //
	this->mCatView = this->mainUI.mui_tv_category;
	this->mCatView->setSelectionMode(QAbstractItemView::SingleSelection );
	this->mCatViewModel = SqliteCategoryModel::instance(0);
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
    // QObject::connect(this->mAriaMan, SIGNAL(taskLogReady(QString, QString, QString)),
    //                  this->mTaskMan, SLOT(onTaskLogArrived(QString, QString, QString)));
    QObject::connect(this->mAriaMan, SIGNAL(error(QProcess::ProcessError)),
                     this, SLOT(onAriaProcError(QProcess::ProcessError)));
    QObject::connect(this->mAriaMan, SIGNAL(finished(int, QProcess::ExitStatus)),
                     this, SLOT(onAriaProcFinished(int, QProcess::ExitStatus)));
    QObject::connect(this->mAriaMan, SIGNAL(taskLogReady(QString)),
                     this, SLOT(onTaskLogArrived(QString)));

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


    this->initUserOptionSetting();
    // process arguments 
    this->handleArguments();

    this->mSkype = new Skype("karia2");
    QObject::connect(this->mSkype, SIGNAL(skypeError(int, QString)),
                     this, SLOT(onSkypeError(int, QString)));
    this->mSkype->connectToSkype();
    QObject::connect(this->mainUI.pushButton, SIGNAL(clicked()),
                     this, SLOT(onChatWithSkype()));
    QObject::connect(this->mainUI.actionSkype_Tracer_2, SIGNAL(triggered(bool)),
                     this, SLOT(onShowSkypeTracer(bool)));

	//test area 　---------begin-----------------
	LabSpace *labspace = new LabSpace(this);
	//labspace->show();
    Q_UNUSED(labspace);

#ifdef Q_OS_WIN32
	// register system hot key 
	if (!::RegisterHotKey(this->winId(), 'C', MOD_CONTROL|MOD_SHIFT, 'C')) {
		qDebug()<<"::RegisterHotKey faild";
	}
#else	
#endif

	//test area 　---------end-----------------
}

Karia2::~Karia2()
{
    this->mAriaMan->stop();
    delete this->mAriaMan;
}
/**
 * init main window size, split size 
 */
void Karia2::initialMainWindow()
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

void Karia2::moveEvent (QMoveEvent * event )
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

void Karia2::initPopupMenus()
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
	this->mDropZonePopupMenu->addAction(this->mainUI.action_About_Karia2);
	this->mDropZonePopupMenu->addSeparator ();
	this->mDropZonePopupMenu->addAction(this->mainUI.actionQuit);

	//mSysTrayMenu
	this->mSysTrayMenu = new QMenu("NullTray", this);
	this->mSysTrayMenu->addAction(this->mainUI.action_Show_Hide_Main_Widow);
	this->mSysTrayMenu->addSeparator();
	this->mSysTrayMenu->addAction(this->mainUI.actionQuit);
	this->mSysTrayMenu->addAction(this->mainUI.actionAbout_Qt);

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
    // this->mainUI.actionDefault_Download_Properties->setIcon(QIcon(dir + "/configure.png"));
    this->mainUI.action_Options->setIcon(QIcon(dir + "/preferences-system.png"));
    this->mainUI.actionQuit->setIcon(QIcon(dir + "/system-shutdown.png"));
    this->mainUI.label->setPixmap(QPixmap(dir + "/status/unknown.png"));
}

void Karia2::connectAllSignalAndSlog()
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
	QObject::connect(this->mainUI.action_Show_Columns_Editor, SIGNAL(triggered()), this, SLOT(onShowColumnEditor()));
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
	
	// QObject::connect(this->mainUI.actionDefault_Download_Properties , SIGNAL(triggered()), this, SLOT(onShowDefaultDownloadProperty()));

	//statusbar
	QObject::connect(this->mSpeedBarSlider, SIGNAL(valueChanged(int)), this, SLOT(onManualSpeedChanged(int)));
    QObject::connect(this->mainUI.action_Remember, SIGNAL(triggered(bool)),
                     this, SLOT(onRememberSpeedLimitSetting(bool)));
    
	//help action 
	this->connect(this->mainUI.action_Go_to_Karia2_Home_Page, SIGNAL(triggered()), this, SLOT(onGotoHomePage()));
	this->connect(this->mainUI.action_About_Karia2, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
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
	
	//seg log menu
	QObject::connect(this->mainUI.action_Seg_Log_Copy, SIGNAL(triggered()), this, SLOT(onCopySelectSegLog()));
	QObject::connect(this->mainUI.action_Seg_Log_Save_To_File, SIGNAL(triggered()), this, SLOT(onSaveSegLog()));
	QObject::connect(this->mainUI.action_Clear_Seg_Log, SIGNAL(triggered()), this, SLOT(onClearSegLog()));

	//cat view
	QObject::connect(this->mCatView->selectionModel(), SIGNAL(selectionChanged (const QItemSelection &, const QItemSelection &)),
		this, SLOT(onCatListSelectChange(const QItemSelection & , const QItemSelection &   ) ) );

	//toolbar	//在UI设计器中将信号传递到标准菜单中。
	//QObject::connect(this->mainUI.mui_tb_properties, SIGNAL(triggered()), this, SLOT(onShowTaskProperty()));
	//QObject::connect(this->mainUI.mui_tb_open_dir, SIGNAL(triggered()), this, SLOT(onOpenDistDirector()));
	//QObject::connect(this->mainUI.mui_tb_exec_file, SIGNAL(triggered()), this, SLOT(onOpenExecDownloadedFile()));

	//other
	QObject::connect(this->mainUI.action_Copy_URL_To_ClipBoard, SIGNAL(triggered()), this, SLOT(onCopyUrlToClipboard()));

	QObject::connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(onClipBoardDataChanged()));

	QObject::connect(this->mainUI.actionWalk_Site, SIGNAL(triggered()), this, SLOT(onShowWalkSiteWindow()));
	QObject::connect(this->mSysTrayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                     this, SLOT(onActiveTrayIcon(QSystemTrayIcon::ActivationReason)));
	QObject::connect(this->mSysTrayIcon, SIGNAL(messageClicked()),  this, SLOT(onBallonClicked()));

	//test it
	//this->connect(this->mainUI.pushButton_3, SIGNAL(clicked()), this, SLOT(testFunc()));
	//QObject::connect(QApplication::instance(), SIGNAL(aboutToQuit () ), this, SLOT(testFunc2()));

}
//temporary 临时用于隐藏没有实现功能用户界面的代码。
void Karia2::hideUnimplementUiElement()
{
    this->mainUI.menu_Search->menuAction()->setVisible(false);
    this->mainUI.menu_Plugins->menuAction()->setVisible(false);

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
    this->mainUI.actionProcess_Web_Page_File->setVisible(false);

	//cat
	this->mainUI.action_cat_Move_To->setVisible(false);

	//edit
	this->mainUI.actionFind->setVisible(false);
	this->mainUI.actionFind_next->setVisible(false);

	//view
	this->mainUI.actionDetail->setVisible(false);
	this->mainUI.actionGrid->setVisible(false);
    this->mainUI.menuSkin->menuAction()->setVisible(false);
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
    this->mainUI.action_Site_Explorer->setVisible(false);
    this->mainUI.actionWalk_Site->setVisible(false);
	this->mainUI.action_Download_Rules->setVisible(false);
	// this->mainUI.action_Save_as_default->setVisible(false);

	//help
	this->mainUI.action_User_Manual_in_Internet->setVisible(false);

	this->mainUI.action_Manual_Karia2->setVisible(false);
	this->mainUI.actionC_heck_for_a_new_version->setVisible(false);
	this->mainUI.actionFAQ_in_Internet->setVisible(false);

	//other
	this->mainUI.action_Seg_List_Restart->setVisible(false);
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
    // qDebug()<<__FUNCTION__<<taskShowColumns<<(ng::tasks::aria_gid);
    if (taskShowColumns != "") {
        colList = taskShowColumns.split(',');
        for (int i = ng::tasks::task_id ; i <= ng::tasks::aria_gid; ++i) {
            // qDebug()<<"show col:"<<colList.contains(QString::number(i));
            if (!colList.contains(QString::number(i))) {
                this->mainUI.mui_tv_task_list->setColumnHidden(i, true);
            }
        }
    }

    //////
    QString rememberSpeedLimit = om->getRememberSpeedLimit();
    if (rememberSpeedLimit == "true") {
        this->mainUI.action_Remember->setChecked(true);
        QString speedLimitType = om->getSpeedLimitType();
        if (speedLimitType == "unlimited") {
        } else if (speedLimitType == "manual") {
            this->mainUI.action_Manual->setChecked(true);
            QString speedLImitSpeed = om->getSpeedLimitSpeed();
            this->onManualSpeedChanged(speedLImitSpeed.toInt()/1000);
            this->mSpeedBarSlider->setValue(speedLImitSpeed.toInt()/1000);
            this->onSwitchSpeedMode(this->mainUI.action_Manual);
        } else if (speedLimitType == "auto") {
            this->mainUI.action_Automatic->setChecked(true);
            this->onSwitchSpeedMode(this->mainUI.action_Automatic);
        }
    }
}

/**
 * access private
 */
int Karia2::getNextValidTaskId()
{
	int taskId = -1;

	SqliteStorage * storage = SqliteStorage::instance(this);
	storage->open();

	taskId = storage->getNextValidTaskID();

	return taskId;
}

/**
 * overload method
 */
int Karia2::createTask(TaskOption *option)
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

int Karia2::createTask(int taskId, TaskOption *option)
{
    this->mTaskMan->addTaskModel(taskId, option);
    return taskId;
}

void Karia2::onAddTaskList(QStringList list)
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
		for (int t = 0; t <list.size(); ++ t )
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

void Karia2::testFunc()
{
	qDebug()<<__FUNCTION__;
	// int count = 7;	
	this->mTaskPopupMenu->popup(QCursor::pos ());
	return;

	this->onStartTask();
	return;
	
	return;

	QString url = "http://localhost/mtv.wmv";
	int nRet = 0;
	qDebug() <<nRet;

}

void Karia2::testFunc2()
{
	qDebug()<<__FUNCTION__;

	return;

	// QStandardItemModel *model = 	mConfigDatabase->createCatModel();	
	// this->mCatView->setModel( model );
	// this->mCatView->expand(model->index(0,0));

	return;

	this->onPauseTask();

}


void Karia2::onSegmentListSelectChange(const QItemSelection & selected, const QItemSelection & deselected )
{
	qDebug()<<__FUNCTION__;
    Q_UNUSED(deselected);

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

void Karia2::onTaskListSelectChange(const QItemSelection & selected, const QItemSelection & deselected)
{
    Q_UNUSED(deselected);

	int taskId;
	// int segId;
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

        // calc how much charactors the label can show
        QFontInfo fontInfo = this->mainUI.label_11->fontInfo();
        int laWidth = this->mainUI.mui_tw_segment_graph_log->width();
        int fpSize = fontInfo.pixelSize();
        // int fppSize = fontInfo.pointSize();
        int chcnt = laWidth * 2/ fpSize;
        // qDebug()<<"calc chcnt:"<<laWidth<<fpSize<<chcnt<<fppSize<<refer.length();

        if (refer.length() > chcnt) {
            this->mainUI.label_11->setText(QString("<a href=\"%1\">%2</a>")
                                           .arg(refer, refer.left(chcnt) + "..."));
        } else {
            this->mainUI.label_11->setText(QString("<a href=\"%1\">%1</a>").arg(refer));
        }
        this->mainUI.label_11->setToolTip(QString(tr("Location: %1")).arg(refer));

        if (fileName.isEmpty()) {
            QString dir = qApp->applicationDirPath() + "/icons";
            this->mainUI.label->setPixmap(QPixmap(dir + "/status/unknown.png").scaled(32, 32));
        } else {
            this->mainUI.label->setPixmap(QPixmap(mimeIconPosition.second).scaled(32, 32));
        }
    }
}

void Karia2::onCatListSelectChange(const QItemSelection & selected, const QItemSelection & deselected )
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
void Karia2::onStartTask()
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

    this->initXmlRpc();

    for (int row = 0; row < smil.count() ; row += step) {
        TaskOption *taskOptions = TaskOption::fromModelRow(model, smil.at(row).row());
        taskId = smil.value(row + ng::tasks::task_id).data().toInt();
        url = smil.value(row + ng::tasks::org_url).data().toString();

        // if running
        if (this->mRunningMap.contains(taskId)) {
            continue;
        }

        QMap<QString, QVariant> payload;
        QVariantList args;
        QList<QVariant> uris;
        QMap<QString, QVariant> options;
        QString aria2RpcMethod;

        {
            // create task object first
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
}

void Karia2::onStartTaskAll()
{
	qDebug()<<__FUNCTION__;

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
	qDebug()<<__FUNCTION__;
	
	QAbstractItemView *view = this->mTaskListView;
	QAbstractItemModel *model = view->model(); // SqliteTaskModel::instance(ng::cats::downloading, this);
	int step = model->columnCount();
	int taskId = -1;
    QString ariaGid;
	QModelIndexList smil = view->selectionModel()->selectedIndexes();
	qDebug()<<__FUNCTION__<<"selectedIndexes count:"<<smil.size();

	if (smil.size() == 0) {
        return;
    }
    
    this->initXmlRpc();
    Q_ASSERT(this->mAriaRpc != NULL);

    for (int i = 0; i < smil.size(); i += step) {
        taskId = smil.value(i).data().toInt();
        ariaGid = smil.value(i + ng::tasks::aria_gid).data().toString();
        qDebug()<<__FUNCTION__<<smil.value(i)<<taskId; 

        // check if running
        if (!this->mRunningMap.contains(taskId)) {
            continue;
        }

        QVariantList args;

        args << ariaGid;
        this->mAriaRpc->call(QString("aria2.remove"), args, QVariant(taskId),
                             this, SLOT(onAriaRemoveResponse(QVariant &, QVariant&)),
                             this, SLOT(onAriaRemoveFault(int, QString, QVariant&)));
    }
}
void Karia2::onPauseTask(int pTaskId )
{
	qDebug()<<__FUNCTION__;
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
	qDebug()<<__FUNCTION__;
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
	qDebug()<<__FUNCTION__;
	
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
        // qDebug()<<"prepare delete ROW:"<<mrow<<" All ROW:"<<rowcnt;
		// int taskId = from_model->data(mil.value(row * colcnt + ng::tasks::task_id)).toInt();
		QString ariaGid = from_model->data(mil.value(row * colcnt + ng::tasks::aria_gid)).toString();
        // int srcCatId = from_model->data(mil.value(row * colcnt + ng::tasks::sys_cat_id)).toInt();

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
	qDebug()<<__FUNCTION__;
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

void Karia2::onTaskDone(int pTaskId)
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
}

void Karia2::onShutdown()
{
    qDebug()<<__FUNCTION__<<" shutdown OS now";
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
		//qDebug()<<dlg->getCatModel();
		//
		mil = ism->selectedIndexes();
		qDebug()<<__FUNCTION__<<mil.size();
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
		//qDebug()<<dlg->getCatModel();
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


void Karia2::initXmlRpc()
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

QMap<QString, QVariant> Karia2::taskOptionToAria2RpcOption(TaskOption *to)
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
        if (!to->mReferer.isEmpty()) {
          header << QString("Referer: %1").arg(to->mReferer);
        }
        if (!to->mAgent.isEmpty()) {
          header << QString("User-Agent: %1").arg(to->mAgent);
        }
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
 */
void Karia2::showNewDownloadDialog()
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

void Karia2::testResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}
void Karia2::testFault(int status, QString response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<status<<response<<payload;
}

void Karia2::onAriaAddUriResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload; // why this line cause crash?

    QMap<QString, QVariant> mPayload = payload.toMap();
    int taskId = mPayload["taskId"].toString().toInt();
    QString url = mPayload["url"].toString();
    QString cmd = mPayload["cmd"].toString();

    this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::aria_gid, response.toString());
    // for new bittorrent
    if (mPayload.contains("indexes")) {
        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::select_file, mPayload["indexes"].toString());
        this->mTaskMan->updateSelectFile(taskId, mPayload.value("indexes").toString());

        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::save_path, mPayload.value("savePath").toString());
        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::file_name, mPayload.value("saveName").toString());
        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::user_cat_id, mPayload.value("userCatId").toString());
    }

    this->mRunningMap[taskId] = response.toString();
    // if is torrent, add to torrentMap
    if (url.toLower().endsWith(".torrent")) {
        this->mTorrentMap[taskId] = response.toString();
     }
    
}
void Karia2::onAriaAddUriFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason;
    Q_UNUSED(payload);
}

void Karia2::onAriaGetUriResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response;
    Q_UNUSED(payload);
}
void Karia2::onAriaGetUriFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason;
    Q_UNUSED(payload);
}

/*
QVariant(QVariantMap, QMap(("bitfield", QVariant(QString, "0000") ) ("completedLength" ,  QVariant(QString, "1769472") ) ("connections" ,  QVariant(QString, "2") ) ("dir" ,  QVariant(QString, "/home/gzleo/karia2-svn") ) ("downloadSpeed" ,  QVariant(QString, "35243") ) ("files" ,  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("index", QVariant(QString, "1") ) ("length" ,  QVariant(QString, "13910775") ) ("path" ,  QVariant(QString, "/home/gzleo/karia2-svn/postgresql-9.0alpha5.tar.bz2") ) ("selected" ,  QVariant(QString, "true") ) ("uris" ,  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("status", QVariant(QString, "used") ) ("uri" ,  QVariant(QString, "http://wwwmaster.postgresql.org/redir/394/h/source/9.0alpha5/postgresql-9.0alpha5.tar.bz2") ) )  ) ,  QVariant(QVariantMap, QMap(("status", QVariant(QString, "used") ) ("uri" ,  QVariant(QString, "http://wwwmaster.postgresql.org/redir/394/h/source/9.0alpha5/postgresql-9.0alpha5.tar.bz2") ) )  ) )  ) ) )  ) )  ) ) ("gid" ,  QVariant(QString, "1") ) ("numPieces" ,  QVariant(QString, "14") ) ("pieceLength" ,  QVariant(QString, "1048576") ) ("status" ,  QVariant(QString, "active") ) ("totalLength" ,  QVariant(QString, "13910775") ) ("uploadLength" ,  QVariant(QString, "0") ) ("uploadSpeed" ,  QVariant(QString, "0") ) )  )
 */


void Karia2::onAriaGetStatusResponse(QVariant &response, QVariant &payload)
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
        this->mTaskMan->onPauseTask(taskId); // maybe named clearTask

        if (this->mRunningMap.count() == 0 
            && this->mainUI.action_Shut_Down_When_Done->isChecked()) {
            // shutdown now
        }
    }

    if (sts["status"].toString() == "error") {
        // 
        if (this->mRunningMap.contains(taskId)) {
            this->mRunningMap.remove(taskId);
        }
        if (this->mTorrentMap.contains(taskId)) {
            this->mTorrentMap.remove(taskId);
        }

        this->mTaskMan->onPauseTask(taskId); // maybe named clearTask
    }
}
void Karia2::onAriaGetStatusFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason;
    
    Q_UNUSED(payload);
}

// TODO, combine request to aria2.multicall
void Karia2::onAriaUpdaterTimeout()
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

void Karia2::onAriaRemoveResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
    int taskId = payload.toInt();
    this->mRunningMap.remove(taskId);
    if (this->mTorrentMap.contains(taskId)) {
        this->mTorrentMap.remove(taskId);
    }

    this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::task_status, QString(tr("pause")));
    this->mTaskMan->onPauseTask(taskId);
}

void Karia2::onAriaRemoveFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
    // assert(1 == 2);
}

void Karia2::onAriaGlobalUpdaterTimeout()
{
    QVariantList args;
    QVariant payload;

    if (this->mRunningMap.count() == 0) {
        // qDebug()<<"No Running task in queue, don't need run global update";
        return;
    }
    this->initXmlRpc();
    this->mAriaRpc->call(QString("aria2.tellActive"), args, payload,
                         this, SLOT(onAriaGetActiveResponse(QVariant&, QVariant&)),
                         this, SLOT(onAriaGetActiveFault(int, QString, QVariant &)));
}

void Karia2::onAriaGetActiveResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    Q_UNUSED(payload);

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

    // qDebug()<<"TSpeed:"<<speed<<" TLen:"<<totalLength<<" GLen:"<<gotLength;
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

void Karia2::onAriaGetActiveFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaGetServersResponse(QVariant &response, QVariant &payload)
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

void Karia2::onAriaGetServersFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaGetTorrentPeersResponse(QVariant &response, QVariant &payload)
{
    int taskId = payload.toInt();

    QVariantList peers = response.toList();
    this->mTaskMan->setPeers(taskId, peers);
    // qDebug()<<__FUNCTION__<<response<<payload;
}
void Karia2::onAriaGetTorrentPeersFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaParseTorrentFileResponse(QVariant &response, QVariant &payload)
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
    qDebug()<<__FUNCTION__<<url<<taskId;
        
    this->initXmlRpc();

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

void Karia2::onAriaParseTorrentFileFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
    // onAriaParseTorrentFileFault 1 "We encountered a problem while processing the option '--select-file'."
    /*
      2010-04-21 10:44:24.049116 ERROR - Exception caught
      Exception: [RequestGroup.cc:324] File /home/gzleo/karia2-svn/Kansas - Monolith [1979] exists, but a control file(*.aria2) does not exist. Download was canceled in order to prevent your file from being truncated to 0. If you are sure to download the file all over again, then delete it or add --allow-overwrite=true option and restart aria2.
     */
}

void Karia2::onAriaGetTorrentFilesResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    QMap<QString, QVariant> mPayload = payload.toMap();
    // int taskId = payload.toMap().value("taskId").toInt();
    QMap<QString, QVariant> statusMap = response.toMap();
    QVariantList files = statusMap["files"].toList(); // response.toList();

    SeedFilesDialog *fileDlg = new SeedFilesDialog();
    fileDlg->setFiles(files, true);
    fileDlg->setTorrentInfo(statusMap, statusMap.value("bittorrent").toMap());
    int rv = fileDlg->exec();
    
    if (rv == QDialog::Accepted) {
        // remove the unused aria2 task
        TaskOption *option = NULL;
        option = fileDlg->getOption();
        mPayload["indexes"] = fileDlg->getSelectedFileIndexes();
        mPayload["removeConfirm"] = "no";
        mPayload["savePath"] = option->mSavePath;
        mPayload["saveName"] = option->mSaveName;
        mPayload["userCatId"] = QString::number(option->mCatId);
        // mPayload["taskOption"] = option->toBase64Data();
        delete option; option = NULL;

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

void Karia2::onAriaGetTorrentFilesFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onTorrentRemoveConfirmTimeout()
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

void Karia2::onAriaRemoveGetTorrentFilesConfirmResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    QVariantMap msts = response.toMap();
    QVariantMap mPayload = payload.toMap();

    if (msts.value("status").toString() == "removed"
        || msts.value("status").toString() == "error") {
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

void Karia2::onAriaRemoveGetTorrentFilesConfirmFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaRemoveTorrentParseFileTaskResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    qDebug()<<__FUNCTION__<<payload;

    // insert new torrent task
    QMap<QString, QVariant> mPayload = payload.toMap();
    QString indexList = mPayload["indexes"].toString();
    QString url = mPayload["url"].toString();
    QString removeConfirm = mPayload["removeConfirm"].toString();
    // TaskOption toption = TaskOption::fromBase64Data(mPayload["taskOption"].toString());
    // toption.dump();
    QString savePath = mPayload["savePath"].toString();

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
    options["dir"] = savePath; // toption.mSavePath;
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

void Karia2::onAriaRemoveTorrentParseFileTaskFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaTorrentUpdaterTimeout()
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

void Karia2::onAriaGetVersionResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}
void Karia2::onAriaGetVersionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}
void Karia2::onAriaGetSessionInfoResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}
void Karia2::onAriaGetSessionInfoFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaMultiCallVersionSessionResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}

void Karia2::onAriaMultiCallVersionSessionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaChangeGlobalOptionResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    Q_UNUSED(response);

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

void Karia2::onAriaChangeGlobalOptionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaGetGlobalOptionResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}

void Karia2::onAriaGetGlobalOptionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

// TODO, test with a invalid torrent file
void Karia2::showNewBittorrentFileDialog()
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
        // payload["taskOption"] = to->toBase64Data();

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
        options["dir"] = QDir::tempPath();
        options["file-allocation"] = "none";

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

void Karia2::showNewMetalinkFileDialog()
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

void Karia2::showBatchDownloadDialog()
{
	QString url;
	int segcnt = 7;
	TaskOption * to = 0;	//创建任务属性。
	QStringList sl;
	
	BatchJobManDlg *bjd = new BatchJobManDlg(this);
	int er = bjd->exec();
	sl = bjd->getUrlList();

	delete bjd;

	if (er == QDialog::Accepted) {		
		qDebug()<<segcnt<<url;
		taskinfodlg *tid = new taskinfodlg(this);
		for (int i = 0; i < sl.count(); ++i) {
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

void Karia2::showProcessWebPageInputDiglog()	//处理WEB页面，取其中链接并下载
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
		for (int i = 0; i < ulcount; ++i)
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
			for (int i = 0; i < ulcount; ++i)
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
				for (int j = 0; j < linkCount; j ++ )
				{
					srcList.append(QString(linkList[j]));
				}
				html_parse_free_link_mt(linkList,linkCount);
			}	//end for (int i = 0; i < ulcount; ++i)
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
				for (int t = 0; t < resultList.size(); ++ t )
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

void Karia2::onShowTaskProperty()
{
	qDebug()<<__FUNCTION__;
	// QItemSelectionModel * sim;
	TaskQueue *tq = NULL;
	QModelIndexList mil;

	//在运行队列中查找。
	mil  = this->mTaskListView->selectionModel()->selectedIndexes();
	if (this->mTaskListView->model() != 0 &&
		mil.size() == this->mTaskListView->model()->columnCount()) {
		//only selected one ,可以处理，其他多选的不予处理。
		int taskId = mil.at(0).data().toInt();
        if (taskId <= 0) {
        }
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
        int segcnt = 5;
        QString fname;//= tq->mTaskOption->mSaveName;
		
        taskinfodlg *tid = new taskinfodlg(0, this);
        tid->setTaskUrl(url);
        tid->setSegmentCount(segcnt);
        tid->setRename(fname);
		
        int er = tid->exec();
        Q_UNUSED(er);
		
        delete tid;
    }
}
void Karia2::onShowTaskProperty(int pTaskId)
{
    Q_UNUSED(pTaskId);
}

void Karia2::onShowTaskPropertyDigest(const QModelIndex & index )
{
	//qDebug()<<__FUNCTION__ << index.data();
	// int taskId;
	this->mSwapPoint = QCursor::pos();
	this->mSwapModelIndex = index;
	QTimer::singleShot(1000, this, SLOT(onShowTaskPropertyDigest()));
}

void Karia2::onShowTaskPropertyDigest( )
{
	//qDebug()<<__FUNCTION__;
	QString tips = tr("<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:宋体; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\"><table  width=\"100%\"  height=\"100%\" border=\"1\">  <tr>    <td width=\"97\">&nbsp;<img name=\"\" src=\"%1\" width=\"80\" height=\"80\" alt=\"\"></td>    <td  height=\"100%\" ><b>%2</b><br>-------------------------------<br>File Size: %3<br>File Type: .%4<br>Completed: %5<br>-------------------------------<br>Save Postion: %6<br>URL: %7<br>Refferer: %8<br>Comment: %9<br>-------------------------------<br>Create Time: %10<br>------------------------------- </td>  </tr></table></body></html>");

	QPoint np = this->mainUI.mui_tv_task_list->viewport()->mapFromGlobal(QCursor::pos());
	QModelIndex nidx = this->mainUI.mui_tv_task_list->indexAt(np);
	QModelIndex tidx;
	// TaskQueue * tq = 0;
	// int catId = -1;
	
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

QPair<QString, QString> Karia2::getFileTypeByFileName(QString fileName)
{
	QPair<QString,QString> ftype("unknown", "unknown.png");

    // if (fileName.isEmpty()) {
    //     return ftype;
    // }

#if defined(Q_OS_WIN)
	QString path = "icons/crystalsvg/128x128/mimetypes/";
#elif defined(Q_OS_MAC)
    QString path = "icons/crystalsvg/128x128/mimetypes/";
#else // *nix
    QString path = QString("/usr/share/icons/%1/128x128/mimetypes/").arg(QIcon::themeName());
    if (!QDir().exists(path)) {
        path = QString("/usr/share/icons/gnome/32x32/mimetypes/gnome-mime-");
    }
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
}

void Karia2::onEditSelectAll()
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
void Karia2::onEditInvertSelect()
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

	qDebug()<<wg;

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

void Karia2::onTaskListMenuPopup(/* const QPoint & pos */) 
{
	//qDebug()<<__FUNCTION__;

	//将菜单项进行使合适的变灰
	QModelIndex idx;
	QString assumeCategory = "Download";	
	if (this->mCatView->selectionModel()->selectedIndexes().size() == 0 ) {
	} else if (this->mCatView->selectionModel()->selectedIndexes().size() > 0 ) {
		idx = this->mCatView->selectionModel()->selectedIndexes().at(0);
		if (idx.data().toString().compare("Download")!=0
			&& idx.data().toString().compare("Downloaded")!=0
			&& idx.data().toString().compare("Deleted")!=0	) {
			
		} else {
			assumeCategory = idx.data().toString();
		}
	}
	if (assumeCategory.compare("Downloaded") == 0 ) {
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
	} else if (assumeCategory.compare("Deleted") == 0 ) {
		this->mainUI.action_Start->setEnabled(false);
		this->mainUI.action_Pause->setEnabled(false);
		this->mainUI.action_Schedule->setEnabled(false);
		this->mainUI.action_Delete_task->setEnabled(false);
		if (this->mTaskListView->selectionModel()->selectedIndexes().size() > 0) {
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
	} else {//if (assumeCategory.compare("Download") == 0 )
		if (this->mTaskListView->selectionModel()->selectedIndexes().size() > 0) {
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
	
	if (this->mTaskListView == sender() ) {
		this->mTaskPopupMenu->popup(QCursor::pos());
	} else {
		//不是任务视图控件发送来的。
	}
}
void Karia2::onUpdateJobMenuEnableProperty() 
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

void Karia2::onLogListMenuPopup(const QPoint & pos ) 
{
    Q_UNUSED(pos);
	this->mLogPopupMenu->popup(QCursor::pos());
}
void Karia2::onSegListMenuPopup(const QPoint & pos) 
{
	// this->mSegmentPopupMenu->popup(QCursor::pos());
    Q_UNUSED(pos);
}
void Karia2::onCateMenuPopup(const QPoint & pos)
{
    Q_UNUSED(pos);
	this->mCatPopupMenu->popup(QCursor::pos());
}

void Karia2::onCopyUrlToClipboard()
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
void Karia2::onCopySelectSegLog()
{
	qDebug()<<__FUNCTION__;
	
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


	qDebug()<<cols<<text <<mil.size();

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
			fi.write(text.toAscii());
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
	//translator.load(QString("karia2_")+lang);
	//qApp->installTranslator(&translator);

	QString langFile  = QString("karia2_")+lang;
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

void Karia2::onSwitchSkinType(QAction* action)
{
    Q_UNUSED(action);
}

////////////style
//"windows", "motif", "cde", "plastique", "windowsxp", or "macintosh"
// ("Bespin", "Oxygen", "Windows", "Motif", "CDE", "Plastique", "GTK+", "Cleanlooks")

void Karia2::onSwitchWindowStyle(QAction * action )
{
	qDebug()<<__FUNCTION__<<":"<<__LINE__<<" typeL "<< action->data().toString()
		<< this->sender();
    // QStringList styleKeys = QStyleFactory::keys();
    // qDebug()<<styleKeys;

	if (action->data().toString() == "norwegianwood") {
		//qDebug()<<"NorwegianWood style";
        if (this->mNorStyle == NULL) {
            this->mNorStyle = new NorwegianWoodStyle();
        }
		QApplication::setStyle(this->mNorStyle);
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

void Karia2::onSwitchSpeedMode(QAction *action)
{
	qDebug()<<__FUNCTION__;
    QString speedLimitType = "unlimited";
	if (action == mainUI.action_Unlimited) {
		this->mSpeedBarSlider->hide();
		this->mSpeedProgressBar->hide();
		this->mSpeedManualLabel->hide();
		// GlobalOption::instance()->mIsLimitSpeed = 0;
        this->onManualSpeedChanged(0);  // set no limit
	} else if (action == mainUI.action_Manual) {
		this->mSpeedBarSlider->show();
		this->mSpeedProgressBar->hide();
		this->mSpeedManualLabel->show();
		// GlobalOption::instance()->mIsLimitSpeed = 1;
        speedLimitType = "manual";
	} else if (action == mainUI.action_Automatic ) {
		this->mSpeedBarSlider->hide();
		this->mSpeedProgressBar->show();
		this->mSpeedManualLabel->show();
		// GlobalOption::instance()->mIsLimitSpeed = 0;
        this->onManualSpeedChanged(0); // no limit now 
        speedLimitType = "auto";
	}

    if (this->mainUI.action_Remember->isChecked()) {
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

        if (this->mainUI.action_Unlimited->isChecked()) {
            om->saveSpeedLimitType("unlimited");
        } else if (this->mainUI.action_Manual->isChecked()) {
            om->saveSpeedLimitType("manual");
            om->saveSpeedLimitSpeed(QString::number(this->mSpeedBarSlider->value()));
        } else if (this->mainUI.action_Automatic->isChecked()) {
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
	//qDebug()<<__FUNCTION__;
	this->mSpeedManualLabel->setText(QString("%1 KB/s").arg(value));
	//this->mSpeedTotalLable->setText(QString("%1 KB/s").arg(value*this->mTaskQueue.size()));
	// GlobalOption::instance()->mMaxLimitSpeed = value * 1024;	//the value is KB in unit

    this->initXmlRpc();

    QVariant payload;
    QVariantList args;
    QVariantMap options;
    options["max-overall-download-limit"] = QString("%1K").arg(value); // .arg(value * 1024);
    payload = QString("max-overall-download-limit");

    args.insert(0, options);

    this->mAriaRpc->call("aria2.changeGlobalOption", args, payload,
                         this, SLOT(onAriaChangeGlobalOptionResponse(QVariant &, QVariant &)),
                         this, SLOT(onAriaChangeGlobalOptionFault(int, QString, QVariant &)));

    if (this->mainUI.action_Remember->isChecked()) {
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
void Karia2::onDropZoneCustomMenu(const QPoint & pos)
{
    Q_UNUSED(pos);
	this->mDropZonePopupMenu->popup(QCursor::pos());
}


void Karia2::onOpenDistDirector()
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
            || text.startsWith("karia2://", Qt::CaseInsensitive)) {
			// this->showNewDownloadDialog(); // Open sometime later
		}
        if (text.startsWith("karia2://", Qt::CaseInsensitive)) {
            this->showNewDownloadDialog(); // this should be karia2 passed from browser, force handle it
        }
	}
	
	qDebug()<<text;
}

void Karia2::paintEvent (QPaintEvent * event )
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
 * show user exit.
 * clean data, save state
 */
void Karia2::closeEvent (QCloseEvent * event )
{
	qDebug()<<__FUNCTION__ << this->sender();
	//qDebug()<< static_cast<QApplication*>(QApplication::instance())->quitOnLastWindowClosed ();
	if (this->sender() == 0) { // 点击右上角的X号,将该行为转成窗口最小化，隐藏到系统托盘区域
		this->mainUI.action_Show_Hide_Main_Widow->trigger();
		event->setAccepted(false);
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

	//qDebug()<<__FUNCTION__<<__LINE__<<rand();
	if (firstShowEvent == true) {	
		firstShowEvent = false;
		//添加首次显示要处理的工作
		qDebug()<<__FUNCTION__<<__LINE__<<"first show";
		//this->firstShowHandler();
		QTimer::singleShot(30, this, SLOT(firstShowHandler()));
	}
}

#if defined(Q_OS_WIN)
bool Karia2::winEvent (MSG * msg, long * result )
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
#elif defined(Q_OS_MAC)
bool Karia2::macEvent (EventHandlerCallRef caller, EventRef event )
{
    return QMainWindow::macEvent(caller, event);
}
#else
bool Karia2::x11Event (XEvent * event )
{
    //qDebug()<<"XEvent->type:"<< event->type;
    if (event->type == PropertyNotify) {
        //qDebug()<<"dont push button";
    }
    return QMainWindow::x11Event(event);
}
void Karia2::keyReleaseEvent (QKeyEvent * event )
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
	if (! screenSnapShot.save(fileName, format.toAscii())) {
		qDebug()<< "save screen snap shot error:"<<fileName;
	}
}

//system tray slot handler
void Karia2::onActiveTrayIcon(QSystemTrayIcon::ActivationReason index )
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
void Karia2::onBallonClicked()
{
	qDebug()<<__FUNCTION__;
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
	qDebug()<< this->mWalkSiteDockWidget->isHidden();
	if (this->mWalkSiteDockWidget->isHidden() )
		this->mWalkSiteDockWidget->showMinimized();

}

void Karia2::onAriaProcError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        this->mAriaGlobalUpdater.stop();
        QMessageBox::warning(this, tr("Aria2 backend error :"), 
                             tr("Can not start aria2. Are you already installed it properly?"));
    }
}

void Karia2::onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
}

void Karia2::onTaskLogArrived(QString log)
{
    if (log.length() <= 2) {
        return;
    }
    this->mainUI.plainTextEdit->appendPlainText(log.trimmed());
}

void Karia2::onTaskShowColumnsChanged(QString columns)
{
    //////
    QStringList colList;
    QString taskShowColumns = columns;
    // qDebug()<<__FUNCTION__<<taskShowColumns<<(ng::tasks::aria_gid);
    if (taskShowColumns != "") {
        colList = taskShowColumns.split(',');
        for (int i = ng::tasks::task_id ; i <= ng::tasks::aria_gid; ++i) {
            // qDebug()<<"show col:"<<colList.contains(QString::number(i));
            if (!colList.contains(QString::number(i))) {
                this->mainUI.mui_tv_task_list->setColumnHidden(i, true);
            } else {
                this->mainUI.mui_tv_task_list->setColumnHidden(i, false);
            }
        }
    }
}

void Karia2::handleArguments()
{
    int argc = qApp->argc();
    char **argv = qApp->argv();

    this->handleArguments(argc, argv);
}

#include "getopt_pp_standalone.h"

void Karia2::handleArguments(int argc, char **argv)
{
    for (int i = 0 ; i < argc ; i ++) {
        qDebug()<<"Arg no: "<<i<<argv[i];
    }

    int rargc = argc;
    char **rargv = argv;
    char *targv[100] = {0};
    std::string noprefix_metafile;
    /* opera for win send this format arguments:
       no:  0 Z:\cross\karia2-svn\release\Karia2.exe
       no:  1 --uri http://down.sandai.net/Thunder5.9.19.1390.exe --refer http://dl.xunlei.com/index.htm?tag=1
       Karia2::handleArguments No uri specified.
     */
    // maybe opera
#if defined(Q_OS_WIN)
    if (argc == 2 && (argv[1][0] == argv[1][1] && argv[1][1] == '-')) {
        rargc = 0;
        rargv = targv;

        qDebug()<<"Reformat arguments for handle properly. "; // ktyytc11
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
            qDebug()<<__FUNCTION__<<"Not a valid metfile.";
            return;
        }
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
    options.mAgent = agent;

    QString ngetUri = "karia2://" + options.toBase64Data();
    QClipboard *cb = QApplication::clipboard();
    cb->setText(ngetUri);
    qDebug()<<__FUNCTION__<<"uri:"<<uri<<"cbtext:"<<cb->text()<<ngetUri;

    // this->mainUI.action_New_Download->trigger();
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
        argv[i] = strdup(args.at(i).toAscii().data());
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
        qDebug()<<__FUNCTION__<<"You says: "<<msg;
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
        qDebug()<<__FUNCTION__<<"Unknown message type: "<<msg;
    }
}


//dynamic language switch
void Karia2::retranslateUi()
{
	this->mainUI.retranslateUi(this);	//不调用它还不行，不知道为什么呢。
	//还有一个问题，怎么把程序中所有的字符串都放在这个函数中呢。
}

void Karia2::onObjectDestroyed(QObject *obj)
{
	qDebug()<<__FUNCTION__<<__LINE__<<" "<< obj->objectName();
	obj->dumpObjectInfo ();
	obj->dumpObjectTree ();
}

void Karia2::onSkypeError(int errNo, QString msg)
{
    qDebug()<<errNo<<msg;
}

void Karia2::onChatWithSkype()
{
    QString skypeName = this->mainUI.lineEdit->text();
    
    QStringList contacts = this->mSkype->getContacts();
    qDebug()<<skypeName<<contacts;

    this->mSkype->newStream("drswinghead");
}

void Karia2::onShowSkypeTracer(bool checked)
{
    if (this->mSkypeTracer == NULL) {
        this->mSkypeTracer = new SkypeTracer(this);
        QObject::connect(this->mSkype, SIGNAL(commandRequest(QString)),
                         this->mSkypeTracer, SLOT(onCommandRequest(QString)));
        QObject::connect(this->mSkype, SIGNAL(commandResponse(QString)),
                         this->mSkypeTracer, SLOT(onCommandResponse(QString)));
        QObject::connect(this->mSkypeTracer, SIGNAL(commandRequest(QString)),
                         this->mSkype, SLOT(onCommandRequest(QString)));
    }

    this->mSkypeTracer->setVisible(!this->mSkypeTracer->isVisible());
}

//QAXFACTORY_DEFAULT(Karia2,
//	   "{074AA25F-F544-401E-8A2A-5C81F01264EF}",
//	   "{4351FA96-A922-4D76-B4AD-A0A4CF0ED8AA}",
//	   "{DBEF3F59-305C-4A58-9491-F7E56ADBB0B0}",
//	   "{9D6E015B-02EF-4FF6-A862-4B26250FCF57}",
//	   "{E0D9ECBF-2E40-4E94-A37B-0E4FB1ADBBB9}")


//////////end of karia2.cpp


