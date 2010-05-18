// taskballmapwidget.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-04 21:14:24 +0800
// Version: $Id$
// 

#include <QVector>
#include <QImage>
#include <QList>
#include <QRadialGradient>
#include <QtGui>

#include "taskballmapwidget.h"

#include "taskqueue.h"

TaskBallMapWidget * TaskBallMapWidget::hWnd = 0 ;

TaskBallMapWidget::TaskBallMapWidget(QWidget *parent)
	: QWidget(parent) , mp(parent)
{
    this->mTaskMan = TaskQueue::instance();
	// mBlockSize = 50 * 1024 ;	// 50K
	this->mCurrentTaskId = -8888 ;
	this->setMinimumSize(parent->size().width() , parent->size().height());
	qDebug()<< parent->size() ;
	// this->mBallRadius = 10 ;
    this->mBallRadius = 7;

	this->mBallMap = QImage(parent->size().width(), parent->size().height(), 
                            QImage::Format_ARGB32_Premultiplied);	// 初始化一个和窗口一样大的图象。

	mSwithTimer.start(1000);
	QObject::connect(&this->mSwithTimer, SIGNAL(timeout()), this, SLOT(onSwitchState()));
}

TaskBallMapWidget::~TaskBallMapWidget()
{
//	if( TaskBallMapWidget::hWnd != 0 )
//		delete TaskBallMapWidget::hWnd ;
//	TaskBallMapWidget::hWnd = 0 ;
}

TaskBallMapWidget * TaskBallMapWidget::instance(QWidget * parent) 
{
	if (TaskBallMapWidget::hWnd == 0) {
		TaskBallMapWidget::hWnd = new TaskBallMapWidget(parent);
	}
	return TaskBallMapWidget::hWnd;
}

// void dumpBitArray(QBitArray ba)
// {
//     char buff[ba.size() + 1];
//     buff[0] = '\0';
//     for (int i = 0; i < ba.size(); i ++) {
//         if (ba.testBit(i)) {
//             buff[i] = '1';
//         } else {
//             buff[i] = '0';
//         }
//         buff[i+1] = '\0';
//     }
//     qDebug()<<ba.size()<<QString(buff);
// }

/**
 * 重绘
 * 实现只出现垂直滚动条的重载的画图事件处理方法。
 * 
 */
