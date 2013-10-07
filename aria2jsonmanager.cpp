// aria2jsonmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-26 22:36:51 +0000
// Version: $Id$
// 

#include "simplelog.h"
#include "taskinfodlg.h"

#include "karia2statcalc.h"
#include "aria2rpcserver.h"
#include "aria2jsonmanager.h"

Aria2JsonManager::Aria2JsonManager()
    : Aria2RpcManager()
{

}

Aria2JsonManager::~Aria2JsonManager()
{
}

int Aria2JsonManager::addTask(int task_id, const QString &url, TaskOption *to)
{
    Aria2JsonRpcClient *jrpc = new Aria2JsonRpcClient(this->mRpcServer->getRpcUri(Aria2RpcServer::AST_JSONRPC_HTTP));
    this->mJsonRpc = jrpc;

    this->m_tasks[task_id] = 0;
    this->statCalc_.reset(new Karia2StatCalc(task_id, 1000));
    QObject::connect(this->statCalc_.get(), &Karia2StatCalc::progressStat, this, &Aria2JsonManager::onAllStatArrived);

    QMap<QString, QVariant> payload;
    QVariantList args;
    QList<QVariant> uris;
    QMap<QString, QVariant> options;
    QString aria2RpcMethod;

    {
        // create task object first
        payload["taskId"] = QString("%1").arg(task_id);
        payload["url"] = url;
    }

    options["gid"] = this->tid2hex(task_id);
    options["max-download-limit"] = "20000";
    options["max-connection-per-server"] = "3";
    options["min-split-size"] = "1M";

    if (url.startsWith("file://") && url.endsWith(".torrent")) {
        aria2RpcMethod = QString("aria2.addTorrent");
        
        QFile torrentFile(url.right(url.length() - 7));
        torrentFile.open(QIODevice::ReadOnly);
        QByteArray torrentConntent = torrentFile.readAll();
        torrentFile.close();

        args.insert(0, torrentConntent);
        args.insert(1, uris);

        // options["split"] = QString("5");
        options["split"] = to->mSplitCount;
        options["dir"] = to->mSavePath;
        args.insert(2, options);
    } else if (url.startsWith("file://") && url.endsWith(".metalink")) {
        aria2RpcMethod = QString("aria2.addMetalink");
        
        QFile metalinkFile(url.right(url.length() - 7));
        metalinkFile.open(QIODevice::ReadOnly);
        QByteArray metalinkContent = metalinkFile.readAll();
        metalinkFile.close();

        args.insert(0, metalinkContent);
        // options["split"] = QString("5");
        options["split"] = to->mSplitCount;
        options["dir"] = to->mSavePath;
        args.insert(1, options);

    } else {
        aria2RpcMethod = QString("aria2.addUri");
        uris << QString(url);
        args.insert(0, uris);
    
        options["split"] = QString("3");
        // options["split"] = to->mSplitCount;
        options["dir"] = to->mSavePath;
        args.insert(1, options);
    }

    jrpc->call(aria2RpcMethod, args, QVariant(payload),
                         this, SLOT(onAriaAddUriResponse(QVariant &, QNetworkReply*, QVariant &)),
                         this, SLOT(onAriaAddUriFault(int, QString, QNetworkReply*, QVariant &)));

    if (!this->mAriaUpdater.isActive()) {
        this->mAriaUpdater.setInterval(3000);
        QObject::connect(&this->mAriaUpdater, &QTimer::timeout, this, &Aria2JsonManager::onAriaUpdaterTimeout);
        this->mAriaUpdater.start();
    }

    /*
    QString method = "aria2.getVersion";
    QVariantList args;
    args << QString("a");

    jrpc->call(method, args, QVariant(), 
                         this, SLOT(onAriaAddUriResponse(QVariant &, QNetworkReply*, QVariant &)),
                         this, SLOT(onAriaAddUriFault(int, QString, QNetworkReply*, QVariant &)));
    */
    qLogx()<<"called";
    
    return 0;
}

int Aria2JsonManager::pauseTask(int task_id)
{
    
    return 0;
}

