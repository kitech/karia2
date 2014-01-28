// aria2wsjsonmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-10-14 13:23:21 +0000
// Version: $Id$
// 

#ifndef _ARIA2WSJSONMANAGER_H_
#define _ARIA2WSJSONMANAGER_H_

#include <memory>

#include <QtCore>
#include <QtNetwork>

#include "qjsonrpcmessage.h"
#include "qjsonrpcservice.h"

#include "libwebsockets.h"

#include "aria2rpcmanager.h"

class Karia2StatCalc;
class Aria2WSJsonRpcClient;
class QLibwebsockets;

class Aria2WSJsonManager : public Aria2RpcManager
{
    Q_OBJECT;
public:
    Aria2WSJsonManager(bool useSsl, QObject *parent = 0);
    virtual ~Aria2WSJsonManager();

    virtual void run();

public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to);
    virtual int pauseTask(int task_id);
    /////
    virtual bool onAllStatArrived(int stkey);
    virtual bool setSpeedLimit(int downloadSpeed, int uploadSpeed) {return true;};

public slots:
    void onAriaAddUriResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaAddUriFault(int, QString, QNetworkReply *reply, QVariant &payload);
    void onAriaUpdaterTimeout();
    void onAriaGetStatusFault(int code, QString reason, QNetworkReply *, QVariant &payload);

private:
    Aria2WSJsonRpcClient *mWSJsonRpc;
    std::unique_ptr<Karia2StatCalc> statCalc_;    
    bool mUseSsl;
};


////////////////
class Aria2WSJsonRpcClient : public QObject
{
    Q_OBJECT;
public:
    Aria2WSJsonRpcClient(QString url, QObject *parent = 0);
    virtual ~Aria2WSJsonRpcClient();

    // bool call(QString method, QVariantList arguments);
    bool call(QString method, QVariantList arguments, QVariant payload, 
              QObject* responseObject, const char* responseSlot,
              QObject* faultObject, const char* faultSlot);

protected slots:
    bool onRawSocketConnectError(QAbstractSocket::SocketError socketError);
    bool onRawSocketConnected();
    void onMessageReceived(QJsonObject message);
    void onDisconnectConnection(void *cbmeta);

signals:
    void aresponse(QVariant &, QNetworkReply* reply, QVariant &);
    void fault(int, const QString &, QNetworkReply* reply, QVariant &);
    void disconnectConnection(void *cbmeta);

private:
    QString mUrl;

    struct CallbackMeta {
        QLibwebsockets *mLws;
        QString method;
        QVariantList arguments;
        QVariant payload;
        QObject *responseObject;
        const char *responseSlot;
        QObject *faultObject;
        const char *faultSlot;
    };

    QHash<QLibwebsockets*, CallbackMeta*> mCbMeta;

private:

};



//////////////
class QLibwebsockets : public QThread
{
    Q_OBJECT;
public:
    QLibwebsockets(QObject *parent = 0);
    virtual ~QLibwebsockets();

    virtual void run();


    bool connectToHost(QString host, unsigned short port);
    bool close();

    bool sendMessage(QJsonRpcMessage message);
    bool sendMessage(QString method, QVariantList arguments);

    int wsLoopCallback(struct libwebsocket_context *ctx,
                       struct libwebsocket *wsi,
                       enum libwebsocket_callback_reasons reason,
                       void *user, void *in, size_t len);

signals:
    void connected();
    void readyRead();
    void closed();
    void messageReceived(QJsonObject message);
    void destroyContext(void *ctx);
                                                        
private slots:
    void onLoopCycle();
    void onDestroyContext(void *ctx);

private:
    QTimer loop_timer;
    struct libwebsocket_context *lws_ctx = 0;
    struct libwebsocket *h_lws = 0;
    bool m_closed = false;
    struct libwebsocket_context *del_ctx = 0;
    QQueue<QByteArray> m_wrq;
};

#endif /* _ARIA2WSJSONMANAGER_H_ */
