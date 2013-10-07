// aria2jsonmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-26 22:35:48 +0000
// Version: $Id$
// 
#ifndef _ARIA2JSONRPCMANAGER_H_
#define _ARIA2JSONRPCMANAGER_H_

#include <QtNetwork>

#include "qjsonrpcmessage.h"
#include "qjsonrpcservice.h"

#include "aria2rpcmanager.h"

class Aria2JsonRpcClient;

/**
 * 负责解析Json格式结果，转换成统一格式返回给显示层
 * 负责组装Json格式请求，发送给后端处理
 */
class Aria2JsonManager : public Aria2RpcManager
{
    Q_OBJECT;
public:
    Aria2JsonManager();
    virtual ~Aria2JsonManager();

public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to);
    virtual int pauseTask(int task_id);
    /////
    virtual bool onAllStatArrived(int stkey);
    virtual bool setSpeedLimit(int downloadSpeed, int uploadSpeed) {};

private:
    Aria2JsonRpcClient *mJsonRpc;
};

class  Aria2JsonRpcClient : public QObject
{
    Q_OBJECT;
public:
    Aria2JsonRpcClient(QString url);
    virtual ~Aria2JsonRpcClient();

    bool call(QString method, QVariantList arguments);

protected slots:
    bool onRawSocketConnected();

private:
    QString mUrl;
    QJsonRpcSocket *mJsonRpcSock;
    QTcpSocket *mRawSock;
    QString method;
    QVariantList arguments;
};

#endif /* _ARIA2JSONRPCMANAGER_H_ */
