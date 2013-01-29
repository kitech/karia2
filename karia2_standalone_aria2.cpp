// karia2.cpp ---
//
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL:
// Created: 2010-04-03 22:27:02 +0800
// Version: $Id: karia2.cpp 183 2011-03-29 12:32:01Z drswinghead $
//

#include <QtCore>
#include <QtGui>

#include "utility.h"
#include "karia2.h"
#include "aboutdialog.h"
#include "dropzone.h"
#include "taskinfodlg.h"

#include "sqlitecategorymodel.h"
#include "sqlitestorage.h"
#include "sqlitetaskmodel.h"
#include "sqlitesegmentmodel.h"
#include "segmentlogmodel.h"
#include "taskitemdelegate.h"
#include "optionmanager.h"

#include "catmandlg.h"
#include "catpropdlg.h"
#include "preferencesdialog.h"

#include "batchjobmandlg.h"
#include "webpagelinkdlg.h"
#include "taskqueue.h"

#include "taskballmapwidget.h"
#include "instantspeedhistogramwnd.h"
#include "walksitewndex.h"

#include "torrentpeermodel.h"
#include "taskservermodel.h"
#include "seedfilemodel.h"
#include "seedfilesdialog.h"

//////
#include "libng/html-parse.h"

//labspace
#include "labspace.h"

#if !defined(Q_OS_WIN32)
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#endif

//#include "ariaman.h"
//#include "maiaXmlRpcClient.h"


void Karia2::testResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}
void Karia2::testFault(int status, QString response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<status<<response<<payload;
}

void Karia2::onAriaAddUriResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload; // why this line cause crash?

    QMap<QString, QVariant> mPayload = payload.toMap();
    int taskId = mPayload["taskId"].toString().toInt();
    QString url = mPayload["url"].toString();
    QString cmd = mPayload["cmd"].toString();

    this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::aria_gid, response.toString());
    // for new bittorrent
    if (mPayload.contains("indexes")) {
        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::select_file, mPayload["indexes"].toString());
        this->mTaskMan->updateSelectFile(taskId, mPayload.value("indexes").toString());

        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::save_path, mPayload.value("savePath").toString());
        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::file_name, mPayload.value("saveName").toString());
        this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::user_cat_id, mPayload.value("userCatId").toString());
    }

//    this->mRunningMap[taskId] = response.toString();
//    // if is torrent, add to torrentMap
//    if (url.toLower().endsWith(".torrent")) {
//        this->mTorrentMap[taskId] = response.toString();
//     }

}
void Karia2::onAriaAddUriFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason;
    Q_UNUSED(payload);
}

void Karia2::onAriaGetUriResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response;
    Q_UNUSED(payload);
}
void Karia2::onAriaGetUriFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason;
    Q_UNUSED(payload);
}

/*
QVariant(QVariantMap, QMap(("bitfield", QVariant(QString, "0000") ) ("completedLength" ,  QVariant(QString, "1769472") ) ("connections" ,  QVariant(QString, "2") ) ("dir" ,  QVariant(QString, "/home/gzleo/karia2-svn") ) ("downloadSpeed" ,  QVariant(QString, "35243") ) ("files" ,  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("index", QVariant(QString, "1") ) ("length" ,  QVariant(QString, "13910775") ) ("path" ,  QVariant(QString, "/home/gzleo/karia2-svn/postgresql-9.0alpha5.tar.bz2") ) ("selected" ,  QVariant(QString, "true") ) ("uris" ,  QVariant(QVariantList, (QVariant(QVariantMap, QMap(("status", QVariant(QString, "used") ) ("uri" ,  QVariant(QString, "http://wwwmaster.postgresql.org/redir/394/h/source/9.0alpha5/postgresql-9.0alpha5.tar.bz2") ) )  ) ,  QVariant(QVariantMap, QMap(("status", QVariant(QString, "used") ) ("uri" ,  QVariant(QString, "http://wwwmaster.postgresql.org/redir/394/h/source/9.0alpha5/postgresql-9.0alpha5.tar.bz2") ) )  ) )  ) ) )  ) )  ) ) ("gid" ,  QVariant(QString, "1") ) ("numPieces" ,  QVariant(QString, "14") ) ("pieceLength" ,  QVariant(QString, "1048576") ) ("status" ,  QVariant(QString, "active") ) ("totalLength" ,  QVariant(QString, "13910775") ) ("uploadLength" ,  QVariant(QString, "0") ) ("uploadSpeed" ,  QVariant(QString, "0") ) )  )
 */


