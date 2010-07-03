// labspace.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-03 13:30:32 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QApplication>
#include <QtGui>

#include "labspace.h"
#include "libng/qbihash.h"
KBiHash<QString, int> bihTest;

LabSpace::LabSpace(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	
	mSysMon = new QFileSystemWatcher(this);
#ifdef WIN32
	mSysMon->addPath("d:/temp/aa.txt");	
	mSysMon->addPath("d:/temp/");	//好象对文件管用，对目录不管用。windows平台是这样的。Linux下是可用的。
#else
	// mSysMon->addPath(QDir::homePath() + "/aa.txt");	
	// mSysMon->addPath(QDir::homePath());	//好象对文件管用，对目录不管用。windows平台是这样的。
    // mSysMon->addPath(QDir::homePath() + "/.opera/cookies4.dat");
#endif

	QObject::connect(mSysMon, SIGNAL(directoryChanged ( const QString &  )) ,
		this, SLOT(onDirectoryChanged ( const QString &  ) ) );
	QObject::connect(mSysMon, SIGNAL(fileChanged ( const QString &  ) ),
		this, SLOT(onFileChanged ( const QString & ) ) ) ;
	qDebug()<<__FUNCTION__<<__LINE__;

	QObject::connect(this->ui.pushButton,SIGNAL(clicked()),
		this,SLOT(getDiskRawData()) );
}

LabSpace::~LabSpace()
{

}

void LabSpace::onDirectoryChanged ( const QString & path )
{
	qDebug()<<__FUNCTION__<<__LINE__;
	qDebug()<< path ;
    // sometimes disappear readd
    if (!this->mSysMon->directories().contains(path)) {
        this->mSysMon->addPath(path);
    }
}
void LabSpace::onFileChanged ( const QString & path ) 
{
	qDebug()<<__FUNCTION__<<__LINE__;
	qDebug()<< path << this->mSysMon->files();
    // sometimes disappear readd
    if (!this->mSysMon->files().contains(path)) {
        this->mSysMon->addPath(path);
    }
}

void LabSpace::getDiskRawData()
{
	QString rawData ;

#ifdef WIN32

	//读取磁盘C的前两个扇区
	CDiskInfo CD;
	unsigned char Buf[1024];
	BOOL bRet=
	CD.ReadSectors(3,0,2,Buf);

	if(!bRet)
	{
		qDebug()<<" CD.ReadSectors(3,0,2,Buf); error";
		return ;
	}

	char Str[1024*10];
	memset(Str,0,1024*10);
	for(int i=0;i<1024;i++)
	{
		sprintf(Str,"%s%02X ",Str,Buf[i]);
		if((i%16)==15)sprintf(Str,"%s\r\n",Str);
	}
	
	rawData = Str ;

	this->ui.textEdit->setText(rawData);

#endif


	return  ;
}



