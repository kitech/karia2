
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


///////////////////////
//////////////////////

QLibwebsockets::QLibwebsockets(QObject *parent)
    : QObject(0)
{

}


QLibwebsockets::~QLibwebsockets()
{

}

static int
callback_dumb_increment(struct libwebsocket_context *cthis,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
    qLogx()<<cthis<<wsi<<reason<<user<<in<<len;
	switch (reason) {
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		qLogx()<<"callback_dumb_increment: LWS_CALLBACK_CLIENT_ESTABLISHED";
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

static struct libwebsocket_protocols protocols[] = {
	{
		"default",
		callback_dumb_increment,
		0,
		20,
	},
	{
		"lws-mirror-protocol",
		NULL, // callback_lws_mirror,
		0,
		128,
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
    this->h_lws = libwebsocket_client_connect(lws_ctx, "127.0.0.1", 6800, 0,
                                              "/jsonrpc", "127.0.0.1", "127.0.0.1",
                                              protocols[0].name, -1);

    while (true) {
        libwebsocket_service(lws_ctx, 30000);
    }

    qLogx()<<h_lws;

    return true;
}



/*
  libwebsockets的函数前缀，
  lws开始的是比较简单的函数
  libwebsockets开始的是server端函数
  libwebsocket是客户端函数
 */

