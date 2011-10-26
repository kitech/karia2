// catmandlg.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-14 16:19:49 +0800
// Version: $Id$
// 

#include <QtGui>
#include <QDir>
#include <QFileInfo>

// #include "viewmodel.h"
#include "catmandlg.h"

#include "sqlitestorage.h"
#include "sqlitecategorymodel.h"

CatManDlg::CatManDlg(QWidget *parent)
    : QDialog(parent)
{
	this->uiwin.setupUi(this);

	this->mState = 0 ;	//create
	//this->mCatModel  = CategoryModel::instance();
	this->mCatModel = SqliteCategoryModel::instance(0);

	this->uiwin.cmd_tv_cat_tree->setModel(this->mCatModel);
    this->uiwin.cmd_tv_cat_tree->setRootIndex(this->mCatModel->index(0, 0));
	
	//this->expandAll();
	this->uiwin.cmd_tv_cat_tree->expandAll();
	this->uiwin.cmd_tv_cat_tree->resizeColumnToContents(0);

	//this->uiwin.cmd_tv_cat_tree->expand(this->mCatModel->index(0,0));
	//this->uiwin.cmd_tv_cat_tree->expand(this->mCatModel->index(1,0,this->mCatModel->index(0,0)) );
		
	QObject::connect(this->uiwin.cmd_tv_cat_tree->selectionModel(),
                     SIGNAL(selectionChanged ( const QItemSelection & , const QItemSelection &)),
                     this, SLOT(onCategoryListSelectChange(const QItemSelection &, const QItemSelection &)));
	QObject::connect(this->uiwin.cmd_tb_change_director, SIGNAL(clicked()), this, SLOT(onModifyCategoryDefaultDirectory()));
	//QObject::connect(this->uiwin.okButton,SIGNAL(clicked()),this,SLOT(onCreateNewCategory()));
	QObject::connect(this->uiwin.cmd_le_cat_name, SIGNAL(textChanged(QString )), this, SLOT(onCatNameChanged(QString)));
	
	//on windows
#if defined(Q_OS_WIN)
	this->uiwin.cmd_le_default_directory->setText("C:/NGDownloads/");
#else
	//on unix maybe
	//this->uiwin.cmd_le_default_directory->setText("~/.nullget/NGDownloads/");
    this->uiwin.cmd_le_default_directory->setText("~/NGDownloads/");
#endif
}

CatManDlg::~CatManDlg()
{
	//delete this->mCatModel ;
}

void CatManDlg::onCategoryListSelectChange(const QItemSelection & selection , const QItemSelection & previou  ) 
{
	QString dir;
	int size;
    int catId;
	QModelIndex idx;
	QModelIndexList mil;
    

	size = selection.size();
	for (int i = 0 ; i < size ; i ++) {
		mil = selection.indexes() ;
		qDebug()<<mil<<mil.size() ;
		for(int j = 0 ; j < mil.size() ; j ++) {
			qDebug()<<this->mCatModel->data(mil.at(j));
            if (j == ng::cats::cat_id) {
                catId = this->mCatModel->data(mil.at(j)).toInt();
            }
		}
		// if (this->mCatModel->data(mil.at(1)).toString().isEmpty()) {
        if (catId == ng::cats::cat_root || catId == ng::cats::downloading
            || catId == ng::cats::deleted) {
			//maybe selected system default cat, like NULLGET, downloading, deleted
            this->uiwin.cmd_tv_cat_tree->selectionModel()->select(previou, QItemSelectionModel::ClearAndSelect);
		} else {
			this->uiwin.cmd_le_default_directory->setText(this->mCatModel->data(mil.at(1)).toString() + 
				QDir::separator() + this->uiwin.cmd_le_cat_name->text());
		}
	}
}

void CatManDlg::onModifyCategoryDefaultDirectory( )
{
	QString path = this->uiwin.cmd_le_default_directory->text() ;
	QString newpath ;

	newpath = QFileDialog::getExistingDirectory(this , "Choose a directory",   path  ,
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if(newpath.isEmpty()) {
	} else {
		newpath = QDir::convertSeparators(newpath);
		this->uiwin.cmd_le_default_directory->setText(newpath+QDir::separator());
	}
}

QItemSelectionModel * CatManDlg::getSelectionModel()
{
	return this->uiwin.cmd_tv_cat_tree->selectionModel() ;
}

//void  CatManDlg::onCreateNewCategory()
//{
//	qDebug()<<__FUNCTION__;

//}

QString CatManDlg::getNewCatName() 
{
	return this->uiwin.cmd_le_cat_name->text();
}
QString CatManDlg::getNewCatDir()
{
	return this->uiwin.cmd_le_default_directory->text() ;

}

void CatManDlg::onCatNameChanged(QString name)
{
	QChar sep = QDir::separator ();
	QString oldpath = this->uiwin.cmd_le_default_directory->text() ;
	QStringList sl = oldpath.split(sep);
	sl.replace(sl.size()-1,name);
	QString newpath = sl.join(sep);
	this->uiwin.cmd_le_default_directory->setText(newpath) ;
}

void CatManDlg::changeMoveToState(QModelIndexList & indexList)
{
	this->mState = 1 ;	//move to

	QObject::disconnect(this->uiwin.cmd_tv_cat_tree->selectionModel(),
                        SIGNAL(selectionChanged ( const QItemSelection & , const QItemSelection &   )),
                        this, 
                        SLOT(onCategoryListSelectChange( const QItemSelection & , const QItemSelection &   ) ) ) ;
	this->uiwin.cmd_le_cat_name->setDisabled(true);
	this->uiwin.cmd_le_default_directory->setDisabled(true);
	this->uiwin.cmd_tb_change_director->setDisabled(true);

	this->uiwin.cmd_le_cat_name->setText(indexList.at(0).data().toString());
	this->uiwin.cmd_le_default_directory->setText(indexList.at(1).data().toString());

}

void CatManDlg::onAccept()
{

}
