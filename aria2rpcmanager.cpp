// aria2rpcmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-25 21:41:47 +0000
// Version: $Id$
// 

#include <errno.h>
#include <string.h>

#ifdef Q_OS_WIN
#else
#include <sys/types.h>
#include <signal.h>
#endif

#include <QtNetwork>

#include "maiaXmlRpcClient.h"

#include "simplelog.h"

#include "aria2rpcmanager.h"
#include "aria2rpcserver.h"

Aria2RpcManager::Aria2RpcManager()
    : Aria2Manager()
    , mRpcServer(NULL)
{
    // 模拟异步启动后端进程
    QTimer::singleShot(300, this, SLOT(initialize()));
    qLogx()<<"initialize timer setted.";
}

Aria2RpcManager::~Aria2RpcManager()
{
}


bool Aria2RpcManager::initialize()
{
    qLogx()<<"initialize started, early or later?";
    // start backend
    this->mRpcServer = new Aria2RpcServer();
    this->mRpcServer->startBackend();

    // start backend
//    this->mAriaMan = new AriaMan();
//    this->mAriaMan->start();
//    this->mAriaGlobalUpdater.setInterval(5*1000);
//    QObject::connect(&this->mAriaGlobalUpdater, SIGNAL(timeout()),
//                     this, SLOT(onAriaGlobalUpdaterTimeout()));
//    this->mAriaGlobalUpdater.start();
//    // QObject::connect(this->mAriaMan, SIGNAL(taskLogReady(QString, QString, QString)),
//    //                  this->mTaskMan, SLOT(onTaskLogArrived(QString, QString, QString)));
//    QObject::connect(this->mAriaMan, SIGNAL(error(QProcess::ProcessError)),
//                     this, SLOT(onAriaProcError(QProcess::ProcessError)));
//    QObject::connect(this->mAriaMan, SIGNAL(finished(int, QProcess::ExitStatus)),
//                     this, SLOT(onAriaProcFinished(int, QProcess::ExitStatus)));
//    QObject::connect(this->mAriaMan, SIGNAL(taskLogReady(QString)),
//                     this, SLOT(onTaskLogArrived(QString)));

    return true;
}

