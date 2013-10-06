// aria2xmlmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-27 21:55:36 +0000
// Version: $Id$
// 
#ifndef _ARIA2XMLRPCMANAGER_H_
#define _ARIA2XMLRPCMANAGER_H_

#include <QtNetwork>

#include "aria2rpcmanager.h"

class MaiaXmlRpcClient;

class Aria2XmlManager : public Aria2RpcManager
{
    Q_OBJECT;
public:
    Aria2XmlManager();
    virtual ~Aria2XmlManager();

public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to);
    // virtual int removeTask(int task_id)  = 0;
    // virtual int startTask(int task_id)  = 0;
    // virtual int stopTask(int task_id)  = 0;
    virtual int pauseTask(int task_id);
    /////
    virtual bool onAllStatArrived(int stkey);
    virtual bool setSpeedLimit(int downloadSpeed, int uploadSpeed) {};

public slots:
    void onAriaGetFeatureResponse(QVariant &response, QNetworkReply *, QVariant &payload);
    void onAriaGetFeatureFault(int code, QString reason, QNetworkReply *, QVariant &payload);

private:
    MaiaXmlRpcClient *mAriaRpc;
};

#endif /* _ARIA2XMLRPCMANAGER_H_ */
