// log.cpp --- 
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
    log_to_file_mode = (QSettings(qApp->applicationDirPath()+"/karia2.ini", QSettings::IniFormat)
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
    this->mStream = NULL;
}

QFile *FileLog::stream()
{
    return this->mStream;
}

FileLog *FileLog::instance()
{
    // 双重检查锁定创建单实例对象，减少锁次数优化
    if (FileLog::mInst == NULL) {
        pthread_mutex_lock(&FileLog::mIMutex);
        if (FileLog::mInst == NULL) {
            FileLog *hlog = new FileLog();
            FileLog::mInst = hlog;
        }
        pthread_mutex_unlock(&FileLog::mIMutex);
    }

    return FileLog::mInst;
}

#include <unistd.h>
#include <sys/syscall.h>

// simple 
// setenv("QT_MESSAGE_PATTERN", "[%{type}] %{appname} (%{file}:%{line}) T%{threadid} %{function} - %{message} ", 1);
//     ::qInstallMessageHandler(myMessageOutput);
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    int tid = syscall(__NR_gettid);
    QDateTime now = QDateTime::currentDateTime();
    QString time_str = now.toString("yyyy-MM-dd hh:mm:ss.zzz");

    QStringList tlist = QString(context.file).split('/');
    QString hpath = tlist.takeAt(tlist.count() - 1);

    QString mfunc = QString(context.function);
    tlist = mfunc.split(' ');
    tlist = tlist.takeAt(1).split('(');
    mfunc = tlist.takeAt(0);

    fprintf(stderr, "[] T(%u) %s:%u %s - %s\n", tid, hpath.toLocal8Bit().data(), context.line,
            mfunc.toLocal8Bit().data(), msg.toLocal8Bit().constData());
    return;

    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}
