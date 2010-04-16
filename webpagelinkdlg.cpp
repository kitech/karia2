// webpagelinkdlg.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-10 21:28:08 +0800
// Version: $Id$
// 
#include <QtCore>
#include <QtGui>
#include <QMainWindow>
#include <QMenu>
#include <QActionGroup>
#include <QStatusBar>
#include <QMessageBox>
#include <QDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QUrlInfo>
#include <QUrl>
#include <QFileInfo>
#include <QList>

#include <QRect>
#include <QSize>
#include <QProcess>
#include <QDockWidget>

#include <QToolTip>
#include <QListWidgetItem>
#include <QRegExp>

#include "webpagelinkdlg.h"

WebPageHostSelectDlg::WebPageHostSelectDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	//
	QObject::connect(this->ui.wphs_cb_host,SIGNAL(clicked()),this,SLOT(onSelectAllHost()));
	QObject::connect(this->ui.wphs_cb_extension,SIGNAL(clicked()),this,SLOT(onSelectAllExtension()));
}

WebPageHostSelectDlg::~WebPageHostSelectDlg()
{

}

void WebPageHostSelectDlg::setHostList( QStringList host , QStringList ext)
{
	this->mAllHost = host;
	this->mAllExtension = ext ;

	QListWidgetItem * wi ;

	for( int i = 0 ; i < host.size() ; ++ i )
	{
		wi = new QListWidgetItem(host.at(i));
		wi->setCheckState(Qt::Checked);
		this->ui.wphs_lw_host_list->addItem(wi);
	}

	for( int i = 0 ; i < ext.size() ; ++ i )
	{
		wi = new QListWidgetItem(ext.at(i));
		wi->setCheckState(Qt::Checked);
		this->ui.wphs_lw_extension_list->addItem(wi);
	}

}

void WebPageHostSelectDlg::onSelectAllHost()
{

		int count = this->ui.wphs_lw_host_list->count();
		for( int i = 0 ; i < count ; ++i )
		{
			if( this->ui.wphs_cb_host->isChecked() )
			{
				this->ui.wphs_lw_host_list->item(i)->setCheckState(Qt::Checked) ;
			}	
			else
			{
				this->ui.wphs_lw_host_list->item(i)->setCheckState(Qt::Unchecked) ;
			}
		}
		
}

void WebPageHostSelectDlg::onSelectAllExtension()
{
	int count = this->ui.wphs_lw_extension_list->count();
	for( int i = 0 ; i < count ; ++i )
	{
		if( this->ui.wphs_cb_extension->isChecked() )
		{
			this->ui.wphs_lw_extension_list->item(i)->setCheckState(Qt::Checked) ;
		}	
		else
		{
			this->ui.wphs_lw_extension_list->item(i)->setCheckState(Qt::Unchecked) ;
		}
	}
}

QList<QStringList> WebPageHostSelectDlg::getResultHostList()
{
	QList<QStringList> hostList ;
	QStringList host ,ext ;


		int count = this->ui.wphs_lw_host_list->count();
		for( int i = 0 ; i < count ; ++i )
		{
			if( this->ui.wphs_lw_host_list->item(i)->checkState() == Qt::Checked )
			{
				host.append( this->ui.wphs_lw_host_list->item(i)->text() );
			}
		}

	count = this->ui.wphs_lw_extension_list->count();
	for( int i = 0 ; i < count ; ++i )
	{
		if( this->ui.wphs_lw_extension_list->item(i)->checkState() == Qt::Checked )
		{
			ext.append( this->ui.wphs_lw_extension_list->item(i)->text() );
		}
	}	

	hostList.append(host);
	hostList.append(ext);

	return hostList ;
}

///////////////////////
//////////////////
WebPageLinkDlg::WebPageLinkDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	//
	QObject::connect(this->ui.wpl_pb_option,SIGNAL(clicked()),this,SLOT(onChangeDefaultMakeType()));
	QObject::connect(this->ui.wpl_pb_mark_highlight,SIGNAL(clicked()),this,SLOT(onMarkHightLight()));
	
	QObject::connect(this->ui.wpl_pb_choose,SIGNAL(clicked()),this,SLOT(onChooseHost()));
}

WebPageLinkDlg::~WebPageLinkDlg()
{

}

int WebPageLinkDlg::exec () 
{
	QString host ;
	QListWidgetItem * wi ;

	host = QInputDialog::getText(this,"Input Host:","Enter the Default host When relative Link accurs");
	if( ! host.trimmed().isEmpty() ) this->mUserInputHost = host = host+QString("/") ;
	
	//this->show();
	QUrl u ;
	//insert items
	for( int i = 0 ; i < this->mSrcUrl.size() ; ++i)
	{
		 u = (this->mSrcUrl.at(i));
		 if( u.isRelative() )
		 {
			wi = new QListWidgetItem(host+this->mSrcUrl.at(i));
		 }
		 else
		 {
			wi = new QListWidgetItem(this->mSrcUrl.at(i));
		 }
		
		wi->setCheckState(Qt::Checked);
		this->ui.wpl_lw_url->addItem(wi);

	}

	//this->hide();

	return QDialog::exec();
}

void WebPageLinkDlg::setSourceUrlList(QStringList ul)
{
	this->mSrcUrl = ul ;
	//捡出所有的HOST 及所有的 扩展名列表。
	for( int i = 0 ; i < ul.size() ; ++i)
	{
		QStringList tmpList = QFileInfo(ul.at(i)).fileName().split(".");
		QUrl u(ul.at(i));
		QString chost = QString("%1:%2").arg(u.host()).arg(u.port(80)) ;
		if( ! u.isRelative() && ! this->mAllHost.contains(chost) )
		{
			this->mAllHost.append( chost ) ;
		}
		if( u.path().length() > 0)
		{
			chost = QString(".")+tmpList.at(tmpList.size()-1 );
			if( chost.length()>1) 
			if( ! this->mAllExtension.contains( chost ) )
			{
				this->mAllExtension.append( chost ) ;
			}
		}
	}
	qDebug()<<this->mAllExtension;
	qDebug()<<this->mAllHost ;

}