void Karia2::onAriaGetStatusResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;

    int taskId = payload.toInt();
    QVariantMap sts = response.toMap();

    this->mTaskMan->onTaskStatusNeedUpdate(taskId, sts);

    // qDebug()<<sts["files"];

    if (sts.contains("bittorrent")) {
        QVariantMap stsbt = sts.value("bittorrent").toMap();
        QVariantList stsTrackers = stsbt.value("announceList").toList();
        this->mTaskMan->setTrackers(taskId, stsTrackers);
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
void Karia2::onAriaGetStatusFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason;

    Q_UNUSED(payload);
}

// TODO, combine request to aria2.multicall
void Karia2::onAriaUpdaterTimeout()
{
    // qDebug()<<"time out update";
//    QHashIterator<int, QString> hit(this->mRunningMap);

//    if (this->mAriaRpc == NULL) {
//        Q_ASSERT(this->mAriaRpc != NULL);
//    }
    QVariantList args;
    int taskId;
    QString ariaGid;

//    while (hit.hasNext()) {
//        hit.next();
//        taskId = hit.key();
//        ariaGid = hit.value();

//        args<<ariaGid;
//        this->mAriaRpc->call(QString("aria2.tellStatus"), args, QVariant(taskId),
//                             this, SLOT(onAriaGetStatusResponse(QVariant &, QVariant &)),
//                             this, SLOT(onAriaGetStatusFault(int, QString, QVariant &)));
//        args.clear();

//        //
//        args<<ariaGid;
//        this->mAriaRpc->call(QString("aria2.getServers"), args, QVariant(taskId),
//                             this, SLOT(onAriaGetServersResponse(QVariant &, QVariant &)),
//                             this, SLOT(onAriaGetServersFault(int, QString, QVariant &)));
//        args.clear();
//    }
}

void Karia2::onAriaRemoveResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
    int taskId = payload.toInt();
//    this->mRunningMap.remove(taskId);
//    if (this->mTorrentMap.contains(taskId)) {
//        this->mTorrentMap.remove(taskId);
//    }

    this->mTaskMan->onTaskListCellNeedChange(taskId, ng::tasks::task_status, QString(tr("pause")));
    this->mTaskMan->onPauseTask(taskId);
}

void Karia2::onAriaRemoveFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
    // assert(1 == 2);
}

void Karia2::onAriaGlobalUpdaterTimeout()
{
    QVariantList args;
    QVariant payload;

//    if (this->mRunningMap.count() == 0) {
//        // qDebug()<<"No Running task in queue, don't need run global update";
//        return;
//    }
//    this->initXmlRpc();
//    this->mAriaRpc->call(QString("aria2.tellActive"), args, payload,
//                         this, SLOT(onAriaGetActiveResponse(QVariant&, QVariant&)),
//                         this, SLOT(onAriaGetActiveFault(int, QString, QVariant &)));
}

void Karia2::onAriaGetActiveResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
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

    // qDebug()<<"TSpeed:"<<speed<<" TLen:"<<totalLength<<" GLen:"<<gotLength;
    double sumSpeed =speed*1.0/1000;
    if (sumSpeed >= 0.0) {
        this->mSpeedTotalLable->setText(QString("%1 KB/s").arg(sumSpeed));
        this->mSpeedProgressBar->setValue((int)sumSpeed);
    } else {
        this->mSpeedTotalLable->setText(QString("%1 B/s").arg(speed));
        this->mSpeedProgressBar->setValue((int)sumSpeed);
    }

    this->mISHW->updateSpeedHistogram(sumSpeed);
}

void Karia2::onAriaGetActiveFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaGetServersResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;

    int taskId = payload.toInt();
    TaskServerModel *serverModel = this->mTaskMan->taskServerModel(taskId);
    if (serverModel) {
        QVariantList servers = response.toList();
        serverModel->setData(servers);
        // QTreeView::rowsInserted internal representation of the model has been corrupted, resetting.
    } else {
        qDebug()<<"server model not found";
    }
}

