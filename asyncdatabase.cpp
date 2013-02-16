// asyncdatabase.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-04-24 20:17:39 +0800
// Version: $Id$
// 

#include "databaseworker.h"
#include "asyncdatabase.h"

// #include "querythread.h"
// #include "db.h"

#include <QDebug>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>



// TODO 还有一个问题，就是怎么区分请求与返回值的问题。
// 因为不同的返回结果需要不同的处理方式。
////

QAtomicInt AsyncDatabase::m_reqno = 1;
AsyncDatabase::AsyncDatabase(QObject *parent)
    : QThread(parent)
{
    this->m_worker = nullptr;
    this->m_connected = false;
}

AsyncDatabase::~AsyncDatabase()
{
    if (m_worker) {
        delete m_worker;
    }
}

void AsyncDatabase::setInitSqls(QMap<QString, QString> creates, QHash<QString, QStringList> cinits)
{
    this->createSqls = creates;
    this->cinitSqls = cinits;
}

void AsyncDatabase::onConnected() { 
    this->m_connected = true; 
    emit this->connected();
}

int AsyncDatabase::execute(const QString& query)
{
    this->m_reqno.testAndSetOrdered(INT_MAX, 1);
    int reqno = this->m_reqno.fetchAndAddOrdered(1);
    emit executefwd(query, reqno); // forwards to the worker
    return reqno;
}

int AsyncDatabase::execute(const QStringList& querys)
{
    this->m_reqno.testAndSetOrdered(INT_MAX, 1);
    int reqno = this->m_reqno.fetchAndAddOrdered(1);
    emit executefwd(querys, reqno); // forwards to the worker
    return reqno;
}

int AsyncDatabase::syncExecute(const QString &query, QList<QSqlRecord> &records)
{
    int iret;

    iret = this->m_worker->syncExecute(query, records);

    return iret;
}

int AsyncDatabase::syncExecute(const QString &query, QVector<QSqlRecord> &records)
{
    int iret;

    iret = this->m_worker->syncExecute(query, records);

    return iret;
}

QString AsyncDatabase::escapseString(const QString &str)
{
    return this->m_worker->escapseString(str);
}

void AsyncDatabase::run()
{
    emit ready(false);
    emit progress( "AsyncDatabase starting, one moment please..." );

    // Create worker object within the context of the new thread
    m_worker = new DatabaseWorker();
    m_worker->setInitSqls(this->createSqls, this->cinitSqls);
    QObject::connect(m_worker, &DatabaseWorker::connected, this, &AsyncDatabase::onConnected);
    QObject::connect(m_worker, &DatabaseWorker::connect_error, this, &AsyncDatabase::onConnectError);

    // TODO 新的connect语法怎么处理这种重载的方法的呢？
    // 需要使用static_cast强制转换类成员函数指针的方式
    // QObject::connect(this, &AsyncDatabase::executefwd, m_worker, &DatabaseWorker::slotExecute);  // error

    void (AsyncDatabase::*c)(const QString&, int) = &AsyncDatabase::executefwd;
    void (AsyncDatabase::*d)(const QStringList&, int) = &AsyncDatabase::executefwd;
    void (DatabaseWorker::*a)(const QString&, int)  = &DatabaseWorker::slotExecute;
    void (DatabaseWorker::*b)(const QStringList&, int)  = &DatabaseWorker::slotExecute;

    QObject::connect(this, c, m_worker, a);
    // QObject::connect(this, static_cast<void (AsyncDatabase::*)(const QString&, int)>(&AsyncDatabase::executefwd),
    //                 m_worker, static_cast<void (DatabaseWorker::*)(const QString&, int)>(&DatabaseWorker::slotExecute));
    QObject::connect(this, static_cast<void(AsyncDatabase::*)(const QStringList&, int)>(&AsyncDatabase::executefwd),
                     m_worker, static_cast<void (DatabaseWorker::*)(const QStringList&,int)>(&DatabaseWorker::slotExecute));
    // QObject::connect(this, SIGNAL(executefwd(const QString&, int)),
    //        m_worker, SLOT(slotExecute(const QString&,int)));
    // QObject::connect(this, SIGNAL(executefwd(const QStringList&, int)),
    //        m_worker, SLOT(slotExecute(const QStringList&,int)));
    
    bool bret = m_worker->connectDatabase();
    if (!bret) {
        delete m_worker; m_worker = nullptr;
        return;
    }

    // Critical: register new type so that this signal can be
    // dispatched across thread boundaries by Qt using the event
    // system
    qRegisterMetaType< QList<QSqlRecord> >( "QList<QSqlRecord>" );

    // forward final signal
    QObject::connect(m_worker, &DatabaseWorker::results, this, &AsyncDatabase::results);

    emit progress( "Press 'Go' to run a query." );
    emit ready(true);

    this->exec();  // our event loop
}

// #include "querythread.moc"
