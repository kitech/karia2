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

#include "metauri.h"
#include "skype.h"

#include "database.h"

SkyServ::SkyServ(QObject *parent)
    : QObject(parent)
{

    this->db = new Database();
    this->db->connectdb();



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

void SkyServ::onNewStreamCreated(QString contactName, int stream)
{

}

void SkyServ::onNewCallArrived(QString contactName, int callID)
{
    
}

void SkyServ::onSkypePackageArrived(QString contactName, int stream, QString data)
{
    qDebug()<<contactName<<stream<<data;
    SkypePackage sp = SkypePackage::fromString(data);
    Q_ASSERT(sp.isValid());

    this->processRequest(contactName, stream, &sp);
}

void SkyServ::processRequest(QString contactName, int stream, SkypePackage *sp)
{
    MetaUri mu;
    MetaUri rmu; // response
    SkypePackage rsp; // response package
    QString rspStr;
    QString myStr;
    int ret = -1;

    switch (sp->type) {
    case SkypePackage::SPT_MU_ADD:
        mu = MetaUri::fromString(sp->data);
        // add to storage here;
        mu.dump();
        rsp.seq = sp->seq; // the same as request
        rsp.type = SkypePackage::SPT_MU_ADD_RESP;
        rsp.data = QString("OK");

        rspStr = rsp.toString();
        this->mSkype->sendPackage(contactName, stream, rspStr);
        break;
    case SkypePackage::SPT_MU_DELETE:
        break;
    case SkypePackage::SPT_GW_SELECT:
        qDebug()<<"SPT_GW_SELECT: "<<sp->data;

        ret = this->db->acquireGateway(contactName, sp->data, myStr);

        rsp.seq = sp->seq;
        rsp.type = SkypePackage::SPT_GW_SELECT_RESULT;
        rsp.data = QString("ret=%1&gateway=%2").arg(ret).arg(myStr);
        
        rspStr = rsp.toString();
        this->mSkype->sendPackage(contactName, stream, rspStr);
        this->ccMap[contactName] = sp->data;
        
        break;
    case SkypePackage::SPT_GW_RELEASE:
        qDebug()<<"SPT_GW_RELEASE: "<<sp->data;
        // this cmd will recived from gateway
        ret = this->db->releaseGateway(sp->data, contactName);
        // ret = this->db->releaseGateway(contactName, sp->data);
        rsp.seq = sp->seq;
        rsp.type = SkypePackage::SPT_GW_RELEASE_RESULT;
        rsp.data = QString("ret=%1").arg(ret);
        
        rspStr = rsp.toString();
        this->mSkype->sendPackage(contactName, stream, rspStr);
        this->ccMap.remove(contactName);

        break;
    default:
        Q_ASSERT(1==2);
        break;
    };
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
