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

signals:
    void taskLogReady(QString cuid, QString itime, QString log);

private slots:
    void onAriaProcError(QProcess::ProcessError error);
    void onAriaProcFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAriaProcReadyReadStdout();
    void onAriaProcReadyReadStderr();
    void onAriaProcStarted();
    void onAriaProcStateChanged(QProcess::ProcessState newState);
    
    void onLogChannelReadyRead();

private:
    QProcess *mAriaProc;
    Q_PID mAriaPid;
    QStringList mStartArgs;
    int defaultRpcPort;
    int currentRpcPort;
    QString mPidFile;
    QString mLogFilePath;
    QFile  *mLogFile;
    QTextCodec *mCodec;
};




#endif /* _ARIAMAN_H_ */
