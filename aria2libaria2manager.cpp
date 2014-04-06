// aria2libaria2manager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-10-05 13:13:50 +0000
// Version: $Id$
// 


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>

#include <cstring>
#include <ostream>
#include <sstream>
#include <iostream>

#include "util.h" //from aria2-root

#include "karia2statcalc.h"

#include "simplelog.h"
#include "taskinfodlg.h"

#include "aria2libaria2manager.h"

Aria2Libaria2Manager::Aria2Libaria2Manager()
    : Aria2Manager()
    , a2sess(NULL)
{
}


Aria2Libaria2Manager::~Aria2Libaria2Manager()
{
}

int Aria2Libaria2Manager::addTask(int task_id, const QString &url, TaskOption *to)
{
    qLogx()<<task_id<<url<<to;

    aria2::KeyVals opt1;
    opt1.push_back(aria2::KeyVal("log", "/tmp/karia2.log"));
    opt1.push_back(aria2::KeyVal("max-overall-download-limit", "20000"));
    opt1.push_back(aria2::KeyVal("max-overall-upload-limit", "20000"));

    aria2::libraryInit();
    this->a2sess = aria2::sessionNew(opt1, this->a2cfg);


    Aria2Libaria2Worker *eaw;
    std::vector<std::string> args;

    eaw = new Aria2Libaria2Worker();
    eaw->m_tid = task_id;
    eaw->statCalc_.reset(new Karia2StatCalc(eaw->m_tid, 60, this->a2sess));
    QObject::connect(eaw->statCalc_.get(), &Karia2StatCalc::progressStat, this, &Aria2Libaria2Manager::onAllStatArrived);

    if (this->m_tasks.contains(task_id)) {
        qLogx()<<"task already in manager: " << task_id << this->m_tasks[task_id];
    }
    this->m_tasks[task_id] = eaw;
    this->m_rtasks[eaw] = task_id;
    QObject::connect(eaw, &QThread::finished, this, &Aria2Libaria2Manager::onWorkerFinished);

    args.push_back(url.toStdString());
    eaw->a2sess = this->a2sess;

    aria2::KeyVals opts;
    opts.push_back(aria2::KeyVal("dir", to->getSaveDir().toStdString()));
    opts.push_back(aria2::KeyVal("split", "3"));
    opts.push_back(aria2::KeyVal("min-split-size", "1M"));
    opts.push_back(aria2::KeyVal("max-connection-per-server", "4"));
    // opts.push_back(aria2::KeyVal("max-download-limit", "20000"));
    QString ugid = QString("%10000000000000000").arg(task_id, 0, 10).left(16);
    opts.push_back(aria2::KeyVal("gid", ugid.toStdString()));
    int rv = aria2::addUri(this->a2sess, nullptr, args, opts);
    eaw->start();    

    /*
    eaw = new Aria2Libaria2Worker();
    eaw->m_tid = task_id;
    eaw->option_ = std::shared_ptr<aria2::Option>(new aria2::Option());
    eaw->statCalc_.reset(new Karia2StatCalc(eaw->m_tid, eaw->option_->getAsInt(aria2::PREF_SUMMARY_INTERVAL)));
    QObject::connect(eaw->statCalc_.get(), &Karia2StatCalc::progressStat, this, &Aria2Libaria2Manager::onAllStatArrived);

    this->m_tasks[task_id] = eaw;
    this->m_rtasks[eaw] = task_id;
    QObject::connect(eaw, &QThread::finished, this, &Aria2Libaria2Manager::onWorkerFinished);

    // 生成taskgroup
    args.push_back(url.toStdString());

    aria2::OptionParser::getInstance()->parseDefaultValues(*eaw->option_.get());
    eaw->option_->put(aria2::PREF_DIR, to->getSaveDir().toStdString());
    eaw->option_->put(aria2::PREF_LOG, "/tmp/karia2.log");
    eaw->option_->put(aria2::PREF_LOG_LEVEL, "debug");
    eaw->option_->put(aria2::PREF_MAX_CONNECTION_PER_SERVER, "6");
    eaw->option_->put(aria2::PREF_MIN_SPLIT_SIZE, "1M");
    eaw->option_->put(aria2::PREF_MAX_DOWNLOAD_LIMIT, "20000");
    QString ugid = QString("%10000000000000000").arg(task_id, 0, 10).left(16);
    eaw->option_->put(aria2::PREF_GID, ugid.toStdString());
    qLogx()<<task_id << ugid;

    // 重新设置aria2的log设置，
    // TODO 需要设计怎么针对不同的任务启动不同的日志记录方式。
    aria2::global::initConsole(false);
    aria2::LogFactory::setLogFile(eaw->option_->get(aria2::PREF_LOG));
    aria2::LogFactory::setLogLevel(eaw->option_->get(aria2::PREF_LOG_LEVEL));
    aria2::LogFactory::setConsoleLogLevel(eaw->option_->get(aria2::PREF_CONSOLE_LOG_LEVEL));
    if (eaw->option_->getAsBool(aria2::PREF_QUIET)) {
        aria2::LogFactory::setConsoleOutput(false);
    }
    aria2::LogFactory::reconfigure();

    // TODO start in thread 
    aria2::createRequestGroupForUri(eaw->requestGroups_, eaw->option_, args, false, false, true);

    eaw->start();
//    aria2::error_code::Value exitStatus = aria2::error_code::FINISHED;
//    exitStatus = aria2::MultiUrlRequestInfo(eaw->requestGroups_, eaw->option_,
//                                            getStatCalc(eaw->option_),
//                                            getSummaryOut(eaw->option_))
//            .execute();
//    exitStatus = aria2::MultiUrlRequestInfo(eaw->requestGroups_, eaw->option_,
//                                            std::shared_ptr<aria2::StatCalc>(),
//                                            std::shared_ptr<aria2::OutputFile>()).execute();
    */
    return 0;
}