QStringList WebPageLinkDlg::getResultUrlList()
{
	this->mResultUrl.clear();

	for( int i = 0 ; i < this->ui.wpl_lw_url->count() ; ++i )
	{
		if( this->ui.wpl_lw_url->item(i)->checkState() == Qt::Checked )
		{
			this->mResultUrl.append(this->ui.wpl_lw_url->item(i)->text() );
		}
	}

	return this->mResultUrl;

}

void WebPageLinkDlg::onChangeDefaultMakeType()
{
	QString mark ;
	QStringList markList ;
	QRegExp reg ;

	QStringList markHas ;
	QListWidgetItem * wi ;

	markHas.append(".ZIP;.EXE;.BIN;.GZ;.BZ2");
	markHas.append(".Z;.TAR;.ARJ;.LZH");
	markHas.append(".MP3;.MPG;.MPEG;.RM;.RMVB;.MIDI;.MOV;.AVI");
	markHas.append(".HTM;.HTML");
	markHas.append(".TXT;.LOG");
	markHas.append(".*");

	mark = QInputDialog::getItem(this,"Set Default Mark:","the Default Mark Is:",markHas).trimmed();
	
	if( ! mark.isEmpty() )
	{
		markList = mark.split(";");
		this->ui.wpl_lw_url->clear();
		QUrl u ;
		//insert items
		for( int i = 0 ; i < this->mSrcUrl.size() ; ++i)
		{
			 u = (this->mSrcUrl.at(i));
			 QFileInfo fi(this->mSrcUrl.at(i));
			 
			 for( int j = 0 ; j < markList.size() ; ++j)
			 {
				 QStringList tmpList = fi.fileName().split(".");
				 reg.setCaseSensitivity(Qt::CaseInsensitive);
				 reg.setPattern(markList.at(j).right(markList.at(j).length()-1));
				 if( mark.compare("*")==0 ||mark.compare(".*")==0|| reg.indexIn(tmpList.at(tmpList.size()-1))>=0)
				 {
					 if( u.isRelative() )
					 {
						 wi = new QListWidgetItem(this->mUserInputHost+this->mSrcUrl.at(i));
					 }
					 else
					 {
						wi = new QListWidgetItem(this->mSrcUrl.at(i));
					 }
					
					wi->setCheckState(Qt::Checked);
					this->ui.wpl_lw_url->addItem(wi);
					break;
				 }
			 }
		}
		
	}
}

void WebPageLinkDlg::onMarkHightLight()
{
	//qDebug()<<__FUNCTION__ ;

	QList<QListWidgetItem*> items = this->ui.wpl_lw_url->selectedItems();
	if( items.size() > 0 )
	{
		for(int i = 0 ; i < items.size() ; i++)
		{
			if( items.at(i)->checkState() == Qt::Checked)
			{
				items.at(i)->setCheckState(Qt::Unchecked);
			}
			else
			{
				items.at(i)->setCheckState(Qt::Checked);
			}
		}
	}
}

void WebPageLinkDlg::onChooseHost()
{
	WebPageHostSelectDlg *wphs = 0 ;
	int er = QDialog::Rejected ;
	QStringList currUrlList ;
	QList<QStringList> selectedHost ;
	QStringList hostList;
	QStringList extList ;

	wphs = new WebPageHostSelectDlg(this);
	
	wphs->setHostList(this->mAllHost,this->mAllExtension);

	er = wphs->exec();
	if( er == QDialog::Accepted )
	{
		selectedHost = wphs->getResultHostList();
		hostList = selectedHost.at(0);
		extList = selectedHost.at(1);

		//取选择的列表。对当前的URL表进行过滤。
		int i = this->ui.wpl_lw_url->count() - 1 ;
		while( i  >= 0 )
		{
			QString currUrl = this->ui.wpl_lw_url->item(i)->text();
			QUrl u(currUrl);
			QFileInfo fi(currUrl);
			bool deleteItem = false ;
			if( ! u.isRelative() )
			{
				QString ch = QString("%1:%2").arg(u.host()).arg(u.port(80));
				if( hostList.contains(ch)   ) 
				{;}
				else
				{
					deleteItem = true ;
				}
			}
			QStringList tmpList =fi.fileName().split(".") ;
			if( ! extList.contains(QString(".") + tmpList.at(tmpList.size()-1)))
			{
				deleteItem = true ;
			}
			if( deleteItem == true )
				delete this->ui.wpl_lw_url->takeItem(i); 
			i -- ;
		}
	}

	if( wphs == 0 ) delete wphs ;
}

/////////////////////////
WebPageUrlInputDlg::WebPageUrlInputDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	//
	QObject::connect(this->ui.wpi_tb_select_file,SIGNAL(clicked()),this,SLOT(onOpenFile()));

}

WebPageUrlInputDlg::~WebPageUrlInputDlg()
{

}

void WebPageUrlInputDlg::onOpenFile()
{
	QString fname ;
	QStringList fnamelist ;
	fnamelist = QFileDialog::getOpenFileNames(this,"Open The Web Page File:","","HTML File(*.html;*.htm)");
	qDebug()<<(fnamelist);

	fname = fnamelist.join("\"+\"");
	fname = QString("\"%1\"").arg(fname);
	this->ui.wpi_cb_url->setEditText(fname);
	
}

QString WebPageUrlInputDlg::getLinkList()
{
	return this->ui.wpi_cb_url->currentText().trimmed() ;
}