//////// statqueue members
// TODO 线程需要一直运行处理等待状态，否则不断启动线程会用掉太多的资源。
void Aria2JsonManager::run()
{
    int stkey;
    Aria2StatCollector *sclt;
    QPair<int, Aria2StatCollector*> elem;
    int tid = -1;
    // Aria2Libaria2Worker *eaw = 0;

    while (!this->stkeys.empty()) {
        elem = this->stkeys.dequeue();
        stkey = elem.first;
        sclt = elem.second;
        tid = sclt->tid;
        // eaw = (Aria2Libaria2Worker*)this->m_tasks[tid];

        qLogx()<<"dispatching stat event:"<<stkey;

        if (stkey < 0) {
            // 任务完成事件
            this->confirmBackendFinished(tid, NULL);
        } else {
            this->checkAndDispatchStat(sclt);
            this->checkAndDispatchServerStat(sclt);
        }

        delete sclt;
    }
}

bool Aria2JsonManager::checkAndDispatchStat(Aria2StatCollector *sclt)
{
    QMap<int, QVariant> stats, stats2; // QVariant可能是整数，小数，或者字符串
    qLogx()<<"";
    // emit this->taskStatChanged(sclt->tid, sclt->totalLength, sclt->completedLength,
    //                            sclt->totalLength == 0 ? 0: (sclt->completedLength*100/ sclt->totalLength),
    //                            sclt->downloadSpeed, sclt->uploadSpeed);

    stats[ng::stat2::task_id] = sclt->tid;
    stats[ng::stat2::total_length] = (qulonglong)sclt->totalLength;
    stats[ng::stat2::completed_length] = (qulonglong)sclt->completedLength;
    stats[ng::stat2::completed_percent] = (int)(sclt->totalLength == 0 ? 0: (sclt->completedLength*100/ sclt->totalLength));
    stats[ng::stat2::download_speed] = sclt->downloadSpeed;
    stats[ng::stat2::upload_speed] = sclt->uploadSpeed;
    stats[ng::stat2::gid] = (qulonglong)sclt->gid;
    stats[ng::stat2::num_connections] = sclt->connections;
    stats[ng::stat2::hex_bitfield] = QString(sclt->bitfield.c_str());
    stats[ng::stat2::num_pieces] = sclt->numPieces;
    stats[ng::stat2::piece_length] = sclt->pieceLength;
    stats[ng::stat2::eta] = sclt->eta;
    stats[ng::stat2::str_eta] = ""; //QString::fromStdString(aria2::util::secfmt(sclt->eta));
    stats[ng::stat2::error_code] = sclt->errorCode;
    stats[ng::stat2::status] = sclt->state == aria2::DOWNLOAD_ACTIVE ? "active" : "waiting"; 
    // ready, active, waiting, complete, removed, error, pause
    
    emit this->taskStatChanged(sclt->tid, stats);

    stats2[ng::stat2::download_speed] = sclt->globalStat2.downloadSpeed;
    stats2[ng::stat2::upload_speed] = sclt->globalStat2.uploadSpeed;
    emit this->globalStatChanged(stats2);

    return true;
}

bool Aria2JsonManager::checkAndDispatchServerStat(Aria2StatCollector *sclt)
{
    QList<QMap<QString, QString> > servers;
    QMap<QString, QString> server;

    for (int i = 0; i < sclt->server_stats.servers.size(); i++) {
        server["index"] = QString("%1,%2").arg(i).arg(sclt->server_stats.servers.at(i).state);
        server["currentUri"] = sclt->server_stats.servers.at(i).uri.c_str();
        server["downloadSpeed"] = QString("%1").arg(sclt->server_stats.servers.at(i).downloadSpeed);

        servers.append(server);
        server.clear();
    }

    emit this->taskServerStatChanged(sclt->tid, servers);
}

