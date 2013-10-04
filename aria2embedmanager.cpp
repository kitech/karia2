// aria2embedmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-25 21:56:59 +0000
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

#include "RequestGroupMan.h"
#include "DownloadEngine.h"
#include "LogFactory.h"
#include "Logger.h"
#include "RequestGroup.h"
#include "prefs.h"
// #include "SharedHandle.h"
#include "DownloadEngineFactory.h"
#include "RecoverableException.h"
#include "message.h"
#include "util.h"
#include "Option.h"
#include "OptionParser.h"
#include "OptionHandlerFactory.h"
#include "OptionHandler.h"
#include "Exception.h"
#include "StatCalc.h"
#include "CookieStorage.h"
#include "File.h"
#include "Netrc.h"
#include "AuthConfigFactory.h"
#include "SessionSerializer.h"
#include "TimeA2.h"
#include "fmt.h"
#include "SocketCore.h"
#include "OutputFile.h"
#ifdef ENABLE_SSL
# include "TLSContext.h"
#endif // ENABLE_SSL
#include "console.h"
#include "help_tags.h"
#include "OptionHandlerException.h"
#include "UnknownOptionException.h"
#include "download_helper.h"
#include "MultiUrlRequestInfo.h"
#include "ConsoleStatCalc.h"
#include "NullStatCalc.h"
#include "NullOutputFile.h"

// #include "emaria2c.h"
#include "karia2statcalc.h"

#include "simplelog.h"
#include "taskinfodlg.h"

#include "aria2embedmanager.h"

Aria2EmbedManager::Aria2EmbedManager()
    : Aria2Manager()
{
}


Aria2EmbedManager::~Aria2EmbedManager()
{
}

std::shared_ptr<aria2::StatCalc> getStatCalc2(const std::shared_ptr<aria2::Option>& op)
{
  std::shared_ptr<aria2::StatCalc> statCalc;
  if(op->getAsBool(aria2::PREF_QUIET)) {
    statCalc.reset(new aria2::NullStatCalc());
  } else {
    std::shared_ptr<aria2::ConsoleStatCalc> impl
      (new aria2::ConsoleStatCalc(op->getAsInt(aria2::PREF_SUMMARY_INTERVAL),
                           op->getAsBool(aria2::PREF_HUMAN_READABLE)));
    impl->setReadoutVisibility(op->getAsBool(aria2::PREF_SHOW_CONSOLE_READOUT));
    impl->setTruncate(op->getAsBool(aria2::PREF_TRUNCATE_CONSOLE_READOUT));
    statCalc = impl;
  }
  return statCalc;
}

std::shared_ptr<aria2::OutputFile> getSummaryOut2(const std::shared_ptr<aria2::Option>& op)
{
  if(op->getAsBool(aria2::PREF_QUIET)) {
    return std::shared_ptr<aria2::OutputFile>(new aria2::NullOutputFile());
  } else {
    return aria2::global::cout();
  }
}

int Aria2EmbedManager::addTask(int task_id, const QString &url, TaskOption *to)
{
    qLogx()<<task_id<<url<<to;

    Aria2EmbedWorker *eaw;
    std::vector<std::string> args;

    eaw = new Aria2EmbedWorker();
    eaw->m_tid = task_id;
    eaw->option_ = std::shared_ptr<aria2::Option>(new aria2::Option());
    eaw->statCalc_.reset(new Karia2StatCalc(eaw->m_tid, eaw->option_->getAsInt(aria2::PREF_SUMMARY_INTERVAL)));
    // QObject::connect(statCalc_.get(), SIGNAL(progressState(Aria2StatCollector*)),
    //                  this, SIGNAL(progressState(Aria2StatCollector*)));
    QObject::connect(eaw->statCalc_.get(), &Karia2StatCalc::progressStat, this, &Aria2EmbedManager::onAllStatArrived);

    this->m_tasks[task_id] = eaw;
    this->m_rtasks[eaw] = task_id;
    QObject::connect(eaw, &QThread::finished, this, &Aria2EmbedManager::onWorkerFinished);

    // aria2::option_processing(*eaw->option_.get(), args, m_argc, m_argv);
    // 生成taskgroup
    memset(this->m_argv, 0 , sizeof(this->m_argv));
    this->m_argc = 1;
    strcpy(this->m_argv[0], "./karia2c");

    snprintf(this->m_argv[this->m_argc], sizeof(this->m_argv[m_argc]),
             "--max-connection-per-server=%d", 6);
    this->m_argc++;

    snprintf(this->m_argv[this->m_argc], sizeof(this->m_argv[this->m_argc]),
             "--min-split-size=%dM", 1);
    this->m_argc++;

    snprintf(this->m_argv[this->m_argc], sizeof(this->m_argv[m_argc]),
             "%s", url.toLatin1().data());
    this->m_argc++;

    args.push_back(url.toStdString());

    // this->_option_processing(*eaw->option_.get(), args, this->m_argc, (char**)this->m_argv);
    eaw->option_->put(aria2::PREF_MAX_CONNECTION_PER_SERVER, "6");
    eaw->option_->put(aria2::PREF_MIN_SPLIT_SIZE, "1M");
    eaw->option_->put(aria2::PREF_MAX_DOWNLOAD_LIMIT, "2000000");
    QString ugid = QString("%10000000000000000").arg(task_id, 0, 10).left(16);
    eaw->option_->put(aria2::PREF_GID, ugid.toStdString());
    qLogx()<<task_id << ugid;


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

    return 0;
}

