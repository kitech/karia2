// skyserv.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-03 15:35:54 +0800
// Version: $Id$
// 

#ifndef SKYSERV_H
#define SKYSERV_H

#include <QMainWindow>
#include <QtCore>

class SkypePackage;
// class MetaUri;
class Skype;
class Database;
class SkyServ : public QObject
{
    Q_OBJECT;
public:
    SkyServ(QObject *parent = 0);
    virtual ~SkyServ();

public slots:
    // from skype
    void onSkypeError(int errNo, QString msg);
    void onSkypeConnected(QString skypeName);
    void onSkypeDisconnected(QString skypeName);
    void onNewStreamCreated(QString contactName, int stream);
    void onSkypePackageArrived(QString contactName, int stream, QString data);
    void onNewCallArrived(QString callerName, QString calleeName, int callID);
    void onSkypeCallHangup(QString contactName, QString calleeName, int callID);
    void onSkypeCallAnswered(int callID);

    void processRequest(QString contactName, int stream, SkypePackage *sp);

    // from sip
    void onSipCallFinished(int call_id, int status_code);
    void onSipCallMediaServerReady(unsigned short port);
    void onSipCallIncomingMediaServerReady(unsigned short port);

private:
    Database *db;
    Skype *mSkype;
    QMap<QString, QString> ccMap; // caller -> callee
};

// namespace Ui {
//     class SkyServ;
// }

// class SkyServ : public QMainWindow
// {
//     Q_OBJECT

// public:
//     explicit SkyServ(QWidget *parent = 0);
//     ~SkyServ();

// private:
//     Ui::SkyServ *ui;
// };

#endif // SKYSERV_H
