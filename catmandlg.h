// catmandlg.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-14 16:19:42 +0800
// Version: $Id$
// 

#ifndef CATMANDLG_H
#define CATMANDLG_H

#include <QtCore>
#include <QtGui>
#include <QtGui/QMainWindow>
#include <QtXml>
#include <QMessageBox>
#include <QTreeWidget>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QItemSelectionModel>
#include <QItemSelection>
#include <QTime>
#include <QDate>
#include <QDateTime>

#include <QDialog>
#include "ui_catmandlg.h"

#include "sqlitestorage.h"

class CatManDlg : public QDialog
{
    Q_OBJECT;
public:
    CatManDlg(QWidget *parent = 0);
    ~CatManDlg();
	QAbstractItemModel * getCatModel() { return this->mCatModel ; }
	QItemSelectionModel * getSelectionModel() ;
	QString getNewCatName() ;
	QString getNewCatDir();

public slots:	
	//void expandAll(QModelIndex  index = QModelIndex() );
	//void collapseAll();
	void changeMoveToState(QModelIndexList & indexList);

private:
    Ui::CatManDlgClass ui;

	QAbstractItemModel *mCatModel;

	int mState;

private slots:
	void onCategoryListSelectChange(const QItemSelection & selection, const QItemSelection & previou);
	void onModifyCategoryDefaultDirectory();
	//void onCreateNewCategory();
	void onCatNameChanged(QString name);

};

#endif // CATMANDLG_H