int Aria2EmbedManager::pauseTask(int task_id)
{
    Aria2EmbedWorker *eaw;

    if (this->m_tasks.contains(task_id)) {
        eaw = (Aria2EmbedWorker*)this->m_tasks.value(task_id);
        Q_ASSERT(eaw->m_tid == task_id);

        // eaw->terminate();
        // 这个可用，比直接终止(terminate)掉要好
        // eaw->muri->getDownloadEngine()->getRequestGroupMan()->halt();
        eaw->muri->getDownloadEngine()->getRequestGroupMan()->forceHalt();
    }

    return 0;
}


/////
void overrideWithEnv2
(aria2::Option& op,
 const std::shared_ptr<aria2::OptionParser>& optionParser,
 const aria2::Pref* pref,
 const std::string& envName)
{
  char* value = getenv(envName.c_str());
  if(value) {
    try {
      optionParser->find(pref)->parse(op, value);
    } catch(aria2::Exception& e) {
        aria2::global::cerr()->printf
        (_("Caught Error while parsing environment variable '%s'"),
         envName.c_str());
        aria2::global::cerr()->printf("\n%s\n", e.stackTrace().c_str());
    }
  }
}

/*
int Aria2EmbedManager::_option_processing(aria2::Option& op, std::vector<std::string>& uris,
                       int argc, char* argv[])
{
    const std::shared_ptr<aria2::OptionParser>& oparser = aria2::OptionParser::getInstance();
  try {
    bool noConf = false;
    std::string ucfname;
    std::stringstream cmdstream;
    // oparser->parseArg(cmdstream, uris, argc, argv);
    // {
    //   // first evaluate --no-conf and --conf-path options.
    //     aria2::Option op;
    //   oparser->parse(op, cmdstream);
    //   noConf = op.getAsBool(aria2::PREF_NO_CONF);
    //   ucfname = op.get(aria2::PREF_CONF_PATH);

    //   if(op.defined(aria2::PREF_VERSION)) {
    //     showVersion();
    //     exit(aria2::error_code::FINISHED);
    //   }
    //   if(op.defined(aria2::PREF_HELP)) {
    //     std::string keyword;
    //     if(op.get(aria2::PREF_HELP).empty()) {
    //       keyword = strHelpTag(aria2::TAG_BASIC);
    //     } else {
    //       keyword = op.get(aria2::PREF_HELP);
    //       if(aria2::util::startsWith(keyword, "--")) {
    //         keyword.erase(keyword.begin(), keyword.begin()+2);
    //       }
    //       std::string::size_type eqpos = keyword.find("=");
    //       if(eqpos != std::string::npos) {
    //         keyword.erase(keyword.begin()+eqpos, keyword.end());
    //       }
    //     }
    //     showUsage(keyword, oparser, aria2::global::cout());
    //     exit(aria2::error_code::FINISHED);
    //   }
    // }

    oparser->parseDefaultValues(op);

    if(!noConf) {
      std::string cfname =
        ucfname.empty() ?
        oparser->find(aria2::PREF_CONF_PATH)->getDefaultValue() : ucfname;

      if(aria2::File(cfname).isFile()) {
        std::stringstream ss;
        {
          aria2::BufferedFile fp(cfname.c_str(), aria2::BufferedFile::READ);
          if(fp) {
            fp.transfer(ss);
          }
        }
        try {
          oparser->parse(op, ss);
        } catch(aria2::OptionHandlerException& e) {
            aria2::global::cerr()->printf(_("Parse error in %s"), cfname.c_str());
            aria2::global::cerr()->printf("\n%s", e.stackTrace().c_str());
          const aria2::OptionHandler* h = oparser->find(e.getPref());
          if(h) {
              aria2::global::cerr()->printf(_("Usage:"));
              aria2::global::cerr()->printf("\n%s\n", h->getDescription());
          }
          exit(e.getErrorCode());
        } catch(aria2::Exception& e) {
            aria2::global::cerr()->printf(_("Parse error in %s"), cfname.c_str());
            aria2::global::cerr()->printf("\n%s", e.stackTrace().c_str());
          exit(e.getErrorCode());
        }
      } else if(!ucfname.empty()) {
          aria2::global::cerr()->printf(_("Configuration file %s is not found."),
                               cfname.c_str());
          aria2::global::cerr()->printf("\n");
          // showUsage(strHelpTag(aria2::TAG_HELP), oparser, aria2::global::cerr());
        exit(aria2::error_code::UNKNOWN_ERROR);
      }
    }
    // Override configuration with environment variables.
    overrideWithEnv2(op, oparser, aria2::PREF_HTTP_PROXY, "http_proxy");
    overrideWithEnv2(op, oparser, aria2::PREF_HTTPS_PROXY, "https_proxy");
    overrideWithEnv2(op, oparser, aria2::PREF_FTP_PROXY, "ftp_proxy");
    overrideWithEnv2(op, oparser, aria2::PREF_ALL_PROXY, "all_proxy");
    overrideWithEnv2(op, oparser, aria2::PREF_NO_PROXY, "no_proxy");

    // we must clear eof bit and seek to the beginning of the buffer.
    cmdstream.clear();
    cmdstream.seekg(0, std::ios::beg);
    // finaly let's parse and store command-iine options.
    oparser->parse(op, cmdstream);
#ifdef __MINGW32__
    for(size_t i = 1, len = option::countOption(); i < len; ++i) {
      const Pref* pref = option::i2p(i);
      if(op.defined(pref) && !util::isUtf8(op.get(pref))) {
        op.put(pref, nativeToUtf8(op.get(pref)));
      }
    }
#endif // __MINGW32__
  } catch(aria2::OptionHandlerException& e) {
      aria2::global::cerr()->printf("%s", e.stackTrace().c_str());
    const aria2::OptionHandler* h = oparser->find(e.getPref());
    if(h) {
        aria2::global::cerr()->printf(_("Usage:"));
        aria2::global::cerr()->printf("\n");
      write(aria2::global::cerr(), *h);
    }
    exit(e.getErrorCode());
  } catch(aria2::UnknownOptionException& e) {
      // showUsage("", oparser, aria2::global::cerr());
      // showCandidates(e.getUnknownOption(), oparser);
    exit(e.getErrorCode());
  } catch(aria2::Exception& e) {
      aria2::global::cerr()->printf("%s", e.stackTrace().c_str());
      // showUsage("", oparser, aria2::global::cerr());
    exit(e.getErrorCode());
  }
  if(!op.getAsBool(aria2::PREF_ENABLE_RPC) &&
#ifdef ENABLE_BITTORRENT
     op.blank(aria2::PREF_TORRENT_FILE) &&
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
     op.blank(aria2::PREF_METALINK_FILE) &&
#endif // ENABLE_METALINK
     op.blank(aria2::PREF_INPUT_FILE)) {
    if(uris.empty()) {
        // aria2::global::cerr()->printf(MSG_URI_REQUIRED);
        aria2::global::cerr()->printf("\n");
        // showUsage("", oparser, aria2::global::cerr());
      exit(aria2::error_code::UNKNOWN_ERROR);
    }
  }
  if(op.getAsBool(aria2::PREF_DAEMON)) {
    if(daemon(0, 0) < 0) {
        // perror(MSG_xDAEMON_FAILED);
      exit(aria2::error_code::UNKNOWN_ERROR);
    }
  }

  return 0;
}
*/

