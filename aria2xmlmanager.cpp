// aria2xmlmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-27 21:56:32 +0000
// Version: $Id$
// 

#include <QtNetwork>

#include "simplelog.h"
#include "taskinfodlg.h"
#include "karia2statcalc.h"

#include "maiaXmlRpcClient.h"
#include "aria2rpcserver.h"
#include "aria2xmlmanager.h"

Aria2XmlManager::Aria2XmlManager()
    : Aria2RpcManager()
{
}

Aria2XmlManager::~Aria2XmlManager()
{
}

int Aria2XmlManager::addTask(int task_id, const QString &url, TaskOption *to)
{

    // 这个运行状态怎么记录呢
    this->m_tasks[task_id] = 0;
    this->statCalc_.reset(new Karia2StatCalc(task_id, 1000));
    QObject::connect(this->statCalc_.get(), &Karia2StatCalc::progressStat, this, &Aria2XmlManager::onAllStatArrived);

    this->mAriaRpc = new MaiaXmlRpcClient(QUrl(this->mRpcServer->getRpcUri(Aria2RpcServer::AST_XMLRPC)));

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

    this->mAriaRpc->call(aria2RpcMethod, args, QVariant(payload),
                         this, SLOT(onAriaAddUriResponse(QVariant &, QNetworkReply*, QVariant &)),
                         this, SLOT(onAriaAddUriFault(int, QString, QNetworkReply*, QVariant &)));

    if (!this->mAriaUpdater.isActive()) {
        this->mAriaUpdater.setInterval(3000);
        QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
        this->mAriaUpdater.start();
    }
       
    return 0;
}

int Aria2XmlManager::pauseTask(int task_id)
{
    return 0;
}

//////// statqueue members
// TODO 线程需要一直运行处理等待状态，否则不断启动线程会用掉太多的资源。
void Aria2XmlManager::run()
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

bool Aria2XmlManager::checkAndDispatchStat(Aria2StatCollector *sclt)
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

bool Aria2XmlManager::checkAndDispatchServerStat(Aria2StatCollector *sclt)
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

bool Aria2XmlManager::confirmBackendFinished(int tid, void *eaw)
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

bool Aria2XmlManager::onAllStatArrived(int stkey)
{
    Aria2StatCollector *sclt = static_cast<Karia2StatCalc*>(sender())->getNextStat(stkey);
    this->stkeys.enqueue(QPair<int, Aria2StatCollector*>(stkey, sclt));
    if (!this->isRunning()) {
        this->start();
    }
    return true;
}

void Aria2XmlManager::onAriaGetFeatureResponse(QVariant &response, QNetworkReply * reply, QVariant &payload)
{
    qLogx()<<response<<payload;

/*
    this->mAriaRpc->deleteLater(); // can not delete it on it slot, should use deleteLater;
    this->mAriaRpc = 0;

    QVariantMap features = response.toList().at(0).toMap();

    this->mVersionString = features.value("version").toString();

    this->mIVersion = 0;
    QStringList verParts = this->mVersionString.split(".");
    Q_ASSERT(verParts.count() > 0 && verParts.count() <= 4);
    for (int i = verParts.count() - 1 ; i >= 0 ; --i) {
        this->mIVersion |= (verParts.at(i).toInt() << (i * 8));
    }

    this->mFeatures = 0;    
    QVariantList enabledFeatures = features.value("enabledFeatures").toList();
    if (enabledFeatures.contains(QString("BitTorrent"))) {
        this->mFeatures |= FeatureBitTorrent;
    }

    if (enabledFeatures.contains(QString("GZip"))) {
        this->mFeatures |= FeatureGZip;
    }

    if (enabledFeatures.contains(QString("HTTPS"))) {
        this->mFeatures |= FeatureHTTPS;
    }

    if (enabledFeatures.contains(QString("Message Digest"))) {
        this->mFeatures |= FeatureMessageDigest;
    }

    if (enabledFeatures.contains(QString("Metalink"))) {
        this->mFeatures |= FeatureMetalink;
    }

    if (enabledFeatures.contains(QString("XML-RPC"))) {
        this->mFeatures |= FeatureXMLRPC;
    }

    if (enabledFeatures.contains(QString("Async DNS"))) {
        this->mFeatures |= FeatureAsyncDNS;
    }

    if (enabledFeatures.contains(QString("Firefox3 Cookie"))) {
        this->mFeatures |= FeatureFirefox3Cookie;
    }

    // session parser
    if (response.toList().at(1).type() == QVariant::List) {
        QVariantList sessions = response.toList().at(1).toList();
        this->mSessionId = sessions.at(0).toMap().value("sessionId").toString();
    } else {
        // aria2 == 1.8.0, now getSession method. this is the error info.
        QVariantMap sessions = response.toList().at(1).toMap();
        qLogx()<<sessions;
    }
*/
}

