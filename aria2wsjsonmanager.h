#ifndef _ARIA2WSJSONMANAGER_H_
#define _ARIA2WSJSONMANAGER_H_

#include <QtCore>
#include <QtNetwork>

#include "libwebsockets.h"

#include "aria2rpcmanager.h"

class Aria2WSJsonManager : public Aria2RpcManager
{
    Q_OBJECT;
public:
    Aria2WSJsonManager();
    virtual ~Aria2WSJsonManager();

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

#endif /* _ARIA2WSJSONMANAGER_H_ */
