// labspace.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-03 13:32:00 +0800
// Version: $Id: labspace.h 198 2013-02-16 03:59:01Z drswinghead $
// 

#ifndef LABSPACE_H
#define LABSPACE_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QPrintDialog>
#include <QWidget>
#include <QDialog>
#include <QtWidgets>
#include <QtPrintSupport>

#include "ui_labspace.h"

#ifdef WIN32
#include "DiskInfo.h"
#endif


class LabSpace : public QDialog
{
    Q_OBJECT

public:
    LabSpace(QWidget *parent = 0);
    ~LabSpace();

public slots:
	void onDirectoryChanged ( const QString & path );
	void onFileChanged ( const QString & path ) ;

	void getDiskRawData();

private:
	QFileSystemWatcher * mSysMon ;
private:
    Ui::LabSpaceClass ui;
};

#endif // LABSPACE_H
