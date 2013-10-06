// dmstatusbar.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-10-06 18:25:48 +0000
// Version: $Id$
// 
#ifndef _DMSTATUSBAR_H_
#define _DMSTATUSBAR_H_

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

namespace Ui {
    class DMStatusBar;
}

class DMStatusBar : public QStatusBar
{
    Q_OBJECT;
public:
    explicit DMStatusBar(QWidget * parent = 0);
    virtual ~DMStatusBar();
  
    bool setSpeedValue(QString downloadSpeed, QString uploadSpeed);

    bool setSliderValue(int value);
    int speedValue();

    bool displaySpeedModeControl(QString mode);

signals:
    bool speedChanged(int value);

private slots:
    bool onSpeedChanged(int value);

private:
    bool adjuestRealBar();

private:
    Ui::DMStatusBar *uiw;
    QWidget *barwin;

    //
	QLabel *mStatusMessageLabel;
	QSlider *mSpeedBarSlider;
	QProgressBar *mSpeedProgressBar;
	QLabel *mSpeedManualLabel;
	QLabel *mDownloadSpeedTotalLable;
	QLabel *mUploadSpeedTotalLable;    
};

#endif /* _DMSTATUSBAR_H_ */
