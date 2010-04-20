// nullget.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 22:19:59 +0800
// Version: $Id$
// 

#ifndef NULLGET_H
#define NULLGET_H

#include <QtCore>
#include <QtGui>
#include <QtGui/QMainWindow>
#include <QMessageBox>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QPalette>

#include "ui_nullget.h"
#include "aboutdialog.h"
#include "taskinfodlg.h"	// class TaskParameter 

#include "sqlitestorage.h"

#include "viewmodel.h"
#include "dropzone.h"

//

#include "walksitewnd.h"

class TaskQueue;
class InstantSpeedHistogramWnd;
// class MusicSearchWnd ;	//搜索窗口类。
class WalkSiteWndEx ;	//网站遍历窗口类。
class AriaMan;
class MaiaXmlRpcClient;

class NullGet : public QMainWindow
{
    Q_OBJECT;
public:
    NullGet(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~NullGet();

	void initialMainWindow();

public slots:
	void testFunc();
	void testFunc2();
	
	int createTask(TaskOption * option);
    int createTask(int taskId, TaskOption *option);
	int createTaskSegment(int pTaskId , QString purl , long fromPostion , long yourLength );

	void onSegmentListSelectChange( const QItemSelection & selected, const QItemSelection & deselected );
	void onTaskListSelectChange( const QItemSelection & selected, const QItemSelection & deselected );
	void onCatListSelectChange( const QItemSelection & selected, const QItemSelection & deselected );
	void onAllocateDiskFileSpace(quint64 fileLength , QString fileName );
	
	void onAddTaskList(QStringList list);	//下载一个列表中所有的URL任务

//public:

	//cat
	void onNewCategory();
	void onShowCategoryProperty();
	void onDeleteCategory();
	void onCategoryMoveTo();

	//edit
	void onEditSelectAll();
	void onEditInvertSelect();
	void onCopySelectSegLog();	//
	void onSaveSegLog();	//
	void onClearSegLog() ;	//

	//view
	void onShowToolbarText(bool show) ;

	//任务管理
	void onStartTask();		
	// void onStartTask(int pTaskId);

	void onStartTaskAll();	
	void onPauseTask() ;
	void onPauseTask(int pTaskId ) ;

	void onPauseTaskAll() ;
	void onStartSegment(int pTaskId,int pSegId);
	void onPauseSegment(int pTaskId,int pSegId);
	void onStartSegment();
	void onPauseSegment();

	void onDeleteTask();
	// void onDeleteTask(int pTaskId);
	void onDeleteTaskAll();
	void onDeleteSegment();
	void onDeleteSegment(int pTaskId,int pSegId);

	void onTaskDone(int pTaskId);	//

	//
	void onCopyUrlToClipboard();

	//help actions
	void onGotoHomePage();
	void onAboutQt(){ QMessageBox::aboutQt(this);	}

	//////////
	void onShowWalkSiteWindow();

    // arguments handler
    void handleArguments();
    void handleArguments(int argc, char **argv);
    void handleArguments(QStringList args);

    // single application message handler
    void onOtherKaria2MessageRecived(const QString &msg);

private:
    Ui::NullGet mainUI; 
	AboutDialog *mpAboutDialog;
	QTreeView * mTaskListView ;
	QTreeView * mSegListView;
	QTreeView * mSegLogListView;
	QTreeView * mCatView;
	//
	QAbstractItemModel * mTaskTreeViewModel;
	QAbstractItemModel * mCatViewModel;

	//swap buff
	char mSwapBuff[1024];

	ConfigDatabase * mConfigDatabase;
	SqliteStorage * mStorage ;

	///pop menu
	QMenu * mTaskPopupMenu;
	QMenu * mSegmentPopupMenu;
	QMenu * mLogPopupMenu ;
	QMenu * mCatPopupMenu ;
	QMenu * mDropZonePopupMenu;
	QMenu * mSysTrayMenu;

	InstantSpeedHistogramWnd *mISHW;
	DropZone * mDropZone ;
	QSystemTrayIcon * mSysTrayIcon;	//系统通知栏图标句柄

	QLabel *mStatusMessageLabel;
	QSlider *mSpeedBarSlider ;
	QProgressBar * mSpeedProgressBar ;
	QLabel * mSpeedManualLabel;
	QLabel * mSpeedTotalLable ;

	//non ui item
	QTimer mAverageSpeedTimer ;

	//搜索窗口实例
	WalkSiteWndEx * mHWalkSiteWndEx ;
	QDockWidget * mWalkSiteDockWidget ;


	//walksite window
	QMainWindow * mWalkSiteWnd ;


	//global vars for dynamic language switch
	QTranslator appTranslator ;
	QTranslator qtTranslator ;
	QString qmPath ;
	QString qmLocale ;
	//
	QPalette orginalPalette;

    TaskQueue *mTaskMan;
    AriaMan  *mAriaMan;
    QTimer mAriaUpdater;
    QTimer mAriaGlobalUpdater;
    QTimer mAriaTorrentUpdater;
    QHash<int, QString> mRunningMap; // <taskId, ariaGID> 
    QHash<int, QString> mTorrentMap; // <taskId, ariaGID>
    QHash<QTimer*, QVariant> mTorrentWaitRemoveConfirm;  // <timer*, payload>
    MaiaXmlRpcClient *mAriaRpc;

public slots:
	
	void onSwitchWindowStyle(QAction * action );

	void onSwitchSpeedMode(QAction *action);
	void onSwitchLanguage(QAction* action) ;
	void onSwitchSkinType(QAction* action ) ;

	
	void showAboutDialog();		//about dialog
	void showNewDownloadDialog();
    void showNewBittorrentFileDialog();
    void showNewMetalinkFileDialog();
	void showBatchDownloadDialog();	//添加批量下载对话框
	void showProcessWebPageInputDiglog();	//处理WEB页面，取其中链接并下载
	void onShowConnectOption();
	//void onShowDownloadRules();
	void onShowDefaultDownloadProperty();

	void onShowOptions();
	void onShowTaskProperty();
	void onShowTaskProperty(int pTaskId);
	void onShowTaskPropertyDigest(const QModelIndex & index );
	void onShowTaskPropertyDigest(  ) ;
	void onShowColumnEditor();	//列的显示情况管理
	void onTaskListMenuPopup( /*const QPoint & pos  = QPoint() */ ) ;
	void onUpdateJobMenuEnableProperty() ;
	void onLogListMenuPopup( const QPoint & pos ) ;
	void onSegListMenuPopup( const QPoint & pos ) ;
	void onCateMenuPopup( const QPoint & pos );
	//toolMenu

	void onDropZoneDoubleClicked();
	void onDropZoneCustomMenu(const QPoint & pos);

	//statusbar
	void onManualSpeedChanged(int value) ;

	void onOpenDistDirector();
	void onOpenExecDownloadedFile();

	//other
	void onClipBoardDataChanged();

	void caclAllTaskAverageSpeed();

	//system tray slot handler
	void onActiveTrayIcon(QSystemTrayIcon::ActivationReason index );
	void onBallonClicked();
	
	void shootScreen() ;
	void firstShowHandler();

	//object listener
	void onObjectDestroyed(QObject * object = 0);

private slots:
    void testResponse(QVariant &res, QVariant &payload);
    void testFault(int, QString, QVariant &payload);
    void onAriaAddUriResponse(QVariant &response, QVariant &payload);
    void onAriaAddUriFault(int, QString, QVariant &payload);
    void onAriaGetUriResponse(QVariant &response, QVariant &payload);
    void onAriaGetUriFault(int, QString, QVariant &payload);
    void onAriaGetStatusResponse(QVariant &response, QVariant &payload);
    void onAriaGetStatusFault(int, QString, QVariant &payload);

    void onAriaUpdaterTimeout();
    void onAriaGlobalUpdaterTimeout();

    void onAriaGetActiveResponse(QVariant &response, QVariant &payload);
    void onAriaGetActiveFault(int, QString, QVariant &payload);

    void onAriaGetServersResponse(QVariant &response, QVariant &payload);
    void onAriaGetServersFault(int code, QString reason, QVariant &payload);
    void onAriaRemoveResponse(QVariant &response, QVariant &payload);
    void onAriaRemoveFault(int, QString, QVariant &payload);
    void onAriaRemoveTorrentParseFileTaskResponse(QVariant &response, QVariant &payload);
    void onAriaRemoveTorrentParseFileTaskFault(int, QString, QVariant &payload);
    void onAriaRemoveGetTorrentFilesConfirmResponse(QVariant &response, QVariant &payload);
    void onAriaRemoveGetTorrentFilesConfirmFault(int code, QString reason, QVariant &payload);
    void onTorrentRemoveConfirmTimeout();

    void onAriaGetTorrentPeersResponse(QVariant &response, QVariant &payload);
    void onAriaGetTorrentPeersFault(int code, QString reason, QVariant &payload);
    void onAriaParseTorrentFileResponse(QVariant &response, QVariant &payload);
    void onAriaParseTorrentFileFault(int code, QString reason, QVariant &payload);
    void onAriaGetTorrentFilesResponse(QVariant &response, QVariant &payload);
    void onAriaGetTorrentFilesFault(int code, QString reason, QVariant &payload);
    void onAriaTorrentUpdaterTimeout();

    void onAriaGetVersionResponse(QVariant &response, QVariant &payload);
    void onAriaGetVersionFault(int code, QString reason, QVariant &payload);
    void onAriaGetSessionInfoResponse(QVariant &response, QVariant &payload);
    void onAriaGetSessionInfoFault(int code, QString reason, QVariant &payload);

    void onAriaMultiCallVersionSessionResponse(QVariant &response, QVariant &payload);
    void onAriaMultiCallVersionSessionFault(int code, QString reason, QVariant &payload);

    void onAriaChangeGlobalOptionResponse(QVariant &response, QVariant &payload);
    void onAriaChangeGlobalOptionFault(int code, QString reason, QVariant &payload);

    void onAriaGetGlobalOptionResponse(QVariant &response, QVariant &payload);
    void onAriaGetGlobalOptionFault(int code, QString reason, QVariant &payload);

private:	//method
	int getNextValidTaskId() ;

	void connectAllSignalAndSlog();

	void initPopupMenus();
	void initStatusBar();
	void initSystemTray();
    void initAppIcons();

	//overload
	void moveEvent ( QMoveEvent * event );

	//dynamic language switch
	void retranslateUi();

	QPoint mSwapPoint ;
	QModelIndex mSwapModelIndex ;

	//
	QPair<QString,QString> getFileTypeByFileName(QString fileName);

	//temporary 临时用于隐藏没有实现功能用户界面的代码。
	void hideUnimplementUiElement();
	void hideUnneededUiElement();

    void initXmlRpc();
    QMap<QString, QVariant> taskOptionToAria2RpcOption(TaskOption *to);

protected:
	virtual void paintEvent ( QPaintEvent * event ) ;
	virtual void closeEvent ( QCloseEvent * event );
	QImage image ;
	void showEvent ( QShowEvent * event ) ;
	bool firstShowEvent ;
	
	
#ifdef Q_OS_WIN32
	virtual bool winEvent ( MSG * message, long * result );
#else
    #ifdef Q_OS_MAC
    virtual bool	macEvent ( EventHandlerCallRef caller, EventRef event );
    #else
	virtual bool x11Event ( XEvent * event ) ;
    virtual void keyReleaseEvent ( QKeyEvent * event );
    #endif
#endif

signals:
	//提出状态改变要求的信号。是否需要呢。
		
};

#endif // NULLGET_H
