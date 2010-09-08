// skypetunnel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-09-08 14:12:07 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#include "metauri.h"
#include "skype.h"

#include "skypetunnel.h"

SkypeTunnel::SkypeTunnel(QObject *parent)
    : QObject(parent)
{
    this->mSkype = NULL;
}

SkypeTunnel::~SkypeTunnel()
{

}

void SkypeTunnel::setSkype(Skype *skype)
{
    Q_ASSERT(skype != NULL);

    this->mSkype = skype;
    QObject::connect(this->mSkype, SIGNAL(skypeError(int, QString)),
                     this, SLOT(onSkypeError(int, QString)));
    this->mSkype->connectToSkype();
    QObject::connect(this->mSkype, SIGNAL(packageArrived(QString, int, QString)),
                     this, SLOT(onSkypePackageArrived(QString, int, QString)));
}

void SkypeTunnel::onSkypeError(int errNo, QString msg)
{
    qDebug()<<errNo<<msg;
}

void SkypeTunnel::onSkypeConnected(QString skypeName)
{

}

void SkypeTunnel::onSkypeDisconnected(QString skypeName)
{

}

void SkypeTunnel::onNewStreamCreated(QString contactName, int stream)
{

}

void SkypeTunnel::onSkypePackageArrived(QString contactName, int stream, QString data)
{
    qDebug()<<contactName<<stream<<data;
    SkypePackage sp = SkypePackage::fromString(data);
    Q_ASSERT(sp.isValid());

    this->processRequest(contactName, stream, &sp);
}

void SkypeTunnel::processRequest(QString contactName, int stream, SkypePackage *sp)
{
    MetaUri mu;
    MetaUri rmu; // response
    SkypePackage rsp; // response package
    QString rspStr;
    QString callee_name;

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
        rsp.seq = sp->seq;
        rsp.type = SkypePackage::SPT_GW_SELECT_RESULT;
        rsp.data = QString("tivr001");
        
        rspStr = rsp.toString();
        this->mSkype->sendPackage(contactName, stream, rspStr);
        break;
    case SkypePackage::SPT_GW_SELECT_RESULT:
        callee_name = sp->data;
        qDebug()<<"get a ivr gate way: "<<sp->data;
        qDebug()<<"ready for call,...";
        this->mSkype->callFriend(callee_name);
        qDebug()<<"ready for called done";
        break;
    default:
        Q_ASSERT(1==2);
        break;
    };
}

