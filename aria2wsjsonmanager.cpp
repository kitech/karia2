
#include "simplelog.h"

#include "aria2wsjsonmanager.h"

Aria2WSJsonManager::Aria2WSJsonManager()
    : Aria2RpcManager()
{
}

Aria2WSJsonManager::~Aria2WSJsonManager()
{
}

void Aria2WSJsonManager::run()
{

}

int Aria2WSJsonManager::addTask(int task_id, const QString &url, TaskOption *to)
{
    QLibwebsockets *qws = new QLibwebsockets();
    qws->connectToHost("", 0);

    return 0;
}

int Aria2WSJsonManager::pauseTask(int task_id)
{
    return 0;
}

/////
bool Aria2WSJsonManager::onAllStatArrived(int stkey)
{
    return true;
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

bool Aria2WSJsonRpcClient::call(QString method, QVariantList arguments, QVariant payload, 
              QObject* responseObject, const char* responseSlot,
              QObject* faultObject, const char* faultSlot)
{
    QTcpSocket *mRawSock = new QTcpSocket();
    QObject::connect(mRawSock, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(onRawSocketConnectError(QAbstractSocket::SocketError)));
    QObject::connect(mRawSock, &QTcpSocket::connected, this, &Aria2WSJsonRpcClient::onRawSocketConnected);

    QUrl uo(this->mUrl);
    mRawSock->connectToHost(uo.host(), uo.port());

    CallbackMeta *meta = new CallbackMeta();
    meta->mRawSock = mRawSock;
    meta->mJsonRpcSock = NULL;
    meta->method = method;
    meta->arguments = arguments;
    meta->payload = payload;
    meta->responseObject = responseObject;
    meta->responseSlot = responseSlot;
    meta->faultObject = faultObject;
    meta->faultSlot = faultSlot;

    this->mCbMeta[mRawSock] = meta;

    qLogx()<<"connecting...";
    // TODO onRawSocketConnectError

    return true;
}

bool Aria2WSJsonRpcClient::onRawSocketConnectError(QAbstractSocket::SocketError socketError)
{
    qLogx()<<socketError;
    QTcpSocket *rsock = (QTcpSocket*)(sender());
    emit this->disconnectConnection(this->mCbMeta[rsock]);

    return true;
}

bool Aria2WSJsonRpcClient::onRawSocketConnected()
{
    qLogx()<<"sending..."<<(sender());
    QTcpSocket *mRawSock = (QTcpSocket*)(sender());
    CallbackMeta *meta = this->mCbMeta[mRawSock];

    QJsonRpcSocket *mJsonRpcSock = new QJsonRpcSocket(mRawSock);
    QObject::connect(mJsonRpcSock, &QJsonRpcSocket::messageReceived, this, &Aria2WSJsonRpcClient::onMessageReceived);
    QObject::connect(this, SIGNAL(aresponse(QVariant &, QNetworkReply *, QVariant &)), 
                     meta->responseObject, meta->responseSlot);
    QObject::connect(this, SIGNAL(fault(int, const QString &, QNetworkReply *, QVariant &)), 
                     meta->faultObject, meta->faultSlot);

    this->mCbMeta2[mJsonRpcSock] = meta;
    meta->mJsonRpcSock = mJsonRpcSock;

    QJsonRpcMessage request = QJsonRpcMessage::createRequest(meta->method, meta->arguments);
    QJsonRpcServiceReply *reply = mJsonRpcSock->sendMessage(request);

    // QJsonRpcMessage response = this->mJsonRpcSock->sendMessageBlocking(request, 5000);
    // qLogx()<<response.result();
    
    return true;
}

void Aria2WSJsonRpcClient::onMessageReceived(const QJsonRpcMessage &message)
{
    qLogx()<<message<<sender();
    CallbackMeta *meta = this->mCbMeta2[(QJsonRpcSocket*)(sender())];

    QVariant result = message.result();
    int errorCode = message.errorCode();
    QString errorMessage = message.errorMessage();

    if (errorCode == 0) {
        emit this->aresponse(result, 0, meta->payload);
    } else {
        emit this->fault(errorCode, errorMessage, 0, meta->payload);
    }   

    emit this->disconnectConnection(meta);
}

void Aria2WSJsonRpcClient::onDisconnectConnection(void *cbmeta)
{
    qLogx()<<cbmeta;

    CallbackMeta *meta = (CallbackMeta*)cbmeta;

    QObject::disconnect(this, SIGNAL(aresponse(QVariant &, QNetworkReply *, QVariant &)), 
                     meta->responseObject, meta->responseSlot);
    QObject::disconnect(this, SIGNAL(fault(int, const QString &, QNetworkReply *, QVariant &)), 
                     meta->faultObject, meta->faultSlot);

    QTcpSocket *rsock = meta->mRawSock;
    QJsonRpcSocket *jsock = meta->mJsonRpcSock;
    this->mCbMeta.remove(rsock);
    if (jsock) this->mCbMeta2.remove(jsock);
    delete meta;
    rsock->deleteLater();
    if (jsock) jsock->deleteLater();
}



///////////////////////
//////////////////////

QLibwebsockets::QLibwebsockets(QObject *parent)
    : QObject(0)
{
    QObject::connect(&this->loop_timer, &QTimer::timeout, this, &QLibwebsockets::onLoopCycle);
}


QLibwebsockets::~QLibwebsockets()
{

}

int QLibwebsockets::wsLoopCallback(struct libwebsocket_context *ctx,
                   struct libwebsocket *wsi,
                   enum libwebsocket_callback_reasons reason,
                   void *user, void *in, size_t len)
{
    qLogx()<<ctx<<wsi<<reason<<user<<in<<len;

    char *http_header;
    char *buff;
    char *rdata = "\"{\"id\": 1,\"jsonrpc\": \"2.0\",\"method\": \"aria2.getVersion\",\"params\": [\"a\"]}\"";
    char *pname = "default";

	switch (reason) {
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		qLogx()<<"callback_dumb_increment: LWS_CALLBACK_CLIENT_ESTABLISHED";
        emit this->connected();

        http_header  = "POST /jsonrpc HTTP/1.0\r\n"
            "Host: 127.0.0.1\r\n"
            "Content-length: %d\r\n"
            "\r\n%s";
        buff = (char*)calloc(1, 1024);
        memset(buff, 0, 1024);
        sprintf(buff, http_header, strlen(rdata), rdata);
        strcpy(buff, rdata);
        qLogx()<<"sending "<<buff;
        libwebsocket_write(wsi, (unsigned char*)buff, strlen(buff), LWS_WRITE_TEXT);
        qLogx()<<"sending "<<buff;

		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		qLogx()<<"LWS_CALLBACK_CLIENT_CONNECTION_ERROR";
		// was_closed = 1;
		break;

	case LWS_CALLBACK_CLOSED:
		qLogx()<<"LWS_CALLBACK_CLOSED";
		// was_closed = 1;
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		((char *)in)[len] = '\0';
        qLogx()<<"rx %d '%s'";
		break;

	/* because we are protocols[0] ... */
	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
        qLogx()<<"not support.";
        return 1;
        /*
		if ((strcmp(in, "deflate-stream") == 0)) {
			qLogx()<<"denied deflate-stream extension";
			return 1;
		}
		if ((strcmp(in, "deflate-frame") == 0)) {
			qLogx()<<"denied deflate-frame extension";
			return 1;
		}
		if ((strcmp(in, "x-google-mux") == 0)) {
			qLogx()<<"denied x-google-mux extension";
			return 1;
		}
        */
		break;
	default:
        qLogx()<<"what reason:"<<reason;
		break;
    }

    return 0;
}

static int QLibwebsockets_loop_callback(struct libwebsocket_context *cthis,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
    QLibwebsockets *qws = (QLibwebsockets*)(user);
    return qws->wsLoopCallback(cthis, wsi, reason, user, in, len);
}

static struct libwebsocket_protocols protocols[] = {
	{
		"default",
		QLibwebsockets_loop_callback, // callback_dumb_increment,
		0,
		20,
	},
	{ NULL, NULL, 0, 0 } /* end */
};


bool QLibwebsockets::connectToHost(QString host, unsigned short port)
{
    lws_set_log_level(LLL_DEBUG | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_CLIENT, NULL);

    struct lws_context_creation_info *lws_ctx_ci = (struct lws_context_creation_info*)calloc(1, sizeof(struct lws_context_creation_info));
    memset(lws_ctx_ci, 0, sizeof(struct lws_context_creation_info));
    lws_ctx_ci->protocols = protocols;

    this->lws_ctx = libwebsocket_create_context(lws_ctx_ci);
    this->h_lws = libwebsocket_client_connect_extended(lws_ctx, "127.0.0.1", 6800, 0,
                                              "/jsonrpc", "127.0.0.1", "127.0.0.1",
                                              protocols[0].name, -1,
                                              this);

    if (!this->loop_timer.isActive()) {
        this->loop_timer.setInterval(300);
        this->loop_timer.start();
    }
    while (false) {
        libwebsocket_service(lws_ctx, 30000);
    }

    qLogx()<<h_lws;

    return true;
}

void QLibwebsockets::onLoopCycle()
{
    if (this->lws_ctx != NULL) {
        int rv = libwebsocket_service(lws_ctx, 0); // 立即返回
    }
}


/*
  libwebsockets的函数前缀，
  lws开始的是比较简单的函数
  libwebsockets开始的是server端函数
  libwebsocket是客户端函数
 */

