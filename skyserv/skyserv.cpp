// skyserv.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-03 15:35:48 +0800
// Version: $Id$
// 

#include <QtCore>

#include "skyserv.h"
#include "ui_skyserv.h"

#include "skype.h"

SkyServ::SkyServ(QObject *parent)
    : QObject(parent)
{
    this->mSkype = new Skype("karia2");
    QObject::connect(this->mSkype, SIGNAL(skypeError(int, QString)),
                     this, SLOT(onSkypeError(int, QString)));
    this->mSkype->connectToSkype();
    QObject::connect(this->mSkype, SIGNAL(packageArrived(QString, int, QString)),
                     this, SLOT(onSkypePackageArrived(QString, int, QString)));
    // QObject::connect(this->mainUI.actionSkype_Tracer_2, SIGNAL(triggered(bool)),
    //                  this, SLOT(onShowSkypeTracer(bool)));
    // QObject::connect(this->mainUI.pushButton, SIGNAL(clicked()),
    //                  this, SLOT(onChatWithSkype()));
    // QObject::connect(this->mainUI.pushButton_2, SIGNAL(clicked()),
    //                  this, SLOT(onSendPackage()));
}

SkyServ::~SkyServ()
{
}

void SkyServ::onSkypeError(int errNo, QString msg)
{
    qDebug()<<errNo<<msg;
}

void SkyServ::onSkypeConnected(QString skypeName)
{

}

void SkyServ::onSkypeDisconnected(QString skypeName)
{

}

void SkyServ::onSkypePackageArrived(QString contactName, int stream, QString data)
{
    qDebug()<<contactName<<stream<<data;
}

// SkyServ::SkyServ(QWidget *parent) :
//     QMainWindow(parent),
//     ui(new Ui::SkyServ)
// {
//     ui->setupUi(this);
// }

// SkyServ::~SkyServ()
// {
//     delete ui;
// }
