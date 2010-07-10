// database.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-07 21:26:24 +0800
// Version: $Id$
// 


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
    char connInfo[200] = {0};
    snprintf(connInfo, sizeof(connInfo), "hostaddr=%s dbname=%s user=%s password=%s port=%d",
             DB_HOST, DB_NAME, DB_USER, DB_PASSWD, DB_PORT);
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
            qDebug()<<"Unknown connect status:"<<st;
            break;
        }
        PQfinish(this->conn);
        this->conn = NULL;
    }
    return false;
}

bool Database::isConnected()
{
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

