// taskui.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-05 14:02:04 +0000
// Version: $Id$
// 
#ifndef _TASKUI_H_
#define _TASKUI_H_

#include "abstractui.h"


class TaskQueue;
class EAria2Man;
class TaskOption;

class TaskUi : public AbstractUi
{
    Q_OBJECT;
public:
    explicit TaskUi(Karia2 *pwin);
    virtual ~TaskUi();

    virtual bool init();

public slots:
    int createTask(TaskOption*option);
    int createTask(int taskId, TaskOption*option);

    void onTaskShowColumnsChanged(QString columns);

    void onSegmentListSelectChange(const QItemSelection & selected, const QItemSelection & deselected);
    void onTaskListSelectChange   (const QItemSelection & selected, const QItemSelection & deselected);
    void onCatListSelectChange    (const QItemSelection & selected, const QItemSelection & deselected);
	
    void onAddTaskList(QStringList list);	// add a list of tasks

	void showNewDownloadDialog();
    void showNewBittorrentFileDialog();
    void showNewMetalinkFileDialog();
	void showBatchDownloadDialog();	//添加批量下载对话框
	void showProcessWebPageInputDiglog();	//处理WEB页面，取其中链接并下载
	// void onShowConnectOption();
	//void onShowDownloadRules();
	// void onShowDefaultDownloadProperty(); ???

	void onShowTaskProperty();
	void onShowTaskProperty(int pTaskId);
	void onShowTaskPropertyDigest(const QModelIndex & index);
	void onShowTaskPropertyDigest( );
	void onTaskListMenuPopup( /*const QPoint & pos  = QPoint() */);
	void onUpdateJobMenuEnableProperty();
	void onLogListMenuPopup( const QPoint & pos);
	void onSegListMenuPopup( const QPoint & pos);
	void onCateMenuPopup( const QPoint & pos);

public:
    static QString decodeQQdlUrl(QString enUrl);
    static QString decodeThunderUrl(QString enUrl);
    static QString decodeFlashgetUrl(QString enUrl);
    static QString decodeEncodeUrl(QString enUrl);

private:
    int getNextValidTaskId();
	QPair<QString,QString> getFileTypeByFileName(QString fileName);

private:
	QTreeView *mTaskListView;
	QTreeView *mSegListView;
	QTreeView *mSegLogListView;
	QTreeView *mCatView;
    QTreeView *mSeedFileView;

	//
	QAbstractItemModel *mTaskTreeViewModel;
	QAbstractItemModel *mCatViewModel;
    // TaskItemDelegate *mTaskItemDelegate;
    // SeedFileItemDelegate *mSeedFileDelegate;

    QString mCustomTaskShowColumns;

	QPoint mSwapPoint;
	QModelIndex mSwapModelIndex;

	///pop menu
	QMenu *mTaskPopupMenu;
	QMenu *mLogPopupMenu;
	QMenu *mCatPopupMenu;

    // temporary
    TaskQueue *mTaskMan;
    EAria2Man *mEAria2Man;
    Ui::Karia2 *mainUI;
};

#endif /* _TASKUI_H_ */
