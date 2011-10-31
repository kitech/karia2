// emaria2c.cpp ---
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-10-23 06:54:12 -0700
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
#include "download_helper.h"
#include "MultiUrlRequestInfo.h"
#include "ConsoleStatCalc.h"
#include "NullStatCalc.h"
#include "NullOutputFile.h"

#include "emaria2c.h"
#include "karia2statcalc.h"

#include "simplelog.h"
#include "taskinfodlg.h"

void test_emaria2c()
{
    std::vector<aria2::SharedHandle<aria2::RequestGroup> > requestGroups_;
    aria2::Option *opt;


    aria2::DownloadEngineHandle de = aria2::DownloadEngineFactory().newDownloadEngine(opt, requestGroups_);
}

QMutex EAria2Man::m_inst_mutex;
EAria2Man *EAria2Man::m_instance = NULL;
EAria2Man::EAria2Man(QObject *parent)
    :QObject(parent)
{

}

EAria2Man::~EAria2Man()
{

}

EAria2Man *EAria2Man::instance()
{
    if (EAria2Man::m_instance == NULL) {
        EAria2Man::m_inst_mutex.lock();
        if (EAria2Man::m_instance == NULL) {
            EAria2Man::m_instance = new EAria2Man();
        }
        EAria2Man::m_inst_mutex.unlock();
    }
    return EAria2Man::m_instance;
}

//int EAria2Man::addTask(QString url)
//{

//    return 0;
//}

//namespace aria2 {
//extern void option_processing(Option& option, std::vector<std::string>& uris,
//                              int argc, char* argv[]);
//}

aria2::SharedHandle<aria2::StatCalc> getStatCalc(const aria2::SharedHandle<aria2::Option>& op)
{
  aria2::SharedHandle<aria2::StatCalc> statCalc;
  if(op->getAsBool(aria2::PREF_QUIET)) {
    statCalc.reset(new aria2::NullStatCalc());
  } else {
    aria2::SharedHandle<aria2::ConsoleStatCalc> impl
      (new aria2::ConsoleStatCalc(op->getAsInt(aria2::PREF_SUMMARY_INTERVAL),
                           op->getAsBool(aria2::PREF_HUMAN_READABLE)));
    impl->setReadoutVisibility(op->getAsBool(aria2::PREF_SHOW_CONSOLE_READOUT));
    impl->setTruncate(op->getAsBool(aria2::PREF_TRUNCATE_CONSOLE_READOUT));
    statCalc = impl;
  }
  return statCalc;
}

aria2::SharedHandle<aria2::OutputFile> getSummaryOut(const aria2::SharedHandle<aria2::Option>& op)
{
  if(op->getAsBool(aria2::PREF_QUIET)) {
    return aria2::SharedHandle<aria2::OutputFile>(new aria2::NullOutputFile());
  } else {
    return aria2::global::cout();
  }
}

int EAria2Man::addUri(int task_id, const QString &url, TaskOption *to)
{
    qLogx()<<task_id<<url<<to;

    EAria2Worker *eaw;
    std::vector<std::string> args;

    eaw = new EAria2Worker();
    eaw->m_tid = task_id;
    eaw->option_ = aria2::SharedHandle<aria2::Option>(new aria2::Option());
    this->m_tasks[task_id] = eaw;
//    QObject::connect(eaw, SIGNAL(progressState(int,quint32,quint64,quint64,quint32,quint32,quint32,quint32)),
//                     this, SIGNAL(progressState(int,quint32,quint64,quint64,quint32,quint32,quint32,quint32)));
    QObject::connect(eaw, SIGNAL(progressState(Aria2StatCollector*)),
                     this, SIGNAL(progressState(Aria2StatCollector*)));
    QObject::connect(eaw, SIGNAL(finished()), this, SLOT(onWorkerFinished()));

    // aria2::option_processing(*eaw->option_.get(), args, m_argc, m_argv);
    // 生成taskgroup
    memset(this->m_argv, 0 , sizeof(this->m_argv));
    this->m_argc = 1;
    strcpy(this->m_argv[0], "./karia2c");

    snprintf(this->m_argv[this->m_argc], sizeof(this->m_argv[m_argc]),
             "--max-connection-per-server=%d", 6);
    this->m_argc++;

    snprintf(this->m_argv[this->m_argc], sizeof(this->m_argv[m_argc]),
             "%s", url.toAscii().data());
    this->m_argc++;

    args.push_back(url.toStdString());

    this->_option_processing(*eaw->option_.get(), args, this->m_argc, (char**)this->m_argv);
    eaw->option_->put(aria2::PREF_MAX_CONNECTION_PER_SERVER, "6");



    aria2::createRequestGroupForUri(eaw->requestGroups_, eaw->option_, args, false, false, true);

    eaw->start();
//    aria2::error_code::Value exitStatus = aria2::error_code::FINISHED;
//    exitStatus = aria2::MultiUrlRequestInfo(eaw->requestGroups_, eaw->option_,
//                                            getStatCalc(eaw->option_),
//                                            getSummaryOut(eaw->option_))
//            .execute();
//    exitStatus = aria2::MultiUrlRequestInfo(eaw->requestGroups_, eaw->option_,
//                                            aria2::SharedHandle<aria2::StatCalc>(),
//                                            aria2::SharedHandle<aria2::OutputFile>()).execute();

    return 0;
}

