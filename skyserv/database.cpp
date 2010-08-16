// database.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-07 21:26:24 +0800
// Version: $Id$
// 

#include <QtCore>

#include "database.h"

#define DB_HOST "127.0.0.1"
#define DB_NAME "karia2_resource"
#define DB_USER "gzleo"
#define DB_PASSWD ""
#define DB_PORT 5432

static void noticeReceiver(void *arg, const PGresult *res)
{
    qDebug()<<__FILE__<<__LINE__<<arg<<res;
}

Database::Database(QObject *parent)
    : QObject(parent)
{
    this->conn = NULL;
}

Database::~Database()
{

}

bool Database::connectdb()
{
    QString db_host;
    QString db_name;
    QString db_user;
    QString db_passwd;
    QString db_port;

    char connInfo[200] = {0};
    QString confFile = "/home/gzleo/.skyserv.ini";
    QSettings setting(confFile, QSettings::IniFormat);
    
    setting.beginGroup("database");
    db_host = setting.value("host").toString();
    db_name = setting.value("db_name").toString();
    db_user = setting.value("user").toString();
    db_passwd = setting.value("passwd").toString();
    db_port = setting.value("port").toString();
    // qDebug()<<db_host<<db_name<<db_user<<db_passwd<<db_port;

    snprintf(connInfo, sizeof(connInfo), "hostaddr=%s dbname=%s user=%s password=%s port=%d",
             db_host.toAscii().data(), db_name.toAscii().data(), db_user.toAscii().data(),
             db_passwd.toAscii().data(), db_port.toInt());
    //             DB_HOST, DB_NAME, DB_USER, DB_PASSWD, DB_PORT);

    this->conn = PQconnectdb(connInfo);
    int st = PQstatus(this->conn);
    if (st == CONNECTION_OK) {
        qDebug()<<"Connect pg ok.";
        PQsetNoticeReceiver(this->conn, noticeReceiver, 0);
        return true;
    } else {
        switch(st) {
        case CONNECTION_STARTED:
            break;
        case CONNECTION_MADE:
            break;
        default:
            qDebug()<<"Unknown db connect status:"<<st;
            break;
        }
        PQfinish(this->conn);
        this->conn = NULL;
    }
    return false;
}

bool Database::isConnected()
{
    return (this->conn != NULL);
    return false;
}

bool Database::disconnectdb()
{
    return false;
}
bool Database::reconnectdb()
{
    return false;
}

