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

#include "sip_entry.h"

// global vars
SipVarSet *hSip;

SkyServ::SkyServ(QObject *parent)
    : QObject(parent)
{
    // todo , connection stabilty
    this->db = new Database();
    this->db->connectdb();

    hSip = new SipVarSet();
    hSip->sip_app_init();
    QObject::connect(hSip, SIGNAL(sip_call_finished(int, int)),
                     this, SLOT(onSipCallFinished(int, int)));
    QObject::connect(hSip, SIGNAL(sip_call_media_server_ready(unsigned short)),
                     this, SLOT(onSipCallMediaServerReady(unsigned short)));
    QObject::connect(hSip, SIGNAL(sip_call_incoming_media_server_ready(unsigned short)),
                     this, SLOT(onSipCallIncomingMediaServerReady(unsigned short)));

    this->mSkype = new Skype("karia2");
    QObject::connect(this->mSkype, SIGNAL(connected(QString)),
                     this, SLOT(onSkypeConnected(QString)));
    QObject::connect(this->mSkype, SIGNAL(connectionLost(QString)),
                     this, SLOT(onSkypeDisconnected(QString)));
    QObject::connect(this->mSkype, SIGNAL(skypeError(int, QString)),
                     this, SLOT(onSkypeError(int, QString)));
    this->mSkype->connectToSkype();
    QObject::connect(this->mSkype, SIGNAL(packageArrived(QString, int, QString)),
                     this, SLOT(onSkypePackageArrived(QString, int, QString)));
    QObject::connect(this->mSkype, SIGNAL(newCallArrived(QString, QString, int)),
                     this, SLOT(onNewCallArrived(QString, QString, int)));
    QObject::connect(this->mSkype, SIGNAL(callHangup(QString, QString, int)),
                     this, SLOT(onCallHangup(QString, QString, int)));
    QObject::connect(this->mSkype, SIGNAL(callAnswered(int)),
                     this, SLOT(onCallAnswered(int)));


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
    qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<skypeName;
}

void SkyServ::onSkypeDisconnected(QString skypeName)
{
    qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<skypeName;
}

void SkyServ::onNewStreamCreated(QString contactName, int stream)
{

}

void SkyServ::onNewCallArrived(QString callerName, QString calleeName, int callID)
{
    qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<callerName<<calleeName<<callID;
    pj_status_t status;
    QString callee_phone;

    // 查找
    callee_phone = this->db->getForwardPhone(callerName, calleeName);
    if (callee_phone.isEmpty() || callee_phone.length() == 0) {
        qDebug()<<"Error: call pair not found.";
        return;
    }
    // accept
    this->mSkype->answerCall(QString("%1").arg(callID));

    // sip call
    QString serv_addr = "172.24.172.21:5060";
    status = hSip->call_phone(callerName, callee_phone, serv_addr);
    hSip->payload_id = callID;

}

void SkyServ::onCallHangup(QString contactName, QString calleeName, int callID)
{
    qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<contactName<<callID;
    
}

void SkyServ::onCallAnswered(int callID)
{
    qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<callID;
    // set call output port
    // this->mSkype->setCallMediaOutputPort(QString::number(callID), 12356);
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
        ret = this->db->releaseGateway(contactName, sp->data);
        // ret = this->db->releaseGateway(contactName, sp->data);
        rsp.seq = sp->seq;
        rsp.type = SkypePackage::SPT_GW_RELEASE_RESULT;
        rsp.data = QString("ret=%1&caller=%2&gateway=%3").arg(ret).arg(contactName).arg(sp->data);
        
        rspStr = rsp.toString();
        this->mSkype->sendPackage(contactName, stream, rspStr);
        this->ccMap.remove(contactName);

        break;
    default:
        Q_ASSERT(1==2);
        break;
    };
}


// from sip signals
void SkyServ::onSipCallFinished(int call_id, int status_code)
{
    qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<call_id<<status_code;

    int skype_call_id = hSip->payload_id;
    this->mSkype->setCallHangup(QString("%1").arg(skype_call_id));
    hSip->payload_id = -1;
}

void SkyServ::onSipCallMediaServerReady(unsigned short port)
{
    qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<port;

    int skype_call_id = hSip->payload_id;
    int ok = this->mSkype->setCallMediaInputPort(QString("%1").arg(skype_call_id), port);

}

void SkyServ::onSipCallIncomingMediaServerReady(unsigned short port)
{
    qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<port;

    int skype_call_id = hSip->payload_id;
    int ok = this->mSkype->setCallMediaOutputPort(QString::number(skype_call_id), port);
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
