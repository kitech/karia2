// instantspeedhistogramwnd.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-08 21:39:55 +0800
// Version: $Id$
// 

#ifndef INSTANTSPEEDHISTOGRAMWND_H
#define INSTANTSPEEDHISTOGRAMWND_H
#include <QtCore>
#include <QtGui>

#include <QWidget>
#include <QImage>
#include <QList>
#include <QVector>
#include <QQueue>

class InstantSpeedHistogramWnd : public QWidget
{
	Q_OBJECT;
public:
    InstantSpeedHistogramWnd(QWidget *parent);
    ~InstantSpeedHistogramWnd();
protected:
	virtual void paintEvent(QPaintEvent * event);

private:
    QImage mHistogram;
	int mHistogramInteval;
	float mFullSpeed;
	int mMaxQueuedSpeed ;
	// QList<double> mSpeedQueue;
    QVector<float> mSpeedQueue;

	QWidget *mDrawCanvas;

public slots:
	void updateSpeedHistogram(double pSpeed);

};

#endif // INSTANTSPEEDHISTOGRAMWND_H