bool Aria2JsonManager::confirmBackendFinished(int tid, void *eaw)
{
    QMap<int, QVariant> stats; // QVariant可能是整数，小数，或者字符串

    // aria2::GroupId::clear();

    /*
    switch(eaw->exit_status) {
    case aria2::error_code::FINISHED:
        emit this->taskFinished(eaw->m_tid, eaw->exit_status);

        this->m_tasks.remove(eaw->m_tid);
        this->m_rtasks.remove(eaw);
        eaw->deleteLater();    
        break;
    case aria2::error_code::IN_PROGRESS:
        stats[ng::stat2::error_code] = eaw->exit_status;
        stats[ng::stat2::error_string] = "";// QString::fromStdString(aria2::fmt(MSG_DOWNLOAD_NOT_COMPLETE, tid, ""));
        stats[ng::stat2::status] = "pause";
        emit this->taskStatChanged(eaw->m_tid, stats);
        this->m_tasks.remove(eaw->m_tid);
        this->m_rtasks.remove(eaw);
        eaw->deleteLater();
        break;
    case aria2::error_code::REMOVED:
        break;
    case aria2::error_code::RESOURCE_NOT_FOUND:
        stats[ng::stat2::error_code] = eaw->exit_status;
        stats[ng::stat2::error_string] = "";// QString(MSG_RESOURCE_NOT_FOUND);
        stats[ng::stat2::status] = "error";
        emit this->taskStatChanged(eaw->m_tid, stats);
        this->m_tasks.remove(eaw->m_tid);
        this->m_rtasks.remove(eaw);
        eaw->deleteLater();
        break;
    default:
        break;
    }
    */
    return true;
}

bool Aria2JsonManager::onAllStatArrived(int stkey)
{
    Aria2StatCollector *sclt = static_cast<Karia2StatCalc*>(sender())->getNextStat(stkey);
    this->stkeys.enqueue(QPair<int, Aria2StatCollector*>(stkey, sclt));
    if (!this->isRunning()) {
        this->start();
    }
    return true;
}

////////
void Aria2JsonManager::onAriaAddUriResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<response<<reply<<payload;
}

void Aria2JsonManager::onAriaAddUriFault(int errorCode, QString errorString, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<errorCode<<reply<<payload;
}

