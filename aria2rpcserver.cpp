// aria2rpcserver.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-10-06 21:59:56 +0000
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

#include "simplelog.h"
#include "aria2rpcserver.h"

Aria2RpcServer::Aria2RpcServer()
    : QThread(0)
    , mAriaProc(0)
{
}

Aria2RpcServer::~Aria2RpcServer()
{
}

void Aria2RpcServer::run()
{
}

bool Aria2RpcServer::startBackend()
{
    this->setInitPaths();
    this->chooseRpcPort();
    this->setBootArgs();

    if (this->mAriaProc == NULL) {
        this->mAriaProc = new QProcess();
        QObject::connect(this->mAriaProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onAriaProcError(QProcess::ProcessError)));
        QObject::connect(this->mAriaProc, SIGNAL(finished(int, QProcess::ExitStatus)), 
                         this, SLOT(onAriaProcFinished(int, QProcess::ExitStatus)));
        // QObject::connect(this->mAriaProc, SIGNAL(readyReadStandardError()), this, SLOT(onAriaProcReadyReadStderr()));
        QObject::connect(this->mAriaProc, SIGNAL(readyReadStandardOutput()), this, SLOT(onAriaProcReadyReadStdout()));
        QObject::connect(this->mAriaProc, SIGNAL(started()), this, SLOT(onAriaProcStarted()));
        QObject::connect(this->mAriaProc, SIGNAL(stateChanged(QProcess::ProcessState)), 
                         this, SLOT(onAriaProcStateChanged(QProcess::ProcessState)));
    }

    if (this->mAriaProc->state() == QProcess::NotRunning) {
#ifdef Q_OS_WIN
        this->mAriaProc->start(qApp->applicationDirPath() + "/aria2c.exe", this->mStartArgs);
#else
        // for my test
        if (QFile::exists("/usr/bin/aria2c.patched")) {
            this->mAriaProc->start("aria2c.patched", this->mStartArgs);
        } else {
            this->mAriaProc->start("aria2c", this->mStartArgs);
        }
#endif
        this->mAriaPid = this->mAriaProc->pid();
        qLogx()<<"aria2c's pid: "<<this->mAriaPid;
#ifdef Q_OS_WIN
#else
        QFile pidFile(this->mPidFile);
        pidFile.open(QIODevice::WriteOnly);
        pidFile.write(QString("%1").arg(this->mAriaPid).toLatin1());
#endif

        this->mCodec = QTextCodec::codecForLocale();
        if (this->mCodec == NULL) {
            this->mCodec = QTextCodec::codecForName("UTF-8");
        }
        this->mLogFile = new QFile(this->mLogFilePath);
        // bool openok =
        this->mLogFile->open(QIODevice::ReadWrite | QIODevice::Unbuffered);
        this->mLogFile->resize(0);
        this->mLogFile->seek(this->mLogFile->size());
        // QObject::connect(this->mLogFile, SIGNAL(readyRead()), this, SLOT(onLogChannelReadyRead()));
        // qLogx()<<__FUNCTION__<<this->mLogFile<<openok;
    }

    return true;
}
bool Aria2RpcServer::stopBackend()
{
    // TODO 更友好的关闭，先检查是否要aria2任务，以调用方式关闭aria2
    if (this->mAriaProc->state() != QProcess::NotRunning) {
        this->mAriaProc->terminate();
    }
    return true;
}
bool Aria2RpcServer::restartBackend()
{
    this->stopBackend();
    this->startBackend();
    return true;
}

QString Aria2RpcServer::getRpcUri(int rpcType)
{
    QString uri;

    switch (rpcType) {
    case AST_XMLRPC:
        uri = QString("http://127.0.0.1:%1/rpc").arg(this->currentRpcPort);
        break;
    case AST_JSONRPC_HTTP:
        uri = QString("http://127.0.0.1:%1/jsonrpc").arg(this->currentRpcPort);
        break;
    case AST_JSONRPC_WS:
        uri = QString("ws://127.0.0.1:%1/jsonrpc").arg(this->currentRpcPort);
        break;
    case AST_JSONRPC_WSS:
        uri = QString("wss://127.0.0.1:%1/jsonrpc").arg(this->currentRpcPort);
        break;
    default:
        qLogx()<<"unknown rpcType:"<<rpcType;
        break;
    }

    return uri;
}

