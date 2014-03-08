// karia2.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-03 22:19:59 +0800
// Version: $Id: karia2.h 209 2013-10-06 12:05:58Z drswinghead $
// 

#ifndef KARIA2_H
#define KARIA2_H

// #include <X11/Xlib.h>

#include <QtCore>
#include <QtGui>
#include <QtWidgets>


#include "taskinfodlg.h"  // class TaskParameter

#include "sqlitestorage.h"
#include "walksitewnd.h"

namespace Ui {
    class Karia2;
};
class DropZone;
class TaskQueue;
class InstantSpeedHistogramWnd;
class WalkSiteWndEx;  //网站遍历窗口类。
//class AriaMan;
//class MaiaXmlRpcClient;
class SeedFileItemDelegate;
class TaskItemDelegate;
class OptionManager;
class DMStatusBar;

class AsyncTask;
class AbstractUi;
class OptionUi;

class EAria2Man;
class Karia2StatCalc;
class Aria2Manager;

// class Skype;
// class SkypeTracer;

class Karia2 : public QMainWindow
{
    Q_OBJECT;
public:
    Karia2(int argc, char **argv, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~Karia2();
    void initialMainWindow();
    void testFunc();
    void testFunc2();

public slots:
    // task ui
    int createTask(TaskOption*option);
    int createTask(int taskId, TaskOption*option);

    void onSegmentListSelectChange(const QItemSelection & selected, const QItemSelection & deselected);
    void onTaskListSelectChange   (const QItemSelection & selected, const QItemSelection & deselected);
    void onCatListSelectChange    (const QItemSelection & selected, const QItemSelection & deselected);
	
    void onAddTaskList(QStringList list);	// add a list of tasks

	void showNewDownloadDialog();
    void showNewBittorrentFileDialog();
    void showNewMetalinkFileDialog();
	void showBatchDownloadDialog();	//添加批量下载对话框
	void showProcessWebPageInputDiglog();	//处理WEB页面，取其中链接并下载
	void onShowTaskProperty();
	void onShowTaskProperty(int pTaskId);
	void onShowTaskPropertyDigest(const QModelIndex & index);
	void onShowTaskPropertyDigest( );
	void onTaskListMenuPopup( /*const QPoint & pos  = QPoint() */);
	void onUpdateJobMenuEnableProperty();
	void onLogListMenuPopup( const QPoint & pos);
	void onSegListMenuPopup( const QPoint & pos);
	void onCateMenuPopup( const QPoint & pos);

    // storage
    void onStorageOpened();

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
    void onClearSegLog();	//

    //view
    void onShowToolbarText(bool show);

	//任务管理
	void onStartTask();		
	// void onStartTask(int pTaskId);

	void onStartTaskAll();	
	void onPauseTask();
	void onPauseTask(int pTaskId);

	void onPauseTaskAll();

	void onDeleteTask();
	void onDeleteTaskAll();
	void onTaskDone(int pTaskId, int code);	//
    void onShutdown();

    void onLogAppended(const QString &path);
    

	//
	void onCopyUrlToClipboard();

	//help actions
	void onGotoHomePage();
	void onAboutQt(){ QMessageBox::aboutQt(this);	}

	//////////
	void onShowWalkSiteWindow();

    //
    int getNextValidTaskId();
	QPair<QString,QString> getFileTypeByFileName(QString fileName);

    static QString decodeQQdlUrl(QString enUrl);
    static QString decodeThunderUrl(QString enUrl);
    static QString decodeFlashgetUrl(QString enUrl);
    static QString decodeEncodeUrl(QString enUrl);

    // arguments handler
    void handleArguments();
    void handleArguments(int argc, char **argv);
    void handleArguments(QStringList args);

    // single application message handler
    void onOtherKaria2MessageRecived(const QString &msg);

protected:
    friend class AsyncTask;

private:
    Ui::Karia2 *mainUI;

    // ***Uis
	QTreeView *mTaskListView;
	QTreeView *mSegListView;
	QTreeView *mSegLogListView;
	QTreeView *mCatView;
    QTreeView *mSeedFileView;

	//
	QAbstractItemModel *mTaskTreeViewModel;
	QAbstractItemModel *mCatViewModel;
    TaskItemDelegate *mTaskItemDelegate;
    SeedFileItemDelegate *mSeedFileDelegate;

	// ConfigDatabase *mConfigDatabase;
	SqliteStorage *mStorage;

	///pop menu
	QMenu *mTaskPopupMenu;
	QMenu *mSegmentPopupMenu;
	QMenu *mLogPopupMenu;
	QMenu *mCatPopupMenu;
	QMenu *mDropZonePopupMenu;
	QMenu *mSysTrayMenu;

	QPoint mSwapPoint;
	QModelIndex mSwapModelIndex;

	InstantSpeedHistogramWnd *mISHW;
	DropZone *mDropZone;
	QSystemTrayIcon *mSysTrayIcon;	//system tray icon
    QToolButton *mAddOtherTaskButton;

    DMStatusBar *mStatusBar;

	//non ui item
	// QTimer mAverageSpeedTimer;

	//搜索窗口实例
	WalkSiteWndEx *mHWalkSiteWndEx;
	QDockWidget *mWalkSiteDockWidget;


	//walksite window
	QMainWindow *mWalkSiteWnd;

	//global vars for dynamic language switch
	QTranslator appTranslator;
	QTranslator qtTranslator;
	QString qmPath;
	QString qmLocale;
    AsyncTask *mAtask;
	//
	QPalette orginalPalette;
    
    OptionManager *mOptionMan;
    QString mCustomTaskShowColumns;

    TaskQueue *mTaskMan;
    // depcreated
//    AriaMan  *mAriaMan;
//    QTimer mAriaUpdater;
//    QTimer mAriaGlobalUpdater;
//    QTimer mAriaTorrentUpdater;
//    QHash<int, QString> mRunningMap; // <taskId, ariaGID>
//    QHash<int, QString> mTorrentMap; // <taskId, ariaGID>
//    QHash<QTimer*, QVariant> mTorrentWaitRemoveConfirm;  // <timer*, payload>
//    MaiaXmlRpcClient *mAriaRpc;

    //////// using embeded aria2c procedue
    //EAria2Man *mEAria2Man;
    QFile *mLogFile;
    QFileSystemWatcher *mLogWatcher;
    
    // all aria2 xmlrpc/json/websocket manager
    Aria2Manager *mAria2Manager;

    // Skype *mSkype;
    // SkypeTracer *mSkypeTracer;
                  
public slots:
	void onSwitchWindowStyle(QAction *action);

	void onSwitchSpeedMode(QAction *action);
    void onRememberSpeedLimitSetting(bool checked);

	void onSwitchLanguage(QAction*action);
	void onSwitchSkinType(QAction*action);
	
	void showAboutDialog();		//about dialog
	// void onShowConnectOption();
	//void onShowDownloadRules();
	// void onShowDefaultDownloadProperty();

	void onShowOptions();
	void onShowColumnEditor();	//列的显示情况管理
	//toolMenu
    // void onShutdownWhenDone(); // not need save in db
    // void onHungupWhenDone();  // the same 

	void onDropZoneDoubleClicked();
	void onDropZoneCustomMenu(const QPoint & pos);

	//statusbar
	void onManualSpeedChanged(int value);

	void onOpenDistDirector();
	void onOpenExecDownloadedFile();
    void onOpenRefererUrl();

	//other
	void onClipBoardDataChanged();

	//system tray slot handler
	void onActiveTrayIcon(QSystemTrayIcon::ActivationReason index);
	void onBallonClicked();
	
	void shootScreen();
	void firstShowHandler();
	void asyncFirstShowHandler();

    void onAriaProcError(QProcess::ProcessError error);
    void onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onTaskLogArrived(QString log); // log is from AriaMan
    void onTaskShowColumnsChanged(QString columns);

    void onReselectFile(int row, bool selected);
    void applyReselectFile();

	//object listener
	void onObjectDestroyed(QObject *object = 0);

    // skype related
    // void onShowSkypeTracer(bool checked);
    // void onSkypeError(int errNo, QString msg);
    // skype related test
    // void onChatWithSkype();
    // void onSendPackage();
    // void onCallSkype();

public slots: // embeded aria2c related
    void onAria2GlobalStatChanged(QMap<int, QVariant> stats);

private slots:
    // aria2rpc related
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

    void onAriaTorrentReselectFileMachineResponse(QVariant &response, QVariant &payload);
    void onAriaTorrentReselectFileMachineFault(int code, QString reason, QVariant &payload);

private:	//method

	void connectAllSignalAndSlog();

	void initPopupMenus();
	void initStatusBar();
	void initSystemTray();
    void initAppIcons();
    void initUserOptionSetting();

	//overload
	void moveEvent(QMoveEvent *event);

	//dynamic language switch
	void retranslateUi();

	//temporary method, hide not impled function
	void hideUnimplementUiElement();
	void hideUnneededUiElement();

//    void initXmlRpc();
//    QMap<QString, QVariant> taskOptionToAria2RpcOption(TaskOption *to);

    QString showCommandLineArguments();
    int m_argc;
    char **m_argv;

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void closeEvent(QCloseEvent *event);
	QImage image;
	void showEvent(QShowEvent *event);
	bool firstShowEvent;
	
    /*	
#if defined(Q_OS_WIN32)
	virtual bool winEvent(MSG *message, long *result);
#elif defined(Q_OS_MAC)
    virtual bool macEvent(EventHandlerCallRef caller, EventRef event);
#else
	virtual bool x11Event(XEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
#endif
    */
    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result);

};
#endif // KARIA2_H
