// databaseworker.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-04-25 20:19:04 +0800
// Version: $Id$
// Version: $Id$
// 

#ifndef _DATABASEWORKER_H_
#define _DATABASEWORKER_H_

#include <QtSql>

// The class that does all the work with the database. This class will
// be instantiated in the thread object's run() method.
class DatabaseWorker : public QObject
{
    Q_OBJECT;
public:
    explicit DatabaseWorker(QObject* parent = 0);
    virtual ~DatabaseWorker();

    void setInitSqls(QMap<QString, QString> creates, QHash<QString, QStringList> cinits);
    bool connectDatabase();                             

    // utils
    QString escapseString(const QString &str);

public slots:
    void slotExecute( const QString& query, int reqno);
    void slotExecute( const QStringList& querys, int reqno);
    int syncExecute(const QString &query, QList<QSqlRecord> &records);
    int syncExecute(const QString &query, QVector<QSqlRecord> &records);
 
signals:
    void connected();
    void connect_error(const QString &errmsg);
    void connect_error(const QString &errmsg);
    // eval is lastInsertId
    void results( const QList<QSqlRecord>& records, int reqno, bool eret, 
                  const QString &estr, const QVariant &eval);

private:
    // QSqlDatabase m_database;
    QMap<QString, QString> createSqls; // table_name -> create table
    QHash<QString, QStringList> cinitSqls; // table_name -> insert after create table
};

/*
  不要把QSqlDatabase实现作为类成员变量存储起来。

  Warning: There should be no open queries on the database connection when this function is called, otherwise a resource leak will occur.

  Example:
  // WRONG
  QSqlDatabase db = QSqlDatabase::database("sales");
  QSqlQuery query("SELECT NAME, DOB FROM EMPLOYEES", db);
  QSqlDatabase::removeDatabase("sales"); // will output a warning
  // "db" is now a dangling invalid database connection,
  // "query" contains an invalid result set

  The correct way to do it:
  {
  QSqlDatabase db = QSqlDatabase::database("sales");
  QSqlQuery query("SELECT NAME, DOB FROM EMPLOYEES", db);
  }
  // Both "db" and "query" are destroyed because they are out of scope
  QSqlDatabase::removeDatabase("sales"); // correct
 */

#endif /* _DATABASEWORKER_H_ */
