#ifndef _ARIA2WSJSONMANAGER_H_
#define _ARIA2WSJSONMANAGER_H_

#include <QtCore>
#include <QtNetwork>

#include "qjsonrpcmessage.h"
#include "qjsonrpcservice.h"

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
    int wsLoopCallback(struct libwebsocket_context *ctx,
                       struct libwebsocket *wsi,
                       enum libwebsocket_callback_reasons reason,
                       void *user, void *in, size_t len);
signals:
    void connected();
    void readyRead();
    void closed();

private slots:
    void onLoopCycle();

private:
    QTimer loop_timer;
    struct libwebsocket_context *lws_ctx;
    struct libwebsocket *h_lws;
};

#endif /* _ARIA2WSJSONMANAGER_H_ */
