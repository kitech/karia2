// aboutdialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-06 13:22:13 +0800
// Version: $Id: aboutdialog.h 122 2010-05-18 02:23:39Z drswinghead $
// 
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

#include "ui_aboutdialog.h"

class AboutDialog : public QDialog
{
    Q_OBJECT;
public:
    AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

private:
    Ui::AboutDialog ui;
};

#endif // ABOUTDIALOG_H
