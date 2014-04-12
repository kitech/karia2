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

#include <memory>

#include <QtNetwork>

#include "qjsonrpcmessage.h"
#include "qjsonrpcservice.h"

#include "aria2rpcmanager.h"

class Karia2StatCalc;
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

    virtual void run();

public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to);
    virtual int pauseTask(int task_id);
    /////
    virtual bool onAllStatArrived(int stkey);
    virtual bool setSpeedLimit(int downloadSpeed, int uploadSpeed) {return true;};

/**
 * 实现下载状态信息的暂存
 * 实现下载状态状态的合并
 * 实现下载状态拆分发出
 * 实现三角通信的一个节点，另两个是GUI和aria2实例
 */
public:
    bool checkAndDispatchStat(Aria2StatCollector *sclt);
    bool checkAndDispatchServerStat(Aria2StatCollector *sclt);
    bool confirmBackendFinished(int tid, void *);


public slots:
    void onAriaAddUriResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaAddUriFault(int, QString, QNetworkReply *reply, QVariant &payload);
    void onAriaUpdaterTimeout();

private:
    Aria2JsonRpcClient *mJsonRpc = NULL;
    std::unique_ptr<Karia2StatCalc> statCalc_;
};



///////////////////
class  Aria2JsonRpcClient : public QObject
{
    Q_OBJECT;
public:
    Aria2JsonRpcClient(QString url);
    virtual ~Aria2JsonRpcClient();

    // bool call(QString method, QVariantList arguments);
    bool call(QString method, QVariantList arguments, QVariant payload, 
              QObject* responseObject, const char* responseSlot,
              QObject* faultObject, const char* faultSlot);

protected slots:
    bool onRawSocketConnectError(QAbstractSocket::SocketError socketError);
    bool onRawSocketConnected();
    void onMessageReceived(const QJsonRpcMessage &message);
    void onDisconnectConnection(void *cbmeta);

signals:
    void aresponse(QVariant &, QNetworkReply* reply, QVariant &);
    void fault(int, const QString &, QNetworkReply* reply, QVariant &);
    void disconnectConnection(void *cbmeta);

private:
    QString mUrl;

    struct CallbackMeta {
        QTcpSocket *mRawSock;
        QJsonRpcSocket *mJsonRpcSock;
        QString method;
        QVariantList arguments;
        QVariant payload;
        QObject *responseObject;
        const char *responseSlot;
        QObject *faultObject;
        const char *faultSlot;
    };

    QHash<QTcpSocket*, CallbackMeta*> mCbMeta;
    QHash<QJsonRpcSocket*, CallbackMeta*> mCbMeta2;
};

#endif /* _ARIA2JSONRPCMANAGER_H_ */
