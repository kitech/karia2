// aria2rpcserver.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-10-06 22:00:01 +0000
// Version: $Id$
// 

#ifndef _ARIA2RPCSERVER_H_
#define _ARIA2RPCSERVER_H_

#include <QtCore>

/**
 * 这一层实现aria2c服务后端进程的管理
 */
class Aria2RpcServer : public QThread
{
    Q_OBJECT;
public:
    explicit Aria2RpcServer();
    virtual ~Aria2RpcServer();
    virtual void run();

    enum {
        AST_XMLRPC, AST_JSONRPC_HTTP, AST_JSONRPC_WS, AST_JSONRPC_WSS
    };

    bool startBackend();
    bool stopBackend();
    bool restartBackend();
    QString getRpcUri(int rpcType);
    int rpcPort();

    // ws://HOST:PORT/jsonrpc
    // wss://HOST:PORT/jsonrpc
    // http://HOST:PORT/jsonrpc
    // http://HOST:PORT/rpc

public slots:

signals:
    void taskLogReady(QString cuid, QString itime, QString log);
    void taskLogReady(QString log);
    void error(QProcess::ProcessError e);
    void finished(int eixtCode, QProcess::ExitStatus s);

private slots:
    void onAriaProcError(QProcess::ProcessError error);
    void onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAriaProcReadyReadStdout();
    void onAriaProcReadyReadStdoutWithParser();
    void onAriaProcReadyReadStderr();
    void onAriaProcStarted();
    void onAriaProcStateChanged(QProcess::ProcessState newState);
    void onLogChannelReadyRead();

private:
    bool setInitPaths();
    unsigned short chooseRpcPort();
    bool setBootArgs();

private:
    QProcess *mAriaProc;
    Q_PID mAriaPid;
    int mRpcType;
    QStringList mStartArgs;
    int defaultRpcPort;
    int currentRpcPort;
    QString mPidFile;
    QString mLogFilePath;
    QFile  *mLogFile;
    QTextCodec *mCodec;
    QString dhtFilePath;
};

#endif /* _ARIA2RPCSERVER_H_ */