void Aria2XmlManager::onAriaGetFeatureFault(int code, QString reason, QNetworkReply * reply, QVariant &payload)
{
    qLogx()<<code<<reason<<payload;
    this->mAriaRpc->deleteLater(); // can not delete it on it slot, should use deleteLater;
    this->mAriaRpc = 0;
}


void Aria2XmlManager::onAriaAddUriResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response<<payload; // why this line cause crash?

    QMap<QString, QVariant> mPayload = payload.toMap();
    int taskId = mPayload["taskId"].toString().toInt();
    QString url = mPayload["url"].toString();
    QString cmd = mPayload["cmd"].toString();

    /*
    this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::aria_gid, response.toString());
    // for new bittorrent
    if (mPayload.contains("indexes")) {
        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::select_file, mPayload["indexes"].toString());
        this->mTaskMan->updateSelectFile(taskId, mPayload.value("indexes").toString());

        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::save_path, mPayload.value("savePath").toString());
        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::file_name, mPayload.value("saveName").toString());
        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::user_cat_id, mPayload.value("userCatId").toString());
    }
    */

//    this->mRunningMap[taskId] = response.toString();
//    // if is torrent, add to torrentMap
//    if (url.toLower().endsWith(".torrent")) {
//        this->mTorrentMap[taskId] = response.toString();
//     }

}
void Aria2XmlManager::onAriaAddUriFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason;
    Q_UNUSED(payload);
}

void Aria2XmlManager::onAriaGetUriResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response;
    Q_UNUSED(payload);
}
void Aria2XmlManager::onAriaGetUriFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason;
    Q_UNUSED(payload);
}

/*
QVariant(QVariantMap, QMap(("bitfield", QVariant(QString, "0000") ) ("completedLength" ,  QVariant(QString, "1769472") ) ("connections" ,  QVariant(QString, "2") ) ("dir" ,  QVariant(QString, "/home/gzleo/karia2-svn") ) ("downloadSpeed" ,  QVariant(QString, "35243") ) ("files" ,  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("index", QVariant(QString, "1") ) ("length" ,  QVariant(QString, "13910775") ) ("path" ,  QVariant(QString, "/home/gzleo/karia2-svn/postgresql-9.0alpha5.tar.bz2") ) ("selected" ,  QVariant(QString, "true") ) ("uris" ,  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("status", QVariant(QString, "used") ) ("uri" ,  QVariant(QString, "http://wwwmaster.postgresql.org/redir/394/h/source/9.0alpha5/postgresql-9.0alpha5.tar.bz2") ) )  ) ,  QVariant(QVariantMap, QMap(("status", QVariant(QString, "used") ) ("uri" ,  QVariant(QString, "http://wwwmaster.postgresql.org/redir/394/h/source/9.0alpha5/postgresql-9.0alpha5.tar.bz2") ) )  ) )  ) ) )  ) )  ) ) ("gid" ,  QVariant(QString, "1") ) ("numPieces" ,  QVariant(QString, "14") ) ("pieceLength" ,  QVariant(QString, "1048576") ) ("status" ,  QVariant(QString, "active") ) ("totalLength" ,  QVariant(QString, "13910775") ) ("uploadLength" ,  QVariant(QString, "0") ) ("uploadSpeed" ,  QVariant(QString, "0") ) )  )
 */