int Aria2RpcServer::rpcPort()
{
    return this->currentRpcPort;
}

void Aria2RpcServer::onAriaProcError(QProcess::ProcessError err)
{
    qLogx()<<__FUNCTION__<<err;
    emit error(err);
}

void Aria2RpcServer::onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qLogx()<<__FUNCTION__<<exitCode<<exitStatus;
    emit finished(exitCode, exitStatus);
}

void Aria2RpcServer::onAriaProcReadyReadStdout()
{
    QString logLine;
    while(this->mAriaProc->bytesAvailable() > 0) {
        if (this->mAriaProc->canReadLine()) {
            logLine = this->mAriaProc->readLine();
        } else {
            logLine = this->mAriaProc->readAll();
        }
        // qLogx()<<logLine;

        if (logLine.startsWith("POST /rpc")) {
            while ((logLine = this->mAriaProc->readLine()) != "\r\n"
                   && this->mAriaProc->bytesAvailable() > 0) {
                ;
            }
        } else if (logLine.startsWith("[#")) {
        } else if (logLine.startsWith("***")) {
        } else {
            emit taskLogReady(logLine);
        }

        this->mLogFile->write(logLine.toLatin1());
    }
}


void Aria2RpcServer::onAriaProcReadyReadStdoutWithParser()
{
    QString logLine;
    while(this->mAriaProc->bytesAvailable() > 0) {
        if (this->mAriaProc->canReadLine()) {
            logLine = this->mAriaProc->readLine();
        } else {
            logLine = this->mAriaProc->readAll();
        }
        // qLogx()<<logLine;

        // log parser
        {
            int ipos, spos;
            QString cuid;
            QString itime;
            QString msg, umsg;
            if ((ipos = logLine.indexOf("INFO - CUID#")) >= 0) {
                if (logLine.indexOf("HttpServer: all response transmitted", ipos) >= 0
                    || logLine.indexOf("Persist connection", ipos) >= 0) {
                    // xml-rpc log
                } else if (logLine.indexOf("Setting up HttpListenCommand", ipos) >= 0
                           || logLine.indexOf("Using port 6800 for accepting new connections", ipos) >= 0) {
                    // not care
                } else {
                    spos = logLine.indexOf(" ", ipos + 12);
                    itime = logLine.left(26); // 2010-04-18 21:43:53.805873
                    cuid = logLine.mid(ipos + 12, spos - ipos - 12);
                    msg = logLine.right(logLine.length() - spos - 3);
                    if (msg.indexOf("正在请求") >= 0 
                        || msg.indexOf("收到应答") >= 0) {
                        
                        while ((logLine = this->mAriaProc->readLine()) != "\r\n"
                               && this->mAriaProc->bytesAvailable() > 0) {
                            msg += logLine;
                        }
                    }
                    umsg = this->mCodec->toUnicode(msg.toLatin1());

                    // qLogx()<<"LOG-PART:"<<cuid<<itime<<umsg;
                    emit this->taskLogReady(cuid, itime, umsg);
                }
            } else if (logLine.startsWith("[#")) {
                // general progress bar
            } else {
                // not care
            }
        }

        this->mLogFile->write(logLine.toLatin1());
    }
}

void Aria2RpcServer::onAriaProcReadyReadStderr()
{
    QString logLine;
    while(this->mAriaProc->bytesAvailable() > 0) {
        if (this->mAriaProc->canReadLine()) {
            logLine = this->mAriaProc->readLine();
        } else {
            logLine = this->mAriaProc->readAll();
        }
        qLogx()<<logLine;
    }
}
void Aria2RpcServer::onAriaProcStarted()
{
    this->mStartArgs.clear(); // free memory

    // the default port is 6800 on best case, change to 6800+ if any exception.
    // this->mAriaRpc = new MaiaXmlRpcClient(QUrl(QString("http://127.0.0.1:%1/rpc")
    // .arg(this->rpcPort())));

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
    /*
    this->mAriaRpc->call(QString("system.multicall"), gargs, payload,
                         this, SLOT(onAriaGetFeatureResponse(QVariant&, QVariant&)),
                         this, SLOT(onAriaGetFeatureFault(int, QString, QVariant &)));
    */
}
void Aria2RpcServer::onAriaProcStateChanged(QProcess::ProcessState newState)
{
    Q_UNUSED(newState);
}

