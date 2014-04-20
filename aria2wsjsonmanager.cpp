// aria2wsjsonmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-10-14 13:23:16 +0000
// Version: $Id$
// 

#include "simplelog.h"

#include "taskqueue.h"
#include "taskinfodlg.h"
#include "karia2statcalc.h"

#include "aria2rpcserver.h"
#include "aria2wsjsonmanager.h"

Aria2WSJsonManager::Aria2WSJsonManager(bool useSsl, QObject *parent)
    : Aria2RpcManager()
{
    this->mUseSsl = useSsl;
    QObject::connect(&this->mAriaUpdater, &QTimer::timeout, this, &Aria2WSJsonManager::onAriaUpdaterTimeout);
    QObject::connect(this, &Aria2WSJsonManager::jaresponse, this, &Aria2WSJsonManager::onStatArrived);
}

Aria2WSJsonManager::~Aria2WSJsonManager()
{
}

int Aria2WSJsonManager::addTask(int task_id, const QString &url, TaskOption *to)
{
    if (mws == NULL) {
        mws = new QWebSocket();
        QObject::connect(mws, &QWebSocket::connected, this, &Aria2WSJsonManager::onWSClientConnected);
        QObject::connect(mws, &QWebSocket::disconnected, this, &Aria2WSJsonManager::onWSClientDisconnected);
        QObject::connect(mws, &QWebSocket::textMessageReceived, this, &Aria2WSJsonManager::onWSMessageReceived);
        QObject::connect(mws, SIGNAL(error(QAbstractSocket::SocketError)),
                         this, SLOT(onWSClientError(QAbstractSocket::SocketError)));

        QString wsuri = this->mRpcServer->getRpcUri(Aria2RpcServer::AST_JSONRPC_WS);
        mws->open(wsuri);
    }

    Karia2StatCalc* sc = NULL;
    this->m_tasks[task_id] = 0;
    sc = new Karia2StatCalc(task_id, 1000);
    QObject::connect(sc, &Karia2StatCalc::progressStat, this, &Aria2WSJsonManager::onAllStatArrived);
    statCalcs_.insert(task_id, sc);

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

    this->sendMessage(mws, aria2RpcMethod, args, QVariant(payload));
    /*
    jrpc->call(aria2RpcMethod, args, QVariant(payload),
               this, SLOT(onAriaAddUriResponse(QJsonObject&, QNetworkReply*, QVariant &)),
               this, SLOT(onAriaAddUriFault(int, QString, QNetworkReply*, QVariant &)));
    */
    if (!this->mAriaUpdater.isActive()) {
        this->mAriaUpdater.setInterval(3000);
        // QObject::connect(&this->mAriaUpdater, &QTimer::timeout, this, &Aria2WSJsonManager::onAriaUpdaterTimeout);
        this->mAriaUpdater.start();
    }

    return 0;
}

int Aria2WSJsonManager::pauseTask(int task_id)
{
    return 0;
}

//////// statqueue members
// TODO 线程需要一直运行处理等待状态，否则不断启动线程会用掉太多的资源。
void Aria2WSJsonManager::run()
{
    int stkey;
    Aria2StatCollector *sclt;
    QPair<int, Aria2StatCollector*> elem;
    int tid = -1;

    while (!this->stkeys.empty()) {
        this->stkLocker.lock();
        elem = this->stkeys.dequeue();
        this->stkLocker.unlock();

        stkey = elem.first;
        sclt = elem.second;
        tid = sclt->tid;

        if (this->belongsTos.contains(sclt->strGid)) {
            sclt->strBelongsTo = QString("%1").arg(tid);
        }
        qLogx()<<"dispatching stat event:"<<stkey<<tid;

        if (stkey < 0) {
            // 任务完成事件
            this->confirmBackendFinished(tid, NULL);
        } else {
            this->checkAndDispatchStat(sclt);
            this->checkAndDispatchServerStat(sclt);
        }

        // 子任务关系
        if (sclt->strFollowedBy.size() > 0) {
            for (int i = 0; i < sclt->strFollowedBy.size(); i++) {
                if (!this->belongsTos.contains(sclt->strFollowedBy.at(i))) {
                    this->belongsTos[sclt->strFollowedBy.at(i)] = tid;
                }
            }
            this->getAria2ChildStatus(sclt->strFollowedBy, tid);
        }

        delete sclt;
    }
    
}