void Aria2JsonManager::onAriaUpdaterTimeout()
{
    qLogx()<<"timer out update";

    if (this->mJsonRpc == NULL) {
        Q_ASSERT(this->mJsonRpc != NULL);
    }

    QVariantList args;
    int taskId;
    QString ariaGid;

    QHashIterator<int, void*> hit(this->m_tasks);
    
    while (hit.hasNext()) {
        hit.next();
        taskId = hit.key();
        ariaGid = this->tid2hex(taskId);

        QVariantMap  tellStatusMethod;
        QVariantMap getServersMethod;
        QVariantMap getGlobalStatMethod;
        QVariantList gargs;
        QVariantList args;
        QVariantMap options;
        QVariantList loptions;

        options[0] = ariaGid;
        loptions << ariaGid;

        tellStatusMethod["methodName"] = QString("aria2.tellStatus");
        tellStatusMethod["params"] = loptions;
        getServersMethod["methodName"] = QString("aria2.getServers");
        getServersMethod["params"] = loptions;
        getGlobalStatMethod["methodName"] = QString("aria2.getGlobalStat");
        getGlobalStatMethod["params"] = QVariant(""); // Failed to marshal unknown variant type:  QVariant::Invalid


        args << tellStatusMethod;
        args << getServersMethod;
        args << getGlobalStatMethod;

        gargs.insert(0, args);

        this->mJsonRpc->call(QString("system.multicall"), gargs, QVariant(taskId),
                             this->statCalc_.get(), SLOT(calculateStat(QVariant&, QNetworkReply*, QVariant&)),
                             this, SLOT(onAriaGetStatusFault(int, QString, QNetworkReply*, QVariant &)));

        /*
        args<<ariaGid;
        this->mAriaRpc->call(QString("aria2.tellStatus"), args, QVariant(taskId),
                             this, SLOT(onAriaGetStatusResponse(QVariant &, QNetworkReply*, QVariant &)),
                             this, SLOT(onAriaGetStatusFault(int, QString, QNetworkReply *, QVariant &)));
        args.clear();

        //
        args<<ariaGid;
        this->mAriaRpc->call(QString("aria2.getServers"), args, QVariant(taskId),
                             this, SLOT(onAriaGetServersResponse(QVariant &, QNetworkReply*, QVariant &)),
                             this, SLOT(onAriaGetServersFault(int, QString, QNetworkReply *, QVariant &)));
        args.clear();
        */
    }

    /*
    // get version and session
    // use aria2's multicall method
    QVariantMap  getVersion;
    QVariantMap getSession;
    QVariantList gargs;
    QVariantList args;
    QVariantMap options;
    QVariant payload;
        
    getVersion["methodName"] = QString("aria2.getVersion");
    getVersion["params"] = QVariant(options);
    getSession["methodName"] = QString("aria2.getSessionInfo");
    getSession["params"] = QVariant(options);

    args.insert(0, getVersion);
    args.insert(1, getSession);

    gargs.insert(0, args);

    this->mAriaRpc->call(QString("system.multicall"), gargs, payload,
                         this, SLOT(onAriaGetFeatureResponse(QVariant&, QVariant&)),
                         this, SLOT(onAriaGetFeatureFault(int, QString, QVariant &)));
*/
    // QVariantList args;
    // int taskId;
    // QString ariaGid;

    //    QHashIterator<int, QString> hit(this->mRunningMap);
//    while (hit.hasNext()) {
//        hit.next();
//        taskId = hit.key();
//        ariaGid = hit.value();

//        args<<ariaGid;
//        this->mAriaRpc->call(QString("aria2.tellStatus"), args, QVariant(taskId),
//                             this, SLOT(onAriaGetStatusResponse(QVariant &, QNetworkReply*, QVariant &)),
//                             this, SLOT(onAriaGetStatusFault(int, QString, QNetworkReply *, QVariant &)));
//        args.clear();

//        //
//        args<<ariaGid;
//        this->mAriaRpc->call(QString("aria2.getServers"), args, QVariant(taskId),
//                             this, SLOT(onAriaGetServersResponse(QVariant &, QNetworkReply*, QVariant &)),
//                             this, SLOT(onAriaGetServersFault(int, QString, QNetworkReply *, QVariant &)));
//        args.clear();
//    }
}

////////////////
Aria2JsonRpcClient::Aria2JsonRpcClient(QString url)
    : QObject(0)
{
    this->mUrl = url;
    QObject::connect(this, &Aria2JsonRpcClient::disconnectConnection, this, &Aria2JsonRpcClient::onDisconnectConnection);
}

Aria2JsonRpcClient::~Aria2JsonRpcClient()
{
}

bool Aria2JsonRpcClient::call(QString method, QVariantList arguments, QVariant payload, 
              QObject* responseObject, const char* responseSlot,
              QObject* faultObject, const char* faultSlot)
{
    QTcpSocket *mRawSock = new QTcpSocket();
    QObject::connect(mRawSock, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(onRawSocketConnectError(QAbstractSocket::SocketError)));
    QObject::connect(mRawSock, &QTcpSocket::connected, this, &Aria2JsonRpcClient::onRawSocketConnected);

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

bool Aria2JsonRpcClient::onRawSocketConnectError(QAbstractSocket::SocketError socketError)
{
    qLogx()<<socketError;
    QTcpSocket *rsock = (QTcpSocket*)(sender());
    emit this->disconnectConnection(this->mCbMeta[rsock]);

    return true;
}

bool Aria2JsonRpcClient::onRawSocketConnected()
{
    qLogx()<<"sending..."<<(sender());
    QTcpSocket *mRawSock = (QTcpSocket*)(sender());
    CallbackMeta *meta = this->mCbMeta[mRawSock];

    QJsonRpcSocket *mJsonRpcSock = new QJsonRpcSocket(mRawSock);
    QObject::connect(mJsonRpcSock, &QJsonRpcSocket::messageReceived, this, &Aria2JsonRpcClient::onMessageReceived);
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

void Aria2JsonRpcClient::onMessageReceived(const QJsonRpcMessage &message)
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

void Aria2JsonRpcClient::onDisconnectConnection(void *cbmeta)
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