/////
void overrideWithEnv(aria2::Option& op, const aria2::OptionParser& optionParser,
                     const std::string& pref,
                     const std::string& envName)
{
  char* value = ::getenv(envName.c_str());
  if(value) {
    try {
      optionParser.findByName(pref)->parse(op, value);
    } catch(aria2::Exception& e) {
      aria2::global::cerr()->printf
        ("Caught Error while parsing environment variable '%s'\n%s\n",
         envName.c_str(),
         e.stackTrace().c_str());
    }
  }
}

int EAria2Man::_option_processing(aria2::Option& op, std::vector<std::string>& uris,
                       int argc, char* argv[])
{
    aria2::OptionParser oparser;
    oparser.setOptionHandlers(aria2::OptionHandlerFactory::createOptionHandlers());
    try {
      bool noConf = false;
      std::string ucfname;
      std::stringstream cmdstream;
//      oparser.parseArg(cmdstream, uris, argc, argv);
//      {
//        // first evaluate --no-conf and --conf-path options.
//        aria2::Option op;
//        oparser.parse(op, cmdstream);
//        noConf = op.getAsBool(aria2::PREF_NO_CONF);
//        ucfname = op.get(aria2::PREF_CONF_PATH);

//        if(op.defined("version")) {
//            // showVersion();
//          exit(aria2::error_code::FINISHED);
//        }
//        if(op.defined("help")) {
//          std::string keyword;
//          if(op.get("help").empty()) {
//            keyword = TAG_BASIC;
//          } else {
//            keyword = op.get("help");
//            if(aria2::util::startsWith(keyword, "--")) {
//              keyword = keyword.substr(2);
//            }
//            std::string::size_type eqpos = keyword.find("=");
//            if(eqpos != std::string::npos) {
//              keyword = keyword.substr(0, eqpos);
//            }
//          }
//          // showUsage(keyword, oparser);
//          exit(aria2::error_code::FINISHED);
//        }
//      }

      oparser.parseDefaultValues(op);

      if(!noConf) {
        std::string cfname =
          ucfname.empty() ?
          oparser.findByName(aria2::PREF_CONF_PATH)->getDefaultValue():
          ucfname;

        if(aria2::File(cfname).isFile()) {
          std::stringstream ss;
          {
            aria2::BufferedFile fp(cfname, aria2::BufferedFile::READ);
            if(fp) {
              fp.transfer(ss);
            }
          }
          try {
            oparser.parse(op, ss);
          } catch(aria2::OptionHandlerException& e) {
            aria2::global::cerr()->printf("Parse error in %s\n%s\n",
                                   cfname.c_str(),
                                   e.stackTrace().c_str());
            aria2::SharedHandle<aria2::OptionHandler> h = oparser.findByName(e.getOptionName());
            if(h) {
              aria2::global::cerr()->printf
                ("Usage:\n%s\n",
                 oparser.findByName(e.getOptionName())->getDescription().c_str());
            }
            exit(e.getErrorCode());
          } catch(aria2::Exception& e) {
            aria2::global::cerr()->printf("Parse error in %s\n%s\n",
                                   cfname.c_str(),
                                   e.stackTrace().c_str());
            exit(e.getErrorCode());
          }
        } else if(!ucfname.empty()) {
          aria2::global::cerr()->printf("Configuration file %s is not found.\n",
                                 cfname.c_str());
          // showUsage(TAG_HELP, oparser);
          exit(aria2::error_code::UNKNOWN_ERROR);
        }
      }
      // Override configuration with environment variables.
      overrideWithEnv(op, oparser, aria2::PREF_HTTP_PROXY, "http_proxy");
      overrideWithEnv(op, oparser, aria2::PREF_HTTPS_PROXY, "https_proxy");
      overrideWithEnv(op, oparser, aria2::PREF_FTP_PROXY, "ftp_proxy");
      overrideWithEnv(op, oparser, aria2::PREF_ALL_PROXY, "all_proxy");
      overrideWithEnv(op, oparser, aria2::PREF_NO_PROXY, "no_proxy");

      // we must clear eof bit and seek to the beginning of the buffer.
      cmdstream.clear();
      cmdstream.seekg(0, std::ios::beg);
      // finaly let's parse and store command-iine options.
      oparser.parse(op, cmdstream);
  #ifdef __MINGW32__
      for(std::map<std::string, std::string>::iterator i = op.begin();
          i != op.end(); ++i) {
        if(!util::isUtf8((*i).second)) {
          (*i).second = nativeToUtf8((*i).second);
        }
      }
  #endif // __MINGW32__
    } catch(aria2::OptionHandlerException& e) {
      aria2::global::cerr()->printf("%s\n", e.stackTrace().c_str());
      aria2::SharedHandle<aria2::OptionHandler> h = oparser.findByName(e.getOptionName());
      if(h) {
        std::ostringstream ss;
        ss << *h;
        aria2::global::cerr()->printf("Usage:\n%s\n", ss.str().c_str());
      }
      exit(e.getErrorCode());
    } catch(aria2::Exception& e) {
      aria2::global::cerr()->printf("%s\n", e.stackTrace().c_str());
      // showUsage(TAG_HELP, oparser);
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
        aria2::global::cerr()->printf("%s\n", MSG_URI_REQUIRED);
        // showUsage(TAG_HELP, oparser);
        exit(aria2::error_code::UNKNOWN_ERROR);
      }
    }
