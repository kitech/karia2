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
#include "viewmodel.h"
#include "catmandlg.h"

#include "sqlitecategorymodel.h"

CatManDlg::CatManDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	this->mState = 0 ;	//create
	//this->mCatModel  = CategoryModel::instance();
	this->mCatModel = SqliteCategoryModel::instance(0);

	this->ui.cmd_tv_cat_tree->setModel(this->mCatModel);
	
	//this->expandAll();
	this->ui.cmd_tv_cat_tree->expandAll();
	this->ui.cmd_tv_cat_tree->resizeColumnToContents(0);

	//this->ui.cmd_tv_cat_tree->expand(this->mCatModel->index(0,0));
	//this->ui.cmd_tv_cat_tree->expand(this->mCatModel->index(1,0,this->mCatModel->index(0,0)) );
		
	QObject::connect(this->ui.cmd_tv_cat_tree->selectionModel(),
                     SIGNAL(selectionChanged ( const QItemSelection & , const QItemSelection &)),
                     this, SLOT(onCategoryListSelectChange(const QItemSelection &, const QItemSelection &)));
	QObject::connect(this->ui.cmd_tb_change_director, SIGNAL(clicked()), this, SLOT(onModifyCategoryDefaultDirectory()));
	//QObject::connect(this->ui.okButton,SIGNAL(clicked()),this,SLOT(onCreateNewCategory()));
	QObject::connect(this->ui.cmd_le_cat_name, SIGNAL(textChanged(QString )), this, SLOT(onCatNameChanged(QString)));
	
	//on windows
	this->ui.cmd_le_default_directory->setText("C:\\NGDownloads\\");
	//on unix maybe
	//this->ui.cmd_le_default_directory->setText("~/.nullget/NGDownloads/");

}

CatManDlg::~CatManDlg()
{
	//delete this->mCatModel ;
}
//void CatManDlg::expandAll(QModelIndex  index )
//{
//	QModelIndexList mil ;
//	QModelIndex idx ,parent ;
//	const QAbstractItemModel * model = 0 ;
//	int row , col ;
//	if( ! index.isValid()  )
//	{
//		model = this->mCatModel ;
//		index = model->index(0,0);
//	}
//	else 
//	{
//		model = index.model();
//	}
//	row = model->rowCount(index) ;
//
//	//qDebug()<<index.isValid()<<row<<model->data(index);	
//
//	this->ui.cmd_tv_cat_tree->expand(index);
//	this->ui.cmd_tv_cat_tree->resizeColumnToContents(0);
//
//	for( int i = row-1 ; i >=0 ; i-- )
//	{
//		idx = model->index(i,0,index);
//
//		this->expandAll(idx);
//	}
//
//}
//void CatManDlg::collapseAll()
//{
//
//}

void	CatManDlg::onCategoryListSelectChange( const QItemSelection & selection , const QItemSelection & previou  ) 
{
	
	QString dir ;
	int size ;
	size = selection.size();
	QModelIndex idx ;
	QModelIndexList mil ;

	for( int i = 0 ; i < size ; i ++)
	{
		mil = selection.indexes() ;
		qDebug()<<mil<<mil.size() ;
		for(int j = 0 ;j < mil.size() ; j ++ )
		{
			qDebug()<<this->mCatModel->data(mil.at(j));
		}
		if( this->mCatModel->data(mil.at(1)).toString().isEmpty() )
		{
			//可能是选择了根
		}
		else
		{
			this->ui.cmd_le_default_directory->setText(this->mCatModel->data(mil.at(1)).toString() + 
				QDir::separator() + this->ui.cmd_le_cat_name->text());
		}
	}
}

void CatManDlg::onModifyCategoryDefaultDirectory( )
{
	QString path = this->ui.cmd_le_default_directory->text() ;
	QString newpath ;

	newpath = QFileDialog::getExistingDirectory(this , "Choose a directory",   path  ,
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if( newpath.isEmpty())
	{
	}
	else
	{
		newpath = QDir::convertSeparators(newpath);
		this->ui.cmd_le_default_directory->setText(newpath+QDir::separator());
	}
	
}

QItemSelectionModel * CatManDlg::getSelectionModel()
{
	return this->ui.cmd_tv_cat_tree->selectionModel() ;
}

//void  CatManDlg::onCreateNewCategory()
//{
//	qDebug()<<__FUNCTION__;

//}

QString CatManDlg::getNewCatName() 
{
	return this->ui.cmd_le_cat_name->text();
}
QString CatManDlg::getNewCatDir()
{
	return this->ui.cmd_le_default_directory->text() ;

}

void CatManDlg::onCatNameChanged(QString name)
{
	QChar sep = QDir::separator ();
	QString oldpath = this->ui.cmd_le_default_directory->text() ;
	QStringList sl = oldpath.split(sep);
	sl.replace(sl.size()-1,name);
	QString newpath = sl.join(sep);
	this->ui.cmd_le_default_directory->setText(newpath) ;
}

void CatManDlg::changeMoveToState(QModelIndexList & indexList)
{
	this->mState = 1 ;	//move to

	QObject::disconnect(this->ui.cmd_tv_cat_tree->selectionModel(),
			SIGNAL(selectionChanged ( const QItemSelection & , const QItemSelection &   )),
		this,SLOT(onCategoryListSelectChange( const QItemSelection & , const QItemSelection &   ) ) ) ;
	this->ui.cmd_le_cat_name->setDisabled(true);
	this->ui.cmd_le_default_directory->setDisabled(true);
	this->ui.cmd_tb_change_director->setDisabled(true);

	this->ui.cmd_le_cat_name->setText(indexList.at(0).data().toString());
	this->ui.cmd_le_default_directory->setText(indexList.at(1).data().toString());

}
