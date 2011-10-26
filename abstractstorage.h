// abstractstorage.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-07 21:03:20 +0800
// Version: $Id$
// 

#ifndef ABSTRACTSTORAGE_H
#define ABSTRACTSTORAGE_H

#include <QObject>
#include <QString>
#include <QCoreApplication>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QThread>

class AbstractStorage : public QThread
{
	Q_OBJECT;
public:
    AbstractStorage(QObject *parent);
    virtual ~AbstractStorage();

	virtual bool open() = 0;
	virtual bool close() = 0;

    virtual bool transaction() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;

	//virtual void writeDefault() = 0 ;

	//virtual void readDefault() = 0 ;

	//virtual void readConfigure() = 0 ;

	//virtual void writeConfigure() = 0 ;

	//virtual void readCategory() = 0 ;

	//virtual void writeCategory() = 0 ;

	//virtual void readTaskList() = 0 ;

	//virtual void writeTaskList() = 0 ;

	//virtual void saveAll() = 0 ;

	//virtual void readOperationLog() = 0 ;

	//virtual void writeOperationLog() = 0 ;

	//virtual void readMirrorList() = 0 ;

	//virtual void writeMirrorList() = 0 ;

protected:
	QString storePath ;
	QString dbSuffix ;

	QString optionDBName  ; 
	QString taskDBName  ; 
	QString logDBName  ; 
	QString mirrorDBName  ; 

	QString optionsPrefixName ;
	QString tasksPrefixName ;
	QString logsPrefixName ;
	QString mirrorsPrefixName ;

	QMap<QString , QString> defaultOptions ;
	QVector< QMap<QString , QString> > defaultCategorys ;

	char * mTaskColumnStr ; 
	char * mSegColumnStr ;
	char * mCatColumnStr ;

private:
    

};

#endif // ABSTRACTSTORAGE_H

