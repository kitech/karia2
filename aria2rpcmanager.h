// aria2rpcmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-25 21:37:36 +0000
// Version: $Id$
// 


#ifndef _ARIA2RPCMANAGER_H_
#define _ARIA2RPCMANAGER_H_

#include <QtCore>

#include "aria2manager.h"

class MaiaXmlRpcClient;
class Aria2RpcTransport;

/**
 * 这一层实现aria2c后端进程的管理
 * 定义数据传输层接口
 */
class Aria2RpcManager : public Aria2Manager
{
    Q_OBJECT;
public:
    Aria2RpcManager();
    virtual ~Aria2RpcManager();

public slots: // from user action

signals:
    void taskLogReady(QString cuid, QString itime, QString log);
    void taskLogReady(QString log);
    void error(QProcess::ProcessError e);
    void finished(int eixtCode, QProcess::ExitStatus s);

public slots: // from aria2c process
    virtual void onAriaProcError(QProcess::ProcessError error) = 0;
    virtual void onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus) = 0;
    virtual void onAriaProcReadyReadStdout() = 0;
    virtual void onAriaProcReadyReadStdoutWithParser() = 0;
    virtual void onAriaProcReadyReadStderr() = 0;
    virtual void onAriaProcStarted() = 0;
    virtual void onAriaProcStateChanged(QProcess::ProcessState newState) = 0;
    
    virtual void onLogChannelReadyRead() = 0;

    virtual void onAriaGetFeatureResponse(QVariant &response, QVariant &payload) = 0;
    virtual void onAriaGetFeatureFault(int code, QString reason, QVariant &payload) = 0;

public:
    // 0xFFFFFFFF
    enum AriaFeature {FeatureBitTorrent = 0x00000001,
                      FeatureGZip = 0x00000002,
                      FeatureHTTPS = 0x00000004,
                      FeatureMessageDigest = 0x00000008,
                      FeatureMetalink = 0x00000010,
                      FeatureXMLRPC = 0x00000020,
                      FeatureAsyncDNS = 0x00000040,
                      FeatureFirefox3Cookie = 0x00000080
    };

public slots: // from internal trigger
    bool startBackend();
    bool stopBackend();
    bool restartBackend();
    int rpcPort();

    bool hasFeature(AriaFeature feature);
    
    bool initBackendState();
    int findFirstUsableRpcPort();

protected:
    Aria2RpcTransport *mTransport;
    QProcess *mAriaProc;
    MaiaXmlRpcClient *mAriaRpc;
    Q_PID mAriaPid;
    QString mSessionId;
    QString mVersionString;
    int mIVersion;
    int mFeatures;
    QStringList mStartArgs;
    int defaultRpcPort;
    int currentRpcPort;
    QString mPidFile;
    QString mLogFilePath;
    QFile  *mLogFile;
    QTextCodec *mCodec;
};

#endif /* _ARIA2RPCMANAGER_H_ */
