// ariaman.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-02 21:17:03 +0800
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

#include "ariaman.h"


AriaMan::AriaMan(QObject *parent)
    : QObject(parent)
    , mAriaProc(NULL)
{
#ifdef Q_OS_WIN
    this->mPidFile = QCoreApplication::applicationDirPath() + "/.karia2/aria2.pid";
#else
    this->mPidFile = QDir::homePath() + "/.karia2/aria2.pid";
#endif

    this->defaultRpcPort = 6800;
    this->currentRpcPort = this->defaultRpcPort;
    QTcpServer serv;
    if (!serv.listen(QHostAddress::Any, this->currentRpcPort)) {
        if (serv.serverError() == QAbstractSocket::AddressInUseError) {
            qDebug()<<serv.errorString();
#ifdef Q_OS_WIN
#else
            qDebug()<<this->mPidFile;
            QFile pidFile(this->mPidFile);
            pidFile.open(QIODevice::ReadOnly);
            QByteArray pidStr = pidFile.readAll();
            this->mAriaPid = QString(pidStr).toInt();
            if (this->mAriaPid > 10) {
                int rv = ::kill(this->mAriaPid, SIGINT); //关闭顺序开始... 紧急关闭请再按Ctrl-C。
                if (rv != 0) {
                    qDebug()<<"terminate previous aria2 error:"<<strerror(errno);
                }
                rv = ::kill(this->mAriaPid, SIGINT); //紧急关闭序列开始...
            }
            QFile::remove(this->mPidFile);
            for (int i = this->defaultRpcPort; i < this->defaultRpcPort + 100; i++) {
                if (serv.listen(QHostAddress::Any, i)) {
                    serv.close();
                    this->currentRpcPort = i;
                    break;
                }
            }
#endif
        }
    } else {
        serv.close();
    }

    
    this->mStartArgs << "--no-conf"
                     << "--enable-xml-rpc=true"
                     << "--xml-rpc-listen-all=false"
                     << QString("--xml-rpc-listen-port=%1").arg(this->currentRpcPort)
                     << "--listen-port=6881-6999"
                     << "--enable-dht=true"
                     << "--dht-listen-port=6881-6999"
                     << "--disable-ipv6=true"
#ifdef Q_OS_WIN
                     << "--log=aria2c.log"
#else
                     << "--log=/tmp/aria2c.log"
#endif
                     << "--human-readable=false"
                     << "--check-certificate=false"
                     << "--user-agent=nullget/0.3"
                     << "--continue"
                     << "--max-overall-upload-limit=20000"
    //                     << "--all-proxy=127.0.0.1:8118"
        
        ;
}

AriaMan::~AriaMan()
{
    if (this->mAriaProc != NULL) {
        this->mAriaProc->kill();
        delete this->mAriaProc;
    }
    QFile::remove(this->mPidFile);
}

bool AriaMan::start()
{
    
    if (this->mAriaProc == NULL) {
        this->mAriaProc = new QProcess();
        QObject::connect(this->mAriaProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onAriaProcError(QProcess::ProcessError)));
        QObject::connect(this->mAriaProc, SIGNAL(finished(int, QProcess::ExitStatus)), 
                         this, SLOT(onAriaProcFinished(int, QProcess::ExitStatus)));
        // QObject::connect(this->mAriaProc, SIGNAL(readyReadStandardError()), this, SLOT(onAriaProcReadyReadStderr()));
        // QObject::connect(this->mAriaProc, SIGNAL(readyReadStandardOutput()), this, SLOT(onAriaProcReadyReadStdout()));
        QObject::connect(this->mAriaProc, SIGNAL(started()), this, SLOT(onAriaProcStarted()));
        QObject::connect(this->mAriaProc, SIGNAL(stateChanged(QProcess::ProcessState)), 
                         this, SLOT(onAriaProcStateChanged(QProcess::ProcessState)));
    }
    if (this->mAriaProc->state() == QProcess::NotRunning) {
#ifdef Q_OS_WIN
        this->mAriaProc->start(qApp->applicationDirPath() + "/aria2c.exe", this->mStartArgs);
#else
        this->mAriaProc->start("aria2c", this->mStartArgs);
#endif
        this->mAriaPid = this->mAriaProc->pid();
        qDebug()<<"aria2c's pid: "<<this->mAriaPid;
#ifdef Q_OS_WIN

#else
        QFile pidFile(this->mPidFile);
        pidFile.open(QIODevice::WriteOnly);
        pidFile.write(QString("%1").arg(this->mAriaPid).toAscii());
#endif

    }
    return true;
}
bool AriaMan::stop()
{
    if (this->mAriaProc->state() != QProcess::NotRunning) {
        this->mAriaProc->terminate();
    }
    return true;
}
bool AriaMan::restart()
{
    this->stop();
    this->start();
    return true;
}

int AriaMan::rpcPort()
{
    return this->currentRpcPort;
}

void AriaMan::onAriaProcError(QProcess::ProcessError error)
{
    qDebug()<<__FUNCTION__<<error;
}

void AriaMan::onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug()<<__FUNCTION__<<exitCode<<exitStatus;
}
void AriaMan::onAriaProcReadyReadStdout()
{
    QString logLine;
    while(this->mAriaProc->bytesAvailable() > 0) {
        if (this->mAriaProc->canReadLine()) {
            logLine = this->mAriaProc->readLine();
        } else {
            logLine = this->mAriaProc->readAll();
        }
        qDebug()<<logLine;
    }
}
void AriaMan::onAriaProcReadyReadStderr()
{
    QString logLine;
    while(this->mAriaProc->bytesAvailable() > 0) {
        if (this->mAriaProc->canReadLine()) {
            logLine = this->mAriaProc->readLine();
        } else {
            logLine = this->mAriaProc->readAll();
        }
        qDebug()<<logLine;
    }
}
void AriaMan::onAriaProcStarted()
{
}
void AriaMan::onAriaProcStateChanged(QProcess::ProcessState newState)
{
}