void Aria2XmlManager::onAriaGetStatusResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response<<payload;

    int taskId = payload.toInt();
    QVariantMap sts = response.toMap();

    // this->mTaskMan->onTaskStatusNeedUpdate(taskId, sts);

    // qLogx()<<sts["files"];

    if (sts.contains("bittorrent")) {
        QVariantMap stsbt = sts.value("bittorrent").toMap();
        QVariantList stsTrackers = stsbt.value("announceList").toList();
        // this->mTaskMan->setTrackers(taskId, stsTrackers);
    }

//    if (sts["status"].toString() == QString("complete")) {
//        if (this->mTaskMan->isTorrentTask(taskId)) {
//            this->mTorrentMap.remove(taskId);
//        }
//        this->mRunningMap.remove(taskId);

//        this->onTaskDone(taskId);
//        this->mTaskMan->onPauseTask(taskId); // maybe named clearTask

//        if (this->mRunningMap.count() == 0
//            && this->mainUI.action_Shut_Down_When_Done->isChecked()) {
//            // shutdown now
//        }
//    }

//    if (sts["status"].toString() == "error") {
//        //
//        if (this->mRunningMap.contains(taskId)) {
//            this->mRunningMap.remove(taskId);
//        }
//        if (this->mTorrentMap.contains(taskId)) {
//            this->mTorrentMap.remove(taskId);
//        }

//        this->mTaskMan->onPauseTask(taskId); // maybe named clearTask
//    }
}
void Aria2XmlManager::onAriaGetStatusFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason;

    Q_UNUSED(payload);
}

void Aria2XmlManager::onAriaUpdaterTimeout()
{
    qLogx()<<"timer out update";

    if (this->mAriaRpc == NULL) {
        Q_ASSERT(this->mAriaRpc != NULL);
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

        this->mAriaRpc->call(QString("system.multicall"), gargs, QVariant(taskId),
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

void Aria2XmlManager::onAriaRemoveResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response<<payload;
    int taskId = payload.toInt();
//    this->mRunningMap.remove(taskId);
//    if (this->mTorrentMap.contains(taskId)) {
//        this->mTorrentMap.remove(taskId);
//    }

    // this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::task_status, QString(tr("pause")));
    // this->mTaskMan->onPauseTask(taskId);
}

void Aria2XmlManager::onAriaRemoveFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
    // assert(1 == 2);
}

void Aria2XmlManager::onAriaGlobalUpdaterTimeout()
{
    QVariantList args;
    QVariant payload;

//    if (this->mRunningMap.count() == 0) {
//        // qLogx()<<"No Running task in queue, don't need run global update";
//        return;
//    }
//    this->initXmlRpc();
//    this->mAriaRpc->call(QString("aria2.tellActive"), args, payload,
//                         this, SLOT(onAriaGetActiveResponse(QVariant&, QVariant&)),
//                         this, SLOT(onAriaGetActiveFault(int, QString, QNetworkReply *, QVariant &)));
}

void Aria2XmlManager::onAriaGetActiveResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    // qLogx()<<__FUNCTION__<<response<<payload;
    Q_UNUSED(payload);

    QVariantList lsts = response.toList();
    QVariantMap  sts;
    int speed = 0;
    quint64 totalLength = 0;
    quint64 gotLength = 0;

    for (int i = 0; i < lsts.size(); i++) {
        sts = lsts.at(i).toMap();
        speed += sts["downloadSpeed"].toInt();
        totalLength += sts["totalLength"].toULongLong();
        gotLength += sts["completedLength"].toULongLong();
    }

    // qLogx()<<"TSpeed:"<<speed<<" TLen:"<<totalLength<<" GLen:"<<gotLength;
    double sumSpeed =speed*1.0/1000;
    if (sumSpeed >= 0.0) {
        // this->mDownloadSpeedTotalLable->setText(QString("%1 KB/s").arg(sumSpeed));
        // this->mSpeedProgressBar->setValue((int)sumSpeed);
    } else {
        // this->mDownloadSpeedTotalLable->setText(QString("%1 B/s").arg(speed));
        // this->mSpeedProgressBar->setValue((int)sumSpeed);
    }

    // this->mISHW->updateSpeedHistogram(sumSpeed);
}

