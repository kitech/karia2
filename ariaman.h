// ariaman.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-02 21:16:57 +0800
// Version: $Id$
// 
#ifndef _ARIAMAN_H_
#define _ARIAMAN_H_

#include <QtCore>

class MaiaXmlRpcClient;

class AriaMan : public QObject
{
    Q_OBJECT;
public:
    AriaMan(QObject *parent = 0);
    ~AriaMan();
    
    bool start();
    bool stop();
    bool restart();

    int rpcPort();

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
    bool hasFeature(AriaFeature feature);

signals:
    void taskLogReady(QString cuid, QString itime, QString log);
    void error(QProcess::ProcessError e);
    void finished(int eixtCode, QProcess::ExitStatus s);

private slots:
    void onAriaProcError(QProcess::ProcessError error);
    void onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAriaProcReadyReadStdout();
    void onAriaProcReadyReadStderr();
    void onAriaProcStarted();
    void onAriaProcStateChanged(QProcess::ProcessState newState);
    
    void onLogChannelReadyRead();

    void onAriaGetFeatureResponse(QVariant &response, QVariant &payload);
    void onAriaGetFeatureFault(int code, QString reason, QVariant &payload);

private:
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




#endif /* _ARIAMAN_H_ */
