// aria2jsonmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-26 22:36:51 +0000
// Version: $Id$
// 

#include "simplelog.h"
#include "taskinfodlg.h"

#include "karia2statcalc.h"
#include "aria2rpcserver.h"
#include "aria2jsonmanager.h"

Aria2JsonManager::Aria2JsonManager()
    : Aria2RpcManager()
{

}

Aria2JsonManager::~Aria2JsonManager()
{
}

int Aria2JsonManager::addTask(int task_id, const QString &url, TaskOption *to)
{

    Aria2JsonRpcClient *jrpc = new Aria2JsonRpcClient(this->mRpcServer->getRpcUri(Aria2RpcServer::AST_JSONRPC_HTTP));
    QString method = "aria2.getVersion";
    QVariantList args;
    args << QString("a");

    jrpc->call(method, args);
    qLogx()<<"called";
    
    return 0;
}

int Aria2JsonManager::pauseTask(int task_id)
{
    
    return 0;
}

bool Aria2JsonManager::onAllStatArrived(int stkey)
{
    
    return true;
}

////////////////
Aria2JsonRpcClient::Aria2JsonRpcClient(QString url)
    : QObject(0)
    , mJsonRpcSock(NULL)
    , mRawSock(NULL)
{
    this->mUrl = url;
}

Aria2JsonRpcClient::~Aria2JsonRpcClient()
{
}

bool Aria2JsonRpcClient::call(QString method, QVariantList arguments)
{
    this->mRawSock = new QTcpSocket();
    QObject::connect(this->mRawSock, &QTcpSocket::connected, this, &Aria2JsonRpcClient::onRawSocketConnected);
    QUrl uo(this->mUrl);
    this->mRawSock->connectToHost(uo.host(), uo.port());

    this->method = method;
    this->arguments = arguments;

    qLogx()<<"connecting...";

    return true;
}

bool Aria2JsonRpcClient::onRawSocketConnected()
{
    qLogx()<<"sending...";

    this->mJsonRpcSock = new QJsonRpcSocket(this->mRawSock);

    QJsonRpcMessage request = QJsonRpcMessage::createRequest(method, arguments);
    // QJsonRpcServiceReply *reply = this->mJsonRpcSock->sendMessage(request);
    QJsonRpcMessage response = this->mJsonRpcSock->sendMessageBlocking(request, 5000);
    
    qLogx()<<response.result();
    
    return true;
}
