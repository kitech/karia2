// dropzone.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-19 23:00:53 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QApplication>
#include <QSizePolicy>
#include <QPainter>
#include <QDialog>
#include <QPaintEvent>
#include <QClipboard>
#include <QMap>
#include <QPair>
#include <QImage>

#include "dropzone.h"

#include "nullget.h"

//f|Qt::ToolTip|Qt::WindowStaysOnTopHint
DropZone::DropZone(QWidget *parent,Qt::WFlags f )
: QWidget(parent,Qt::ToolTip|Qt::WindowStaysOnTopHint ) , mParentMainWindow(parent)
{
	this->mOpque = 0.6;
	this->mPostion = QPoint(800,20);
	this->mHeight = 100 ;
	this->mWidth = 100 ;

	this->setWindowOpacity(this->mOpque);	
	this->move(this->mPostion);
	this->resize(this->mWidth,this->mHeight);
	this->mWndGround =  QImage(this->mWidth-6, this->mHeight-6,QImage::Format_RGB32);

	this->show();
	this->isLeftMousePressing = false ;
	this->setContextMenuPolicy(Qt::CustomContextMenu);

	this->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
	
	//enable drop 
	this->setAcceptDrops(true);

	this->mStateId = 0 ;

	this->mTaskStateSwithTimer.start(1000);
	QObject::connect( &this->mTaskStateSwithTimer,SIGNAL(timeout()),this,SLOT(onSwitchState()));
}

DropZone::~DropZone()
{

}

void DropZone::paintEvent(QPaintEvent * event)
{
	QPainter painter(this);
	//qDebug()<<event << this->mRunTaskCompleteState.count()<<this->mRunTaskCompleteState.size() ;

	QBrush brush(QColor(0,0,0));
	QPen pen(QColor(0,255,0));

	//painter.drawImage(2,2,QImage("cubelogo.png"));
	//painter.drawText(2,2,100,100,0,"slfkkkkkkkkkkk");

	QPainter imgPainter(&this->mWndGround);
	imgPainter.setBrush(brush);
	imgPainter.drawRect(2,2,this->mHeight-6,this->mWidth-6);
	imgPainter.setPen(pen);
	imgPainter.drawText(2,2,100,100,0, this->mStateText );

	painter.drawImage( 2 , 2 ,  (this->mWndGround) );

}

void DropZone::mouseDoubleClickEvent ( QMouseEvent * event )
{
	qDebug()<<__FUNCTION__;

	emit doubleclicked();
}
void DropZone::mouseMoveEvent ( QMouseEvent * event )
{
	if ( event->buttons() == Qt::LeftButton  )
	{
		//qDebug()<<event;
		QPoint cp = event->pos();
		QPoint wp = this->pos();
		QPoint ep = QPoint(wp.x()+(cp.x()-this->mMousePressPos.x()),wp.y()+(cp.y()-this->mMousePressPos.y()));
		this->move(ep);
	}
}
void DropZone::mousePressEvent ( QMouseEvent * event )
{
	this->isLeftMousePressing = true ;
	mMousePressPos = event->pos();
}
void DropZone::mouseReleaseEvent ( QMouseEvent * event )
{
	this->isLeftMousePressing = false ;
}

void DropZone::dragEnterEvent(QDragEnterEvent *event) 
{
	qDebug()<<event->mimeData()->formats()<<event->mimeData()->text() ;
	if( event->mimeData()->hasFormat("text/uri-list") || event->mimeData()->hasUrls() )
	{
		event->acceptProposedAction();		
	}
}

void DropZone::dropEvent(QDropEvent *event) 
{
	QString text = event->mimeData()->text();

	qDebug()<<__FUNCTION__<<text;
	NullGet * ng = static_cast<NullGet*>(this->mParentMainWindow);
	if( ng != 0 )
	{
		QApplication::clipboard()->setText(event->mimeData()->text());
		
	}
	//event->acceptProposedAction();
}

void DropZone::onRunTaskCompleteState(int pTaskId , int pCompletion , QString pFileName ) 
{
	QMap<int , QPair<int , QString> > swap ;

	QPair<int , QString > tm(pCompletion,pFileName) ;
	int taskId = pTaskId  ;
	if( this->mRunTaskCompleteState.contains(taskId) )
	{
		this->mRunTaskCompleteState[taskId] = tm ;
	}
	else
	{
		this->mRunTaskCompleteState[taskId] = tm ;
	}

	if( pCompletion == 100 )
	{
		this->mRunTaskCompleteState.erase(this->mRunTaskCompleteState.find(taskId) ) ;
	}

	this->mToolTip.clear();
	QMapIterator<int , QPair< int ,QString> > it(this->mRunTaskCompleteState);
	//更新tooltip 字符串。
	while( it.hasNext() )
	{
		it.next( ) ;
		this->mToolTip += QString("%1: %2% %3\n").arg(it.key()).arg(it.value().first).arg(it.value().second ) ;
	}
	//for( int i = 0 ; i < this->mRunTaskCompleteState.count() ; i ++ )
	{
		
	}
	if( this->mToolTip.endsWith("\n") ) this->mToolTip = this->mToolTip.left(this->mToolTip.length()-1);
	this->setToolTip(this->mToolTip);

}

void DropZone::onSwitchState() 
{
	int id = 0 ; 
	QList<int> keys ;
	if( this->mRunTaskCompleteState.count() == 0 )
	{
		mStateText = QString("no %1").arg(::rand()) ;
	}
	else
	{
		keys = this->mRunTaskCompleteState.keys();
		id = this->mStateId % ( keys.count()) ;
		mStateText = QString("%1% %2").arg( this->mRunTaskCompleteState[keys[id]].first).arg(this->mRunTaskCompleteState[keys[id]].second) ;
		if( this->mStateId ++  > 50000 ) this->mStateId = 0 ;
	}
	
	this->update();
}
	

