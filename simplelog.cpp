﻿// log.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-05-01 16:02:41 +0800
// Version: $Id$
// 

#include <stdio.h>
#include <assert.h>

#include <QFile>
#include <QDir>
#include <QSettings>
#include <QCoreApplication>

#include "simplelog.h"

#ifdef WIN32
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO  2
#endif

FileLog *FileLog::mInst = NULL;
pthread_mutex_t FileLog::mIMutex = PTHREAD_MUTEX_INITIALIZER;

FileLog::FileLog()
{
    this->mStream = new QFile(0);

    int log_to_file_mode = 1;
    log_to_file_mode = (QSettings(qApp->applicationDirPath()+"/kitphone.ini", QSettings::IniFormat)
                        .value("logtofile").toString() == "true") ? 1 : 0;
    if (log_to_file_mode) {
        if (!QDir().exists(qApp->applicationDirPath() + "/dlog")) {
            QDir().mkdir(qApp->applicationDirPath() + "/dlog");
        }
        QString log_file_name = QString("%1/dlog/kitdebug-%2.log")
            .arg(qApp->applicationDirPath())
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd"));
        this->mStream->setFileName(log_file_name);
        if (!this->mStream->open(QIODevice::Append)) {
            Q_ASSERT(1==2);
        }
    } else {
        // qDebug()<<"opening stream.......";
        if (!this->mStream->open(STDERR_FILENO, QIODevice::WriteOnly)) {
            Q_ASSERT(1==2);
            qDebug()<<this->mStream->errorString();
            assert(1==2);
        }
    }
}

FileLog::~FileLog()
{
    // qDebug()<<__FILE__<<__LINE__<<__FUNCTION__;
    this->mStream->close();
    delete this->mStream;
}

QFile *FileLog::stream()
{
    return this->mStream;
}

FileLog * FileLog::instance()
{
    pthread_mutex_lock(&FileLog::mIMutex);
    if (FileLog::mInst == NULL) {
        FileLog *hlog = new FileLog();
        FileLog::mInst = hlog;
    }
    pthread_mutex_unlock(&FileLog::mIMutex);

    return FileLog::mInst;
}
