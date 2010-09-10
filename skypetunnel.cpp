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
    QObject::connect(this->mSkype, SIGNAL(newCallArrived(QString, int)),
                     this, SLOT(onNewCallArrived(QString, int)));

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

void SkypeTunnel::onNewCallArrived(QString contactName, int callID)
{
    qDebug()<<__FILE__<<__LINE__<<contactName<<callID;
    if (contactName.startsWith("+")) {
        // int ok = this->mSkype->setCallHold(QString("%1").arg(callID));
        int ok = this->mSkype->setCallHangup(QString("%1").arg(callID));
        // QString ret = this->mSkype->callFriend(contactName);

        QString num = contactName; // this->mainUI.lineEdit_2->text();
        SkypePackage sp;
        sp.seq = SkypeCommand::nextID().toInt();
        sp.type = SkypePackage::SPT_GW_SELECT;
        sp.data = num;

        QString spStr = sp.toString();
        qDebug()<<spStr;
        this->mSkype->sendPackage("drswinghead", spStr);

    } else {
        // pstn
    }
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
    QUrl turl;
    QString rcode;

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
        turl = QString("http://www.qtchina.net/skype.cgi?%1").arg(sp->data);
        // callee_name = sp->data;
        rcode = turl.queryItemValue("ret");
        callee_name = turl.queryItemValue("gateway");
        qDebug()<<"get a ivr gate way: "<<sp->data<<turl;
        qDebug()<<"ready for call,...";
        this->mSkype->callFriend(callee_name);
        qDebug()<<"ready for called done";
        break;
    default:
        Q_ASSERT(1==2);
        break;
    };
}