void Aria2EmbedManager::onWorkerFinished()
{
    int tid;
    Aria2EmbedWorker *eaw = static_cast<Aria2EmbedWorker*>(sender());
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
}


//////// statqueue members
// TODO 线程需要一直运行处理等待状态，否则不断启动线程会用掉太多的资源。
void Aria2EmbedManager::run()
{
    int stkey;
    Aria2StatCollector *sclt;
    QPair<int, Aria2StatCollector*> elem;
    int tid = -1;
    Aria2EmbedWorker *eaw = 0;

    while (!this->stkeys.empty()) {
        elem = this->stkeys.dequeue();
        stkey = elem.first;
        sclt = elem.second;
        tid = sclt->tid;
        eaw = (Aria2EmbedWorker*)this->m_tasks[tid];

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

bool Aria2EmbedManager::checkAndDispatchStat(Aria2StatCollector *sclt)
{
    QMap<int, QVariant> stats; // QVariant可能是整数，小数，或者字符串
    qLogx()<<"";
    // emit this->taskStatChanged(sclt->tid, sclt->totalLength, sclt->completedLength,
    //                            sclt->totalLength == 0 ? 0: (sclt->completedLength*100/ sclt->totalLength),
    //                            sclt->downloadSpeed, sclt->uploadSpeed);

    stats[ng::stat::task_id] = sclt->tid;
    stats[ng::stat::total_length] = (qulonglong)sclt->totalLength;
    stats[ng::stat::completed_length] = (qulonglong)sclt->completedLength;
    stats[ng::stat::completed_percent] = (int)(sclt->totalLength == 0 ? 0: (sclt->completedLength*100/ sclt->totalLength));
    stats[ng::stat::download_speed] = sclt->downloadSpeed;
    stats[ng::stat::upload_speed] = sclt->uploadSpeed;
    stats[ng::stat::gid] = (qulonglong)sclt->gid;
    stats[ng::stat::num_connections] = sclt->connections;
    stats[ng::stat::hex_bitfield] = QString(sclt->bitfield.c_str());
    stats[ng::stat::num_pieces] = sclt->numPieces;
    stats[ng::stat::piece_length] = sclt->pieceLength;
    stats[ng::stat::eta] = sclt->eta;
    stats[ng::stat::str_eta] = QString::fromStdString(aria2::util::secfmt(sclt->eta));
    stats[ng::stat::error_code] = sclt->errorCode;
    stats[ng::stat::status] = sclt->state == 1 ? "active" : "waiting"; 
    // ready, active, waiting, complete, removed, error, pause
    
    emit this->taskStatChanged(sclt->tid, stats);

    return true;
}

bool Aria2EmbedManager::checkAndDispatchServerStat(Aria2StatCollector *sclt)
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

bool Aria2EmbedManager::confirmBackendFinished(int tid, Aria2EmbedWorker *eaw)
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
        stats[ng::stat::error_code] = eaw->exit_status;
        stats[ng::stat::error_string] = QString::fromStdString(aria2::fmt(MSG_DOWNLOAD_NOT_COMPLETE, tid, ""));
        stats[ng::stat::status] = "pause";
        emit this->taskStatChanged(eaw->m_tid, stats);
        this->m_tasks.remove(eaw->m_tid);
        this->m_rtasks.remove(eaw);
        eaw->deleteLater();
        break;
    case aria2::error_code::REMOVED:
        break;
    case aria2::error_code::RESOURCE_NOT_FOUND:
        stats[ng::stat::error_code] = eaw->exit_status;
        stats[ng::stat::error_string] = QString(MSG_RESOURCE_NOT_FOUND);
        stats[ng::stat::status] = "error";
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

bool Aria2EmbedManager::onAllStatArrived(int stkey)
{
    Aria2StatCollector *sclt = static_cast<Karia2StatCalc*>(sender())->getNextStat(stkey);
    this->stkeys.enqueue(QPair<int, Aria2StatCollector*>(stkey, sclt));
    if (!this->isRunning()) {
        this->start();
    }
    return true;
}



/////////////////

Aria2EmbedWorker::Aria2EmbedWorker(QObject *parent)
    : QThread(parent)
{
    this->exit_status = aria2::error_code::UNDEFINED;
}

Aria2EmbedWorker::~Aria2EmbedWorker()
{
    qLogx()<<"";
}

void Aria2EmbedWorker::run()
{
    aria2::error_code::Value exitStatus = aria2::error_code::UNDEFINED;
//    exitStatus = aria2::MultiUrlRequestInfo(this->requestGroups_, this->option_,
//                                            getStatCalc(this->option_),
//                                            getSummaryOut(this->option_))

    std::shared_ptr<aria2::UriListParser> ulp;
    std::shared_ptr<aria2::DownloadEngine> e;

    this->muri.reset(new aria2::MultiUrlRequestInfo(this->requestGroups_, this->option_, ulp));
    exit_status = this->muri->prepare();
    this->muri->getDownloadEngine()->setStatCalc(std::move(this->statCalc_));

    // aria2::MultiUrlRequestInfo muri(this->requestGroups_, this->option_,
    // statCalc_, getSummaryOut(this->option_), ulp);
    // exitStatus = aria2::MultiUrlRequestInfo(this->requestGroups_, this->option_,
    //                                         statCalc_, getSummaryOut(this->option_), ulp)
    //         .execute();
    try {
        muri->getDownloadEngine()->run();
    } catch(aria2::RecoverableException& e) {
        // A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
    }
    exitStatus = muri->getResult();
    exit_status = exitStatus;

    // e = muri->getDownloadEngine();
    muri->getDownloadEngine();

    // statCalc_->calculateStat(e.get());

    for (int i = 0; i < this->requestGroups_.size(); ++i) {
        std::shared_ptr<aria2::RequestGroup> rg = this->requestGroups_.at(i);
        qLogx()<<rg->downloadFinished()<<exit_status;
    }
}

// 给MultiUriRequestInfo打个补丁，存储并且返回DownloadEngine对象。