bool Aria2WSJsonManager::checkAndDispatchStat(Aria2StatCollector *sclt)
{
    QMap<int, QVariant> stats, stats2; // QVariant可能是整数，小数，或者字符串
    // qLogx()<<"";
    // emit this->taskStatChanged(sclt->tid, sclt->totalLength, sclt->completedLength,
    //                            sclt->totalLength == 0 ? 0: (sclt->completedLength*100/ sclt->totalLength),
    //                            sclt->downloadSpeed, sclt->uploadSpeed);

    stats[ng::stat2::task_id] = sclt->tid;
    stats[ng::stat2::belongs_to] = sclt->strBelongsTo;
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

bool Aria2WSJsonManager::checkAndDispatchServerStat(Aria2StatCollector *sclt)
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

    if (servers.size() > 0) {
        emit this->taskServerStatChanged(sclt->tid, servers);
    }

    return true;
}

bool Aria2WSJsonManager::confirmBackendFinished(int tid, void *eaw)
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

/////
bool Aria2WSJsonManager::onAllStatArrived(int stkey)
{
    Aria2StatCollector *sclt = static_cast<Karia2StatCalc*>(sender())->getNextStat(stkey);
    QPair<int, Aria2StatCollector*> stval(stkey, sclt);

    this->stkLocker.lock();
    this->stkeys.enqueue(stval);
    this->stkLocker.unlock();

    if (!this->isRunning()) {
        this->start();
    }
    
    return true;
}

////////
void Aria2WSJsonManager::onAriaAddUriResponse(QJsonObject &response, QNetworkReply *reply, QVariant &payload)
{
    // qLogx()<<response<<reply<<payload;
}

void Aria2WSJsonManager::onAriaAddUriFault(int errorCode, QString errorString, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<errorCode<<reply<<payload;
}

void Aria2WSJsonManager::onAriaUpdaterTimeout()
{
    // qLogx()<<"timer out update";
    QVariantList args;
    int taskId;
    QString ariaGid;
    Karia2StatCalc *sc = NULL;

    qLogx()<<m_tasks.size()<<QDateTime::currentDateTime();
    QHashIterator<int, void*> hit(this->m_tasks);
    
    while (hit.hasNext()) {
        hit.next();
        taskId = hit.key();
        ariaGid = this->tid2hex(taskId);
        sc = statCalcs_[taskId];

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

        this->sendMessage(mws, QString("system.multicall"), gargs, QVariant(taskId));
        /*
        jrpc->call(QString("system.multicall"), gargs, QVariant(taskId),
                   sc, SLOT(calculateStat(QJsonObject&, QNetworkReply*, QVariant&)),
                   this, SLOT(onAriaGetStatusFault(int, QString, QNetworkReply*, QVariant &)));
        */
    }

}
void Aria2WSJsonManager::onAriaGetStatusFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<code<<reason<<payload;
}

