// aboutdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-06 13:22:27 +0800
// Version: $Id$
// 

#include <QtGui>

#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	//
	QObject::connect(ui.pushButton, SIGNAL(clicked()), this, SLOT(changeWindowSize()));
    this->ui.label_5->setText(QString(KARIA2_VERSION));
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::changeWindowSize()
{
	if (ui.pushButton->isChecked()) {
		this->showFullScreen();
		ui.pushButton->setText(tr("&Normal"));
	} else {
		this->showNormal();
		ui.pushButton->setText(tr("&FullScreen"));
	}
}
