// dmstatusbar.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-10-06 18:25:43 +0000
// Version: $Id$
// 

#include "ui_dmstatusbar.h"
#include "dmstatusbar.h"

DMStatusBar::DMStatusBar(QWidget * parent)
    : QStatusBar(parent)
    , uiw(new Ui::DMStatusBar)
{
    this->barwin = new QWidget();
    this->uiw->setupUi(this->barwin);

    this->addPermanentWidget(this->barwin);
    this->adjuestRealBar();

    // how signal to signal
    QObject::connect(this->uiw->horizontalSlider, &QSlider::valueChanged, this, &DMStatusBar::speedChanged);
    QObject::connect(this->uiw->horizontalSlider, &QSlider::valueChanged, this, &DMStatusBar::onSpeedChanged);
}

DMStatusBar::~DMStatusBar()
{
}

bool DMStatusBar::adjuestRealBar()
{
    // this->setStyleSheet(QStringLiteral("background-color: rgb(255, 255, 255);"));
    this->setStyleSheet(QStringLiteral("QStatusBar::item {border: none;}")); // ok 

    this->setContentsMargins(0, 0, 0, 0); //no
    /*
      this->barwin->setContentsMargins(0, 0, 0, 0);
    this->barwin->layout()->setSpacing(0);
    this->uiw->horizontalSpacer->changeSize(30, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

    this->barwin->layout()->update();
    */


    return true;
}

bool DMStatusBar::setSpeedValue(QString downloadSpeed, QString uploadSpeed)
{
    this->uiw->toolButton_3->setText(downloadSpeed);
    this->uiw->toolButton_2->setText(uploadSpeed);

    return true;
}

bool DMStatusBar::setSliderValue(int value)
{
    this->uiw->horizontalSlider->setValue(value);
    return true;
}

int DMStatusBar::speedValue()
{
    return this->uiw->horizontalSlider->value();
}

bool DMStatusBar::onSpeedChanged(int value)
{
    this->uiw->toolButton->setText(QString("%1 KB/s").arg(value));
    return true;
}

bool DMStatusBar::displaySpeedModeControl(QString mode)
{
    // 注意控制的显示顺序，防止一点界面的闪烁
	if (mode == "unlimited") {
        this->uiw->progressBar->hide();
        this->uiw->horizontalSlider->hide();
        this->uiw->toolButton->hide();
	} else if (mode == "manual") {
        this->uiw->progressBar->hide();
        this->uiw->toolButton->show();
        this->uiw->horizontalSlider->show();
	} else if (mode == "auto") {
        this->uiw->toolButton->show();
        this->uiw->horizontalSlider->hide();
        this->uiw->progressBar->show();
	}
    
    return true;
}



	//int begin = 1 , end = 500000;	//K	50M
	int begin = 1 , end = 500;	//K	500KB/s

    /*
	QStatusBar *bar = this->statusBar();

    //
	QLabel *lab = new QLabel("" );
	lab->setFixedWidth(0);
	lab->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
	//bar->addWidget(lab);
	bar->addPermanentWidget(lab);
	this->mStatusMessageLabel = lab;

	QProgressBar *pbar = new QProgressBar();	//进度条
	pbar->setFixedWidth(120);
	pbar->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	//bar->addWidget(pbar);
	bar->addPermanentWidget(pbar);
	pbar->setRange (begin , end );
	pbar->setValue(begin);
	pbar->setHidden(true);	//初始化为隐藏

	QSlider *hslider = new QSlider(Qt::Horizontal);	//滑动条
	hslider->setFixedWidth(120);
	hslider->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    hslider->setTracking(false); // avoid drap signal and app busy with socket community

	//bar->addWidget(hslider);
	bar->addPermanentWidget(hslider);
	this->mSpeedBarSlider = hslider;
	hslider->setRange(begin , end);
	hslider->setValue(begin);
	hslider->setHidden(false);	////初始化为隐藏

	lab = new QLabel(QString("%1 KB/s").arg(hslider->value()) );	//用户设置下载速度显示条
	lab->setFixedWidth(100);
	lab->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	//bar->addWidget(lab);
	bar->addPermanentWidget(lab);
	this->mSpeedManualLabel = lab;
	lab->setHidden(false);	//初始化为隐藏

	lab = new QLabel("DS: 0.00KB/s" );	//for total task speed，下载速度
    // lab->setPixmap(QPixmap("icons/crystalsvg/32x32/actions/go-down.png").scaled(16, 16));
	lab->setFixedWidth(100);
	//lab->setFixedHeight(28);
	lab->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	bar->addPermanentWidget(lab);
	this->mDownloadSpeedTotalLable = lab;
	lab->setHidden(false);	//初始化为隐藏/显示

	lab = new QLabel("US: 0.00KB/s" );	//for total task speed，上传速度
    // lab->setPixmap(QPixmap("icons/crystalsvg/32x32/actions/go-up.png").scaled(16, 16));
	lab->setFixedWidth(100);
	//lab->setFixedHeight(28);
	lab->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	bar->addPermanentWidget(lab);
	this->mUploadSpeedTotalLable = lab;
	lab->setHidden(false);	//初始化为隐藏/显示

    */