void Aria2WSJsonManager::getAria2ChildStatus(QStringList childs, int tid)
{
    QVariantList args;
    int taskId;
    QString ariaGid;
    Karia2StatCalc *sc = NULL;
    // Aria2WSJsonRpcClient *jrpc = NULL;

    taskId = tid;
    sc = statCalcs_[tid];
    // jrpc = mWSJsonRpcs[tid];
    // qLogx()<<jrpc<<tid<<mWSJsonRpcs.size()<<childs<<m_tasks.size()<<sender()<<QDateTime::currentDateTime();

    if (true) {
        QVariantMap  tellStatusMethod;
        QVariantMap getServersMethod;
        QVariantMap getGlobalStatMethod;
        QVariantList gargs;
        QVariantList args;
        QVariantMap options;
        QVariantList loptions;

        for (int i = 0; i < childs.count() ; i ++) {
            ariaGid = childs.at(i);
            loptions << ariaGid;
        }

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

        this->sendMessage(mws, QString("system.multicall"), gargs, QVariant(taskId));
        /*
        jrpc->call(QString("system.multicall"), gargs, QVariant(taskId),
                   sc, SLOT(calculateStat(QJsonObject&, QNetworkReply*, QVariant&)),
                   this, SLOT(onGetAria2ChildStatusFault(int, QString, QNetworkReply*, QVariant &)));
        */
    }
}

void Aria2WSJsonManager::onGetAria2ChildStatusFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<code<<reason<<payload;
}

void Aria2WSJsonManager::onWSClientError(QAbstractSocket::SocketError error)
{
    qLogx()<<error;
}

void Aria2WSJsonManager::onWSClientConnected()
{
    qLogx()<<"";
    QString call_msg;
    qint64 slen;

    while (!this->mWaitConnectQueue.empty()) {
        call_msg = this->mWaitConnectQueue.dequeue();
        slen = this->mws->sendTextMessage(call_msg);
    }
}

void Aria2WSJsonManager::onWSClientDisconnected()
{
    qLogx()<<"";
}

void Aria2WSJsonManager::onWSMessageReceived(QString jsmessage)
{
    qLogx()<<""<<jsmessage.length()<<jsmessage;
    QJsonDocument jdoc = QJsonDocument::fromJson(jsmessage.toLatin1());
    QJsonObject jmessage = jdoc.object();
    int taskId;
    QVariant payload;

    if (jmessage.contains("id") &&jmessage.contains("result")
        && jmessage.value("result").isArray()) {

        taskId = mCbMeta.value(jmessage.value("id").toInt()).toInt();
        payload = QVariant(taskId);
        emit this->jaresponse(jmessage, 0, payload);
        mCbMeta.remove(jmessage.value("id").toInt());
    } else {
        // notify event
    }

    /*
    QJsonRpcMessage message(jmessage);
    QVariant result = message.result();
    int errorCode = message.errorCode();
    QString errorMessage = message.errorMessage();

    if (errorCode == 0) {
        // emit this->jaresponse(jmessage, 0, meta->payload);
    } else {
        // emit this->fault(errorCode, errorMessage, 0, meta->payload);
    } 
    */  
}

bool Aria2WSJsonManager::sendMessage(QWebSocket *ws, QString method, QVariantList arguments, QVariant payload)
{
    QJsonObject *jobj = new QJsonObject;
    jobj->insert("jsonrpc", QLatin1String("2.0"));
    jobj->insert("method", method);
    if (!arguments.isEmpty()) {
        jobj->insert("params", QJsonArray::fromVariantList(arguments));
    }

    static int json_id = 0;
    jobj->insert("id", ++ json_id);
    mCbMeta[json_id] = payload;

    QJsonDocument jdoc = QJsonDocument(*jobj);
    jdoc.toJson(QJsonDocument::Compact);
    QString call_msg = jdoc.toJson(QJsonDocument::Compact);

    if (ws->isValid()) {
        qint64 ret = ws->sendTextMessage(call_msg);
    } else {
        this->mWaitConnectQueue.enqueue(call_msg);
    }

    return true;
}

void Aria2WSJsonManager::onStatArrived(QJsonObject &response, QNetworkReply *reply, QVariant &payload)
{
    int taskId;
    Karia2StatCalc *sch = NULL;

    taskId = payload.toInt();
    if (!statCalcs_.contains(taskId)) {
        Q_ASSERT(1 == 2);
        return;
    }

    sch = statCalcs_.value(taskId);
    sch->calculateStat(response, reply, payload);
}