void Aria2XmlManager::onAriaGetActiveFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onAriaGetServersResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response<<payload;

    int taskId = payload.toInt();
    /*
    TaskServerModel *serverModel = this->mTaskMan->taskServerModel(taskId);
    if (serverModel) {
        QVariantList servers = response.toList();
        serverModel->setData(servers);
        // QTreeView::rowsInserted internal representation of the model has been corrupted, resetting.
    } else {
        qLogx()<<"server model not found";
    }
    */
}

void Aria2XmlManager::onAriaGetServersFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onAriaGetTorrentPeersResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    int taskId = payload.toInt();

    QVariantList peers = response.toList();
    // this->mTaskMan->setPeers(taskId, peers);
    // qLogx()<<__FUNCTION__<<response<<payload;
}
void Aria2XmlManager::onAriaGetTorrentPeersFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onAriaParseTorrentFileResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response<<payload;
    /*
      onAriaGetTorrentFilesResponse QVariant(QString, "1") QVariant(QVariantMap, QMap(("cmd", QVariant(QString, "torrent_get_files") ) ("taskId" ,  QVariant(int, 42) ) ("url" ,  QVariant(QString, "file:///home/gzleo/NGDownload/1CA79A2AD33A0C89157E2BE71A0AF8A1A735A83A.torrent") ) )  )
     */
    QString ariaGid = response.toString();
    QMap<QString, QVariant> mPayload = payload.toMap();
    int taskId = mPayload["taskId"].toInt();
    QString url = mPayload["url"].toString();
    mPayload["ariaGid"] = ariaGid;
    qLogx()<<__FUNCTION__<<url<<taskId;

    //this->initXmlRpc();

    QVariantList args;
    args.insert(0, ariaGid);

    // now use tellStatus to get files and torrent file info
//    this->mAriaRpc->call(QString("aria2.tellStatus"), args, QVariant(mPayload),
//                         this, SLOT(onAriaGetTorrentFilesResponse(QVariant &, QNetworkReply*, QVariant &)),
//                         this, SLOT(onAriaGetTorrentFilesFault(int, QString, QNetworkReply *, QVariant &)));


}

void Aria2XmlManager::onAriaParseTorrentFileFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
    // onAriaParseTorrentFileFault 1 "We encountered a problem while processing the option '--select-file'."
    /*
      2010-04-21 10:44:24.049116 ERROR - Exception caught
      Exception: [RequestGroup.cc:324] File /home/gzleo/karia2-svn/Kansas - Monolith [1979] exists, but a control file(*.aria2) does not exist. Download was canceled in order to prevent your file from being truncated to 0. If you are sure to download the file all over again, then delete it or add --allow-overwrite=true option and restart aria2.
     */
}

void Aria2XmlManager::onAriaGetTorrentFilesResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    // qLogx()<<__FUNCTION__<<response<<payload;
    QMap<QString, QVariant> mPayload = payload.toMap();
    // int taskId = payload.toMap().value("taskId").toInt();
    QMap<QString, QVariant> statusMap = response.toMap();
    QVariantList files = statusMap["files"].toList(); // response.toList();

    /*
    SeedFilesDialog *fileDlg = new SeedFilesDialog();
    fileDlg->setFiles(files, true);
    fileDlg->setTorrentInfo(statusMap, statusMap.value("bittorrent").toMap());
    int rv = fileDlg->exec();

    if (rv == QDialog::Accepted) {
        // remove the unused aria2 task
        TaskOption *option = NULL;
        option = fileDlg->getOption();
        mPayload["indexes"] = fileDlg->getSelectedFileIndexes();
        mPayload["removeConfirm"] = "no";
        mPayload["savePath"] = option->mSavePath;
        mPayload["saveName"] = option->mSaveName;
        mPayload["userCatId"] = QString::number(option->mCatId);
        // mPayload["taskOption"] = option->toBase64Data();
        delete option; option = NULL;

        QVariantList args;
        args << payload.toMap().value("ariaGid");

//        this->mAriaRpc->call(QString("aria2.remove"), args, QVariant(mPayload),
//                             this, SLOT(onAriaRemoveTorrentParseFileTaskResponse(QVariant &, QVariant&)),
//                             this, SLOT(onAriaRemoveTorrentParseFileTaskFault(int, QString, QVariant&)));

    }
    */
}

