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
    switch(PQstatus(this->conn)) {
    case CONNECTION_STARTED:
        break;
    case CONNECTION_MADE:
        break;
    default:
        break;
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