void TaskBallMapWidget::paintEvent(QPaintEvent * event ) 
{
	// qDebug()<< __FUNCTION__ <<this->mBallBit.size()<<this->mBallMap.size()<<this->mCurrentTaskId;
	QBrush brush(QColor(0, 0, 0));

	//依靠currtaskid 判断是否需要重绘
	if (this->mCurrentTaskId < 0) {
		return;
	}
    if (this->mBallBit.size() <= 0) {
        qDebug()<<__FUNCTION__<<QString("No bit set,");
        // leave the widget raw clear
        return;
    }

	//计算图象应该具有的宽度，splitter是以线性变化，而图形则必须以非线性变化，需要对取到的窗口宽度进行圆整
	int width = this->mp->size().width() - 20 ;
	if (width % (this->mBallRadius*2) != 0 ) {
		width = ( (width / (this->mBallRadius*2)) ) * ( this->mBallRadius * 2)  ;
	} else {
		width = (width / (this->mBallRadius*2)) * ( this->mBallRadius * 2)  ;
	}
	//窗口的高度。
	int height = this->mBallMap.height() ;

	if (width != this->mBallMap.width()) {
		//qDebug()<< "change to width:" << width ;
		//需要改变图象大小。宽度
		this->mBallMap = QImage(width , this->mBallMap.height() , QImage::Format_ARGB32_Premultiplied);
		//this->mBallMap = this->mBallMap.scaledToWidth( width ) ; 
		//this->resize(width,height);
		this->setFixedWidth( width );
	}

	if (this->mBallBit.size() != (width/(this->mBallRadius*2)) * (height/(this->mBallRadius*2))) {
		height = (this->mBallRadius*2) * (this->mBallBit.size())/((width/(this->mBallRadius*2)));
		if (height % (this->mBallRadius*2) != 0 ) {
			height = ( 1+ (height / (this->mBallRadius*2)) ) * ( this->mBallRadius * 2);
		} else {
			height = ( height / (this->mBallRadius*2) ) * ( this->mBallRadius * 2);
		}

		//qDebug()<< "change to height: " << height ;
		//需要改变图象大小。高度。
		//this->mBallMap = this->mBallMap.scaledToHeight( (this->mCurrentLength / (this->mBlockSize))/((width/(this->mBallRadius*2)) ) * ( this->mBallRadius*2 ));
		this->mBallMap =  QImage(width, height, QImage::Format_ARGB32_Premultiplied);
		//this->mBallMap =  this->mBallMap.scaledToHeight(height);

		//this->resize(width,height);
		this->setFixedHeight(height);	//设置本窗口构件的大小，使得外层的滚动构件能适当的添加或者删除滚动条。
	}
    //  ball ball 48 0 // 0 true 600 0 
    
    // qDebug()<<__FUNCTION__<<this->mBallBit.size()<<this->mBallMap.isNull()<<width<<height;
    if (this->mBallMap.isNull()) {
        /*
          if not check, show this error:
          QPainter::begin: Paint device returned engine == 0, type: 3
          QPainter::setRenderHint: Painter must be active to set rendering hints
          QPainter::setBrush: Painter not active
          QPainter::drawRects: Painter not active
          QPainter::setBrush: Painter not active
          QPainter::setBrush: Painter not actiev
         */
        // this->mBallMap = QImage(mp->width(), mp->height(), QImage::Format_ARGB32_Premultiplied);
        return;
    }

	QPainter imgp(&this->mBallMap) ;
	imgp.setRenderHint(QPainter::Antialiasing, true);
	imgp.setBrush(brush) ;
	imgp.drawRect(0, 0, this->mBallMap.width(), this->mBallMap.height());

	//更改前景颜色。
	brush.setColor(QColor(0,255,0));
	imgp.setBrush(brush);

	//////////
	QRadialGradient radgrad ;	//梯度法
	imgp.setBrush(radgrad) ;

	int row = 0 , col = 0 ;	//当前行，列
	int mrow , mcol ;		//最大行，列
	int srow , scol ;		//当前球块所在行，列。
	mcol = width / (this->mBallRadius*2) ;
	mrow = this->mBallBit.size() % mcol == 0 ? 
        (this->mBallBit.size() / mcol) : (this->mBallBit.size() / mcol + 1);
	int mtb = this->mBallBit.size();	//所有球的个数。

    // qDebug()<<"rows:"<<mrow<<"cols:"<<mcol;
    for (int row = 0; row < mrow; row++) {
        int sballno;
        
        for (int col=0; col < mcol ; col ++) {
            int bx, by, bw, bh;
            sballno = row * mcol + col;   //本球块在整个文件块中的序号。
            if (sballno >= this->mBallBit.size()) {
                break;
            }
            if (this->mBallBit.testBit(sballno) == false) {
                continue;
            }
            bx = col * this->mBallRadius * 2; //本求块的右上角X值。
            by = row * this->mBallRadius * 2; //本求块的右上角Y值。

			//qDebug()<<"x,y,w"<<bx<<by<<(this->mBallRadius*2);
			radgrad.setCenter(bx+this->mBallRadius, by + this->mBallRadius);	//梯度参数设置
			radgrad.setRadius(this->mBallRadius * 2 );
			radgrad.setFocalPoint(bx+this->mBallRadius/2,by+this->mBallRadius/2);
			radgrad.setColorAt(0.0,QColor(255,255,255));
			radgrad.setColorAt(0.5,QColor(0,255,0));
			radgrad.setColorAt(1.0,QColor(0,0,0));
			imgp.setBrush(radgrad);
			imgp.drawEllipse(bx,by,this->mBallRadius*2 , this->mBallRadius*2);
        }
    }

	//输出到屏幕。
	QPainter scrp(this);
	scrp.drawImage(0, 0, this->mBallMap);
}

void TaskBallMapWidget::resizeEvent(QResizeEvent * event)
{

}



//用于更新任务完成状态的槽
void TaskBallMapWidget::onRunTaskCompleteState(int taskId, bool pSwitch) 	//pSwitch 是否强制转换到这个任务图。
{
	// qDebug()<< __FUNCTION__ <<pSwitch;
	if (pSwitch) {
		this->mCurrentTaskId = taskId;
	}

	// if (pTask->mTaskId != this->mCurrentTaskId)	return; //不需要关注这个任务。

    this->onSwitchState();

	//画图。
    // this->mBallBit = this->mTaskMan->getCompletionBitArray(this->mCurrentTaskId);
    // qDebug()<<__FUNCTION__<<this->mCurrentTaskId<<this->mBallBit;
    // // dumpBitArray(this->mBallBit);

	// this->update();

	//qDebug()<< __FUNCTION__ <<" return";
}


void TaskBallMapWidget::onSwitchState() 	//timer 超时调用槽
{
    Q_ASSERT(this->mTaskMan != NULL);
    // if (this->mTaskMan != NULL) {
    QBitArray ba = this->mTaskMan->getCompletionBitArray(this->mCurrentTaskId);
    // dumpBitArray(ba);
    // qDebug()<<__FUNCTION__<<(ba == this->mBallBit);
    if (ba != this->mBallBit) {
        this->mBallBit = ba;
        this->update();
    }

    // this->repaint();
    // }
	//qDebug()<< __FUNCTION__ <<" return";
}

//设置每个球块所表示的文件块大小。
void TaskBallMapWidget::onSetBlockSize(int pSize) 
{
	//50K - 10M
	if (pSize < 50 * 1025 || pSize > 10 * 1024 * 1024) {
		return ;
	}
	// this->mBlockSize = pSize;
}

//设置每个球块的半径
void TaskBallMapWidget::onSetBallRadius(int pRadius)
{
	if (pRadius < 3 || pRadius > 20) return;
	this->mBallRadius = pRadius;
}

