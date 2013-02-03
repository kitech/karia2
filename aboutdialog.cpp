// aboutdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-06 13:22:27 +0800
// Version: $Id: aboutdialog.cpp 113 2010-05-04 09:08:30Z drswinghead $
// 

#include <QtGui>

#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	//
    this->ui.label_5->setText(QString(KARIA2_VERSION));
}

AboutDialog::~AboutDialog()
{
}

