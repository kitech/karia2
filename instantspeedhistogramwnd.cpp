// instantspeedhistogramwnd.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-08 21:40:03 +0800
// Version: $Id$
// 

#include "instantspeedhistogramwnd.h"

InstantSpeedHistogramWnd::InstantSpeedHistogramWnd(QWidget *parent)
	: QWidget(parent)
{	
	this->setMinimumWidth( parent->rect().width()<120 ? 120 : parent->rect().width() );
	//this->setMinimumWidth(parent->rect().width());
	this->setMinimumHeight( parent->rect().height()<32 ? 32 : parent->rect().height() );
	//this->setMinimumHeight(parent->rect().height());
	qDebug()<<parent->rect()<<this->rect() ;
	this->mHistogram = QImage(this->rect().width(), this->rect().height(), QImage::Format_ARGB32_Premultiplied);

	this->mFullSpeed = 100.0;
	this->mHistogramInteval = 2;
	this->mMaxQueuedSpeed = this->rect().width()/this->mHistogramInteval;

	// for (int n = 0 ; n < this->mMaxQueuedSpeed ; n ++) {
	// 	this->mSpeedQueue.append(0.0);
	// }
    this->mSpeedQueue.resize(this->mMaxQueuedSpeed);
    this->mSpeedQueue.fill(this->mMaxQueuedSpeed, 0.0);

    this->updateSpeedHistogram(0.0);
}

InstantSpeedHistogramWnd::~InstantSpeedHistogramWnd()
{

}

void InstantSpeedHistogramWnd::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
	QPainter painter(this);
	
	//painter.drawText(0,0,35,35,0,"hehehe");
	painter.drawImage(0, 0, this->mHistogram);

}

void InstantSpeedHistogramWnd::updateSpeedHistogram(double pSpeed)
{
	QRect rect;
	int xpos , ypos , heigth;
	int qc;
	QBrush brush(Qt::Dense1Pattern);
	brush.setColor(QColor(0,255,0));
	QBrush nullbrush(Qt::SolidPattern);
	nullbrush.setColor(QColor(0,0,0));

	if (pSpeed < 0) pSpeed = 0;
	QPainter painter(&this->mHistogram);
	this->mSpeedQueue.append(pSpeed);
	if (this->mSpeedQueue.count() > this->mMaxQueuedSpeed) {
		this->mSpeedQueue.remove(0);
	}
	painter.fillRect(0, 0, this->rect().width(), this->rect().height(), nullbrush);

	qc = this->mSpeedQueue.count();
	for (int n = 0 ; n < qc ; n ++ ) {
		xpos = this->rect().width() - ( qc - n  ) * this->mHistogramInteval;
		heigth = this->rect().height() * this->mSpeedQueue.at(n) / this->mFullSpeed;
		ypos = this->rect().height() -  heigth;
		if (ypos < 0) {
            ypos = 0; 
        }

		//36 / x = 200 / pSeed ;
		rect.setX(xpos);
		rect.setY(ypos);
		rect.setSize(QSize(this->mHistogramInteval, heigth));
		painter.fillRect(rect, brush);

		//qDebug()<< xpos << ypos << heigth ;
	}

	this->repaint();
}