int Aria2Libaria2Manager::pauseTask(int task_id)
{
    Aria2Libaria2Worker *eaw;

    if (this->m_tasks.contains(task_id)) {
        eaw = (Aria2Libaria2Worker*)this->m_tasks.value(task_id);
        Q_ASSERT(eaw->m_tid == task_id);

        aria2::shutdown(eaw->a2sess, false);
        
        // eaw->terminate();
        // 这个可用，比直接终止(terminate)掉要好
        // eaw->muri->getDownloadEngine()->getRequestGroupMan()->halt();
        // eaw->muri->getDownloadEngine()->getRequestGroupMan()->forceHalt();
    }

    return 0;
}

bool Aria2Libaria2Manager::setSpeedLimit(int downloadSpeed, int uploadSpeed)
{
    if (this->a2sess != NULL) {
        aria2::KeyVals opts;
        downloadSpeed = (downloadSpeed <= 0) ? INT_MAX : downloadSpeed;
        uploadSpeed = (uploadSpeed <= 0) ? INT_MAX : uploadSpeed;
        opts.push_back(aria2::KeyVal("max-overall-download-limit", QString("%1").arg(downloadSpeed).toLatin1().data()));
        opts.push_back(aria2::KeyVal("max-overall-upload-limit", QString("%1").arg(uploadSpeed).toLatin1().data()));

        // 这个设置可能不管用，因为每个任务都有了限速设置
        aria2::changeGlobalOption(this->a2sess, opts);

        opts.clear();
        opts.push_back(aria2::KeyVal("max-download-limit", QString("%1").arg(downloadSpeed).toLatin1().data()));
        opts.push_back(aria2::KeyVal("max-uplaod-limit", QString("%1").arg(downloadSpeed).toLatin1().data()));
        std::vector<aria2::A2Gid> gids = aria2::getActiveDownload(this->a2sess);
        for (auto gid : gids) {
            aria2::changeOption(this->a2sess, gid, opts);
        }
        for (auto item : this->m_tasks) {
            // item会是什么数据类型呢
        }
    }
    return true;
}

void Aria2Libaria2Manager::onWorkerFinished()
{
    /*
    int tid;
    Aria2Libaria2Worker *eaw = static_cast<Aria2Libaria2Worker*>(sender());
    std::shared_ptr<aria2::RequestGroup> rg;

    tid = eaw->m_tid;
    for (int i = 0; i < eaw->requestGroups_.size(); ++i) {
        rg = eaw->requestGroups_.at(i);

        switch(eaw->exit_status) {
        case aria2::error_code::FINISHED:
            break;
        case aria2::error_code::IN_PROGRESS:
            break;
        case aria2::error_code::REMOVED:
            break;
        default:
            break;
        }

        // 这个信号的接收会早于statcalc的时间，导致上层UI处理不正确
        // emit this->taskFinished(tid, eaw->exit_status);

    }

    qLogx()<<"tid:"<<tid<<"done size:"<<eaw->requestGroups_.size()<<" download finished:"<<eaw->exit_status;

    Aria2StatCollector *sclt = new Aria2StatCollector();
    sclt->tid = tid;
    this->doneCounter.testAndSetOrdered(INT_MIN, -1);
    int stkey = this->doneCounter.fetchAndAddRelaxed(-1);
    this->stkeys.enqueue(QPair<int, Aria2StatCollector*>(stkey, sclt));
    if (!this->isRunning()) {
        this->start();
    }
    */
}


