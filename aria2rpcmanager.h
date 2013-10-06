// aria2rpcmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-25 21:37:36 +0000
// Version: $Id$
// 


#ifndef _ARIA2RPCMANAGER_H_
#define _ARIA2RPCMANAGER_H_

#include <QtCore>

#include "aria2manager.h"

class MaiaXmlRpcClient;
class Aria2RpcTransport;
class Aria2RpcServer;


/**
 * 定义数据传输层接口
 */
class Aria2RpcManager : public Aria2Manager
{
    Q_OBJECT;
public:
    Aria2RpcManager();
    virtual ~Aria2RpcManager();

public slots: // from user action

protected slots:
    bool initialize();

signals:

public slots: // from aria2c process

public:

public slots: // from internal trigger   

protected:
    Aria2RpcTransport *mTransport;
    MaiaXmlRpcClient *mAriaRpc;
    Aria2RpcServer *mRpcServer;
};

#endif /* _ARIA2RPCMANAGER_H_ */
