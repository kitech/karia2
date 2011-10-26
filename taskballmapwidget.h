// taskballmapwidget.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-04 21:11:16 +0800
// Version: $Id$
// 

#ifndef TASKBALLMAPWIDGET_H
#define TASKBALLMAPWIDGET_H

#include <QtCore>
#include <QWidget>
#include <QImage>
#include <QMap>
#include <QPair>
#include <QList>
#include <QTimer>

class TaskQueue;

/**
 * 任务完成状态的球状图形显示构件类
 * 实例模式
 */
class TaskBallMapWidget : public QWidget
{
	Q_OBJECT;
public:
	static TaskBallMapWidget *instance(QWidget * parent = 0);
    virtual ~TaskBallMapWidget();

public slots:
	//用于更新任务完成状态的槽
	void onRunTaskCompleteState(int taskId, bool pSwitch = false);	//pSwitch 是否强制转换到这个任务图。

	void onSwitchState();	//timer 超时调用槽
	//void onSwitchTask( TaskQueue * pTask ) ;	//转换到画某一TASK的槽
	//设置每个球块所表示的文件块大小。
	void onSetBlockSize(int pSize) ;
	//设置每个球块的半径
	void onSetBallRadius(int pRadius);

protected:
	virtual void paintEvent ( QPaintEvent * event ) ;
	virtual void resizeEvent ( QResizeEvent * event );

private:
    TaskBallMapWidget( QWidget *parent = 0 );    
	static TaskBallMapWidget *hWnd;
	QTimer	mSwithTimer ;	//显示任务定时开关。

	int mBallRadius;	//
	QImage	mBallMap;	//当前正在显示的图片对象。
	int mCurrentTaskId;	//当前正在显示的taskid 
    TaskQueue *mTaskMan;

    QBitArray mBallBit;

	QWidget *mp;
};

#endif // TASKBALLMAPWIDGET_H