void Aria2RpcServer::onLogChannelReadyRead()
{
    QString line;
    QStringList mline; // multiline

    // qLogx()<<__FUNCTION__<<__LINE__<<"here";

    Q_ASSERT(this->mLogFile->canReadLine());
    while (this->mLogFile->bytesAvailable() > 0) {
        line = this->mLogFile->readLine();
        qLogx()<<line;
    }
}

bool Aria2RpcServer::setInitPaths()
{
#ifdef Q_OS_WIN
    this->mPidFile = QCoreApplication::applicationDirPath() + "/.karia2/aria2.pid";
    dhtFilePath = QCoreApplication::applicationDirPath() + "/.karia2/";
#else
    this->mPidFile = QDir::homePath() + "/.karia2/aria2.pid";
    dhtFilePath = QDir::homePath() + "/.karia2/";
#endif

#ifdef Q_OS_WIN
    this->mLogFilePath = "karia2.log";
#else
    this->mLogFilePath = "/tmp/karia2.log";
#endif

    return true;
}

unsigned short Aria2RpcServer::chooseRpcPort()
{
    this->defaultRpcPort = 6800;
    this->currentRpcPort = this->defaultRpcPort;

    QTcpServer serv;
    if (!serv.listen(QHostAddress::Any, this->currentRpcPort)) {
        if (serv.serverError() == QAbstractSocket::AddressInUseError) {
            qLogx()<<serv.errorString();
#ifdef Q_OS_WIN
#else
            qLogx()<<this->mPidFile;
            QFile pidFile(this->mPidFile);
            pidFile.open(QIODevice::ReadOnly);
            QByteArray pidStr = pidFile.readAll();
            this->mAriaPid = QString(pidStr).toInt();
            if (this->mAriaPid > 10) {
                int rv = ::kill(this->mAriaPid, SIGINT); //关闭顺序开始... 紧急关闭请再按Ctrl-C。
                if (rv != 0) {
                    qLogx()<<"terminate previous aria2 error:"<<strerror(errno);
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

    return this->currentRpcPort;
}

bool Aria2RpcServer::setBootArgs()
{
    this->mStartArgs << "--no-conf"
                     << "--enable-rpc=true"
                     << "--rpc-listen-all=false"
                     << QString("--rpc-listen-port=%1").arg(this->currentRpcPort)
                     << "--listen-port=6881-6999"
                     << "--enable-dht=true"
                     << QString("--dht-file-path=%1").arg(dhtFilePath)
                     << "--dht-listen-port=6881-6999"
        // << "--interface=localhost"
                     << "--disable-ipv6=true"
        // << "--log=-"
                     << QString("--log=%1").arg(this->mLogFilePath)
                     << "--log-level=debug"
        // << "--human-readable=false"   # aria2 1.8.0 can't support this argument
                     << "--check-certificate=false"
                     << "--user-agent=karia2/1.0"
                     << "--continue"
                     << "--max-connection-per-server=7"
                     << "--max-overall-upload-limit=20000"
                     << "--summary-interval=20"
                     << "--seed-time=60"
                     << "--follow-torrent=false"
                     << "--follow-metalink=false"
                     << "--rpc-certificate=rpckey/rpc.pub"
                     << "--rpc-private-key=rpckey/rpc.pri"
                     << "--rpc-secure=true"
                     // << "--load-cookies=/tmp/aria2_cookies.ns"
                     // << "--save-cookies=/tmp/aria2_cookies.ns"
    //                     << "--all-proxy=127.0.0.1:8118"
        // << "\"--header=SetCookie: ASPSESSIONIDQCSSBTAR=CMCLDODAGFELEECLIPNHCIFI; cnzz_a91594=2; sin91594=; rtime=2; ltime=1272524204007; cnzz_eid=57555832-1258166012-http%3A//www.google.com/search%3Fhl%3Den%26source%3Dhp%26q%3Doffice+2007++%25E4%25; __utmz=100221388.1258165995.1.1.utmcsr=google|utmccn=(organic)|utmcmd=organic|utmctr=office%202007%20%20%E4%B8%8B%E8%BD%BD; __utma=100221388.708249400.1258165995.1272523913.1272524204.5; __utmc=100221388; __utmb=100221388.1.10.1272524204\""   // has  cookie ok.
        ;

    // qLogx()<<this->mStartArgs<<this->mStartArgs.join(" ");
    return true;
}