//////// statqueue members
// TODO 线程需要一直运行处理等待状态，否则不断启动线程会用掉太多的资源。
void Aria2Libaria2Manager::run()
{
    int stkey;
    Aria2StatCollector *sclt;
    QPair<int, Aria2StatCollector*> elem;
    int tid = -1;
    Aria2Libaria2Worker *eaw = 0;

    while (!this->stkeys.empty()) {
        elem = this->stkeys.dequeue();
        stkey = elem.first;
        sclt = elem.second;
        tid = sclt->tid;
        eaw = (Aria2Libaria2Worker*)this->m_tasks[tid];

        qLogx()<<"dispatching stat event:"<<stkey;

        if (stkey < 0) {
            // 任务完成事件
            this->confirmBackendFinished(tid, eaw);
        } else {
            this->checkAndDispatchStat(sclt);
            this->checkAndDispatchServerStat(sclt);
        }

        delete sclt;
    }
}

bool Aria2Libaria2Manager::checkAndDispatchStat(Aria2StatCollector *sclt)
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
    stats[ng::stat2::str_eta] = QString::fromStdString(aria2::util::secfmt(sclt->eta));
    stats[ng::stat2::error_code] = sclt->errorCode;
    stats[ng::stat2::status] = sclt->state == aria2::DOWNLOAD_ACTIVE ? "active" : "waiting"; 
    // ready, active, waiting, complete, removed, error, pause
    
    emit this->taskStatChanged(sclt->tid, stats);

    stats2[ng::stat2::download_speed] = sclt->globalStat2.downloadSpeed;
    stats2[ng::stat2::upload_speed] = sclt->globalStat2.uploadSpeed;
    emit this->globalStatChanged(stats2);

    return true;
}

bool Aria2Libaria2Manager::checkAndDispatchServerStat(Aria2StatCollector *sclt)
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
    return true;
}

bool Aria2Libaria2Manager::confirmBackendFinished(int tid, Aria2Libaria2Worker *eaw)
{
    QMap<int, QVariant> stats; // QVariant可能是整数，小数，或者字符串

    aria2::GroupId::clear();

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

    return true;
}

bool Aria2Libaria2Manager::onAllStatArrived(int stkey)
{
    Aria2StatCollector *sclt = static_cast<Karia2StatCalc*>(sender())->getNextStat(stkey);
    this->stkeys.enqueue(QPair<int, Aria2StatCollector*>(stkey, sclt));
    if (!this->isRunning()) {
        this->start();
    }
    return true;
}



/////////////////

Aria2Libaria2Worker::Aria2Libaria2Worker(QObject *parent)
    : QThread(parent)
{
    this->exit_status = aria2::error_code::UNDEFINED;
}

Aria2Libaria2Worker::~Aria2Libaria2Worker()
{
    qLogx()<<"";
}

#include <chrono>
void Aria2Libaria2Worker::run()
{
    auto start = std::chrono::steady_clock::now();
    int rv;
    for (;;) {
        rv = aria2::run(this->a2sess, aria2::RUN_ONCE); // run 1sec, rv==0无下载任务，rv==-1有错误
        if (rv != 1) {
            break;
        }
        auto now = std::chrono::steady_clock::now();
        auto count = std::chrono::duration_cast<std::chrono::milliseconds>
                                (now - start).count();

        if(count >= 300) {
            qLogx()<<"haha cycle:"<<rv;
            start = now;
            std::vector<aria2::A2Gid> gids = aria2::getActiveDownload(this->a2sess);
            for (auto gid : gids) {
                aria2::DownloadHandle* dh = aria2::getDownloadHandle(this->a2sess, gid);
                if(dh) {
                    this->statCalc_->calculateStat(dh);
                    qLogx()<<"progress:"<<gid<<dh->getTotalLength()<<dh->getCompletedLength()
                           <<dh->getDownloadSpeed()<<dh->getUploadSpeed()
                           <<dh->getNumFiles();
                    aria2::deleteDownloadHandle(dh);
                }
            }
        }
    }

    exit_status = rv;
}