void Aria2XmlManager::onAriaGetTorrentFilesFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onTorrentRemoveConfirmTimeout()
{
    QTimer *timer = (QTimer*)(sender());
    QVariant vtimer = QVariant(qVariantFromValue((QObject*)timer));
    timer = (QTimer*)(vtimer.value<QObject*>());
    QVariant payload ;// = this->mTorrentWaitRemoveConfirm[timer];
    QMap<QString, QVariant> mPayload = payload.toMap();

    QString ariaGid = mPayload["ariaGid"].toString();

    QVariantList args;
    args << ariaGid;

//    this->mAriaRpc->call(QString("aria2.tellStatus"), args, payload,
//                         this, SLOT(onAriaRemoveGetTorrentFilesConfirmResponse(QVariant&, QVariant&)),
//                         this, SLOT(onAriaRemoveGetTorrentFilesConfirmFault(int, QString, QNetworkReply *, QVariant &)));
}

void Aria2XmlManager::onAriaRemoveGetTorrentFilesConfirmResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    // qLogx()<<__FUNCTION__<<response<<payload;
    QVariantMap msts = response.toMap();
    QVariantMap mPayload = payload.toMap();

    if (msts.value("status").toString() == "removed"
        || msts.value("status").toString() == "error") {
        mPayload["removeConfirm"] = "yes";
        QVariant aPayload = QVariant(mPayload);
        this->onAriaRemoveTorrentParseFileTaskResponse(response, reply, aPayload);
        // delete no used timer and temporary data
        QTimer *timer = (QTimer*)(mPayload.value("confirmTimer").value<QObject*>());
        QVariant tPayload;// = this->mTorrentWaitRemoveConfirm[timer];
        // this->mTorrentWaitRemoveConfirm.remove(timer);
        delete timer;
    } else {
        QTimer *timer = (QTimer*)(mPayload.value("confirmTimer").value<QObject*>());
        timer->start();
    }
}

void Aria2XmlManager::onAriaRemoveGetTorrentFilesConfirmFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onAriaRemoveTorrentParseFileTaskResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    // qLogx()<<__FUNCTION__<<response<<payload;
    qLogx()<<__FUNCTION__<<payload;

    // insert new torrent task
    QMap<QString, QVariant> mPayload = payload.toMap();
    QString indexList = mPayload["indexes"].toString();
    QString url = mPayload["url"].toString();
    QString removeConfirm = mPayload["removeConfirm"].toString();
    // TaskOption toption = TaskOption::fromBase64Data(mPayload["taskOption"].toString());
    // toption.dump();
    QString savePath = mPayload["savePath"].toString();

    if (removeConfirm != "yes") {
        QTimer *timer = new QTimer(); timer->setSingleShot(true); timer->setInterval(500);
        QObject::connect(timer, SIGNAL(timeout()),
                         this, SLOT(onTorrentRemoveConfirmTimeout()));
        mPayload["confirmTimer"] = qVariantFromValue((QObject*)timer);
        // this->mTorrentWaitRemoveConfirm[timer] = QVariant(mPayload);
        timer->start();
        return;
    }

    //this->initXmlRpc();

    QFile torrentFile(url.right(url.length() - 7));
    torrentFile.open(QIODevice::ReadOnly);
    QByteArray torrentConntent = torrentFile.readAll();
    torrentFile.close();

    QVariantList args;
    QList<QVariant> uris;

    args.insert(0, torrentConntent);
    args.insert(1, uris);

    QMap<QString, QVariant> options;
    // options["split"] = QString("1");
    options["dir"] = savePath; // toption.mSavePath;
    options["select-file"] = indexList;
    args.insert(2, options);
    args.insert(3, QVariant(0));

