// dropzone.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-19 23:00:46 +0800
// Version: $Id$
// 

#ifndef DROPZONE_H
#define DROPZONE_H

#include <QWidget>

#include <QMap> 
#include <QPair>
#include <QImage>
#include <QTimer>

class DropZone : public QWidget
{
	Q_OBJECT;
public:
	DropZone(QWidget *parent = 0,Qt::WFlags f = 0 ) ;
    ~DropZone();

public slots:
	//用于更新任务完成状态的槽
	void onRunTaskCompleteState(int pTaskId , int pCompletion , QString pFileName ) ;
	void onSwitchState() ;

protected:
	void paintEvent(QPaintEvent * event);
	virtual void mouseDoubleClickEvent ( QMouseEvent * event );
	virtual void mouseMoveEvent ( QMouseEvent * event );
	virtual void mousePressEvent ( QMouseEvent * event );
	virtual void mouseReleaseEvent ( QMouseEvent * event );
	virtual void dragEnterEvent(QDragEnterEvent *event) ;
	virtual void dropEvent(QDropEvent *event) ;

private:
	QWidget * mParentMainWindow ;

    double mOpque;
	QPoint mPostion;
	int mWidth;
	int mHeight;

	bool isLeftMousePressing ;

	QPoint mMousePressPos;

	QImage  mWndGround ;	//窗口背景图

	QMap<int , QPair<int , QString> >	mRunTaskCompleteState ;	//任务完成数数据结构
	QString mToolTip ;	//tooltip string字符串。

	QTimer mTaskStateSwithTimer ;	//显示任务定时开关。
	QString mStateText ; //	下一个定时开关开启时需要显示的文字。
	int		mSwithNo ;	//开关号，可能开关是多向的。
	int		mStateId ;	//下一个显示的ID

signals:
	void doubleclicked() ;

};

#endif // DROPZONE_H
