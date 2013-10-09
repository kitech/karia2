#ifndef _ARIA2WSJSONMANAGER_H_
#define _ARIA2WSJSONMANAGER_H_

#include <QtCore>
#include <QtNetwork>

#include "libwebsockets.h"

#include "aria2rpcmanager.h"

class Aria2WSJsonRpcClient;
class QLibwebsockets;

class Aria2WSJsonManager : public Aria2RpcManager
{
    Q_OBJECT;
public:
    Aria2WSJsonManager();
    virtual ~Aria2WSJsonManager();

    virtual void run();

public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to);
    virtual int pauseTask(int task_id);
    /////
    virtual bool onAllStatArrived(int stkey);
    virtual bool setSpeedLimit(int downloadSpeed, int uploadSpeed) {return true;};

private:
    
};


////////////////
class Aria2WSJsonRpcClient : public QObject
{
    Q_OBJECT;
public:
    Aria2WSJsonRpcClient(QObject *parent = 0);
    virtual ~Aria2WSJsonRpcClient();

private:
    struct libwebsocket_context *lws_ctx;
    struct libwebsocket *h_lws;
};



//////////////
class QLibwebsockets : public QObject
{
    Q_OBJECT;
public:
    QLibwebsockets(QObject *parent = 0);
    virtual ~QLibwebsockets();

    bool connectToHost(QString host, unsigned short port);

signals:
    void connected();
    void readyRead();
    void closed();

private:
    struct libwebsocket_context *lws_ctx;
    struct libwebsocket *h_lws;
};

#endif /* _ARIA2WSJSONMANAGER_H_ */