//    if(op.getAsBool(PREF_DAEMON)) {
//      if(daemon(0, 0) < 0) {
//        perror(MSG_DAEMON_FAILED);
//        exit(error_code::UNKNOWN_ERROR);
//      }
//    }
    return 0;
}

void EAria2Man::onWorkerFinished()
{
    int tid;
    EAria2Worker *eaw = static_cast<EAria2Worker*>(sender());\
    aria2::SharedHandle<aria2::RequestGroup> rg;

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

        emit this->taskFinished(tid, eaw->exit_status);
    }

    this->m_tasks.remove(tid);
    eaw->deleteLater();
}

/////////////////

EAria2Worker::EAria2Worker(QObject *parent)
    : QThread(parent)
{
}

EAria2Worker::~EAria2Worker()
{

}

void EAria2Worker::run()
{
    aria2::error_code::Value exitStatus = aria2::error_code::FINISHED;
//    exitStatus = aria2::MultiUrlRequestInfo(this->requestGroups_, this->option_,
//                                            getStatCalc(this->option_),
//                                            getSummaryOut(this->option_))

    statCalc_.reset(new Karia2StatCalc(this->m_tid, this->option_->getAsInt(aria2::PREF_SUMMARY_INTERVAL)));
//    QObject::connect(statCalc_.get(), SIGNAL(progressState(int,quint32,quint64,quint64,quint32,quint32,quint32,quint32)),
//                     this, SIGNAL(progressState(int,quint32,quint64,quint64,quint32,quint32,quint32,quint32)));
    QObject::connect(statCalc_.get(), SIGNAL(progressState(Aria2StatCollector*)),
                     this, SIGNAL(progressState(Aria2StatCollector*)));
    exitStatus = aria2::MultiUrlRequestInfo(this->requestGroups_, this->option_,
                                            statCalc_, getSummaryOut(this->option_))
            .execute();
    exit_status = exitStatus;

    for (int i = 0; i < this->requestGroups_.size(); ++i) {
        aria2::SharedHandle<aria2::RequestGroup> rg = this->requestGroups_.at(i);
        qLogx()<<rg->downloadFinished()<<exit_status;
    }
}

