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
#include <QtWebSockets>

#include "qjsonrpcmessage.h"
#include "qjsonrpcservice.h"

#include "libwebsockets.h"

#include "aria2rpcmanager.h"

class Karia2StatCalc;
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
    void onAriaAddUriResponse(QJsonObject &response, QNetworkReply *reply, QVariant &payload);
    void onAriaAddUriFault(int, QString, QNetworkReply *reply, QVariant &payload);
    void onAriaUpdaterTimeout();
    void onAriaGetStatusFault(int code, QString reason, QNetworkReply *, QVariant &payload);
    void getAria2ChildStatus(QStringList childs, int tid);
    void onGetAria2ChildStatusFault(int code, QString reason, QNetworkReply *, QVariant &payload);

private slots:
    void onWSClientError(QAbstractSocket::SocketError error);
    void onWSClientConnected();
    void onWSClientDisconnected();
    void onWSMessageReceived(QString jsmessage);
    bool sendMessage(QWebSocket *ws, QString method, QVariantList arguments, QVariant payload);
    void onStatArrived(QJsonObject &response, QNetworkReply *reply, QVariant &payload);

signals:
    void jaresponse(QJsonObject &, QNetworkReply* reply, QVariant &);

private:
    QMap<int, Karia2StatCalc*> statCalcs_;
    QWebSocket *mws = NULL;
    QQueue<QString> mWaitConnectQueue;
    QHash<int, QVariant> mCbMeta;
    bool mUseSsl = false;
    QMap<QString, int> belongsTos;
};

#endif /* _ARIA2WSJSONMANAGER_H_ */
