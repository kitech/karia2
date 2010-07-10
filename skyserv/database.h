// database.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-07 21:26:00 +0800
// Version: $Id$
// 

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <QtCore>

#include "libpq-fe.h"

class Database : public QObject
{
    Q_OBJECT;
public:
    Database(QObject *parent = 0);
    virtual ~Database();

    bool connectdb();
    bool isConnected();
    bool disconnectdb();
    bool reconnectdb();

    PGconn *connection() {
        return this->conn;
    }

private:
    PGconn *conn;
};

#endif /* _DATABASE_H_ */
