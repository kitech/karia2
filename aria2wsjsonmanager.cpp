

#include "aria2wsjsonmanager.h"

Aria2WSJsonManager::Aria2WSJsonManager()
    : Aria2RpcManager()
{
}

Aria2WSJsonManager::~Aria2WSJsonManager()
{
}



//////////////////////////
Aria2WSJsonRpcClient::Aria2WSJsonRpcClient(QObject *parent)
    : QObject(0)
{
    lws_get_library_version();
    libwebsocket_create_context(0);
    libwebsocket_client_connect(lws_ctx, 0, 0, 0, 0, 0, 0, 0, 0);
}

Aria2WSJsonRpcClient::~Aria2WSJsonRpcClient()
{

}
/*
  libwebsockets的函数前缀，
  lws开始的是比较简单的函数
  libwebsockets开始的是server端函数
  libwebsocket是客户端函数
 */