void Karia2::onAriaGetServersFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaGetTorrentPeersResponse(QVariant &response, QVariant &payload)
{
    int taskId = payload.toInt();

    QVariantList peers = response.toList();
    this->mTaskMan->setPeers(taskId, peers);
    // qDebug()<<__FUNCTION__<<response<<payload;
}
void Karia2::onAriaGetTorrentPeersFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaParseTorrentFileResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
    /*
      onAriaGetTorrentFilesResponse QVariant(QString, "1") QVariant(QVariantMap, QMap(("cmd", QVariant(QString, "torrent_get_files") ) ("taskId" ,  QVariant(int, 42) ) ("url" ,  QVariant(QString, "file:///home/gzleo/NGDownload/1CA79A2AD33A0C89157E2BE71A0AF8A1A735A83A.torrent") ) )  )
     */
    QString ariaGid = response.toString();
    QMap<QString, QVariant> mPayload = payload.toMap();
    int taskId = mPayload["taskId"].toInt();
    QString url = mPayload["url"].toString();
    mPayload["ariaGid"] = ariaGid;
    qDebug()<<__FUNCTION__<<url<<taskId;

    //this->initXmlRpc();

    QVariantList args;
    args.insert(0, ariaGid);

    // now use tellStatus to get files and torrent file info
//    this->mAriaRpc->call(QString("aria2.tellStatus"), args, QVariant(mPayload),
//                         this, SLOT(onAriaGetTorrentFilesResponse(QVariant &, QVariant &)),
//                         this, SLOT(onAriaGetTorrentFilesFault(int, QString, QVariant &)));


}

void Karia2::onAriaParseTorrentFileFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
    // onAriaParseTorrentFileFault 1 "We encountered a problem while processing the option '--select-file'."
    /*
      2010-04-21 10:44:24.049116 ERROR - Exception caught
      Exception: [RequestGroup.cc:324] File /home/gzleo/karia2-svn/Kansas - Monolith [1979] exists, but a control file(*.aria2) does not exist. Download was canceled in order to prevent your file from being truncated to 0. If you are sure to download the file all over again, then delete it or add --allow-overwrite=true option and restart aria2.
     */
}

void Karia2::onAriaGetTorrentFilesResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    QMap<QString, QVariant> mPayload = payload.toMap();
    // int taskId = payload.toMap().value("taskId").toInt();
    QMap<QString, QVariant> statusMap = response.toMap();
    QVariantList files = statusMap["files"].toList(); // response.toList();

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
}

void Karia2::onAriaGetTorrentFilesFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onTorrentRemoveConfirmTimeout()
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
//                         this, SLOT(onAriaRemoveGetTorrentFilesConfirmFault(int, QString, QVariant &)));
}

void Karia2::onAriaRemoveGetTorrentFilesConfirmResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    QVariantMap msts = response.toMap();
    QVariantMap mPayload = payload.toMap();

    if (msts.value("status").toString() == "removed"
        || msts.value("status").toString() == "error") {
        mPayload["removeConfirm"] = "yes";
        QVariant aPayload = QVariant(mPayload);
        this->onAriaRemoveTorrentParseFileTaskResponse(response, aPayload);
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

void Karia2::onAriaRemoveGetTorrentFilesConfirmFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaRemoveTorrentParseFileTaskResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
    qDebug()<<__FUNCTION__<<payload;

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
//                         this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
//                         this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

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
    this->mTaskMan->setSeedFiles(taskId, seedFiles);
}

void Karia2::onAriaRemoveTorrentParseFileTaskFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaTorrentUpdaterTimeout()
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

void Karia2::onAriaGetVersionResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}
void Karia2::onAriaGetVersionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}
void Karia2::onAriaGetSessionInfoResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}
void Karia2::onAriaGetSessionInfoFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaMultiCallVersionSessionResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}

void Karia2::onAriaMultiCallVersionSessionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaChangeGlobalOptionResponse(QVariant &response, QVariant &payload)
{
    // qDebug()<<__FUNCTION__<<response<<payload;
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
    //                      this, SLOT(onAriaGetGlobalOptionResponse(QVariant &, QVariant &)),
    //                      this, SLOT(onAriaGetGlobalOptionFault(int, QString, QVariant &)));
}

void Karia2::onAriaChangeGlobalOptionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaGetGlobalOptionResponse(QVariant &response, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<response<<payload;
}

void Karia2::onAriaGetGlobalOptionFault(int code, QString reason, QVariant &payload)
{
    qDebug()<<__FUNCTION__<<code<<reason<<payload;
}

void Karia2::onAriaTorrentReselectFileMachineResponse(QVariant &response, QVariant &payload)
{

}

void Karia2::onAriaTorrentReselectFileMachineFault(int code, QString reason, QVariant &payload)
{

}

void Karia2::onAriaProcError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        // this->mAriaGlobalUpdater.stop();
        QMessageBox::warning(this, tr("Aria2 backend error :"),
                             tr("Can not start aria2. Are you already installed it properly?"));
    }
}

void Karia2::onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
}
