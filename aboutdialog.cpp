// aboutdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-06 13:22:27 +0800
// Version: $Id: aboutdialog.cpp 198 2013-02-16 03:59:01Z drswinghead $
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