//    this->mAriaRpc->call(QString("aria2.addTorrent"), args, QVariant(payload),
//                         this, SLOT(onAriaAddUriResponse(QVariant &, QNetworkReply*, QVariant &)),
//                         this, SLOT(onAriaAddUriFault(int, QString, QNetworkReply *, QVariant &)));

//    if (!this->mAriaUpdater.isActive()) {
//        this->mAriaUpdater.setInterval(3000);
//        QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
//        this->mAriaUpdater.start();
//    }

//    if (!this->mAriaTorrentUpdater.isActive()) {
//        this->mAriaTorrentUpdater.setInterval(4000);
//        QObject::connect(&this->mAriaTorrentUpdater, SIGNAL(timeout()), this, SLOT(onAriaTorrentUpdaterTimeout()));
//        this->mAriaTorrentUpdater.start();
//    }

    // set seed file model data
    int taskId = mPayload["taskId"].toString().toInt();
    QVariantList seedFiles = response.toMap().value("files").toList();
    // this->mTaskMan->setSeedFiles(taskId, seedFiles);
}

void Aria2XmlManager::onAriaRemoveTorrentParseFileTaskFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onAriaTorrentUpdaterTimeout()
{
    // this->initXmlRpc();
    QVariantList args;
//    QHashIterator<int, QString> mit(this->mTorrentMap);
//    while(mit.hasNext()) {
//        mit.next();

//        args<<mit.value();
//        this->mAriaRpc->call(QString("aria2.getPeers"), args, QVariant(mit.key()),
//                             this, SLOT(onAriaGetTorrentPeersResponse(QVariant &, QVariant&)),
//                             this, SLOT(onAriaGetTorrentPeersFault(int, QString, QVariant&)));
//    }
}

void Aria2XmlManager::onAriaGetVersionResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response<<payload;
}
void Aria2XmlManager::onAriaGetVersionFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}
void Aria2XmlManager::onAriaGetSessionInfoResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response<<payload;
}
void Aria2XmlManager::onAriaGetSessionInfoFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onAriaMultiCallVersionSessionResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response<<payload;
}

void Aria2XmlManager::onAriaMultiCallVersionSessionFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onAriaChangeGlobalOptionResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    // qLogx()<<__FUNCTION__<<response<<payload;
    Q_UNUSED(response);

    QString which = payload.toString();
    if (which == "max-overall-download-limit") {

    } else if (which == "max-concurrent-downloads") {

    } else if (which == "max-overall-upload-limit") {

    } else {
        Q_ASSERT(1 == 2);
    }

    // for debug, see the change's response result. no use now
    // QVariantList args;
    // this->mAriaRpc->call("aria2.getGlobalOption", args, payload,
    //                      this, SLOT(onAriaGetGlobalOptionResponse(QVariant &, QNetworkReply*, QVariant &)),
    //                      this, SLOT(onAriaGetGlobalOptionFault(int, QString, QNetworkReply *, QVariant &)));
}

void Aria2XmlManager::onAriaChangeGlobalOptionFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onAriaGetGlobalOptionResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<response<<payload;
}

void Aria2XmlManager::onAriaGetGlobalOptionFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{
    qLogx()<<__FUNCTION__<<code<<reason<<payload;
}

void Aria2XmlManager::onAriaTorrentReselectFileMachineResponse(QVariant &response, QNetworkReply *reply, QVariant &payload)
{

}

void Aria2XmlManager::onAriaTorrentReselectFileMachineFault(int code, QString reason, QNetworkReply *reply, QVariant &payload)
{

}
