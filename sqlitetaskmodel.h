// sqlitetaskmodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-07 11:21:26 +0800
// Version: $Id: sqlitetaskmodel.h 200 2013-10-04 07:13:26Z drswinghead $
// 
#ifndef SQLITETASKMODEL_H
#define SQLITETASKMODEL_H

#include <QAbstractItemModel>

#include "sqlitestorage.h"

class ModelTreeNode;
/*
这个模型的目的是做成现有模型的替代品，这个是类似Flashget有一个任务视图和一个任务所属的线程视图
*/
class SqliteTaskModel : public QAbstractItemModel
{
	Q_OBJECT;
public:    
    ~SqliteTaskModel();

	static SqliteTaskModel *instance(int cat_id, QObject *parent = 0);
	static bool removeInstance(int cat_id);

	//static SqliteTaskModel * mHandle ;
	static QHash<int , SqliteTaskModel*> mHandle ;

	QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &child) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual bool setData (const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	virtual bool insertRows (int row, int count, const QModelIndex & parent = QModelIndex());
	virtual bool removeRows (int row, int count, const QModelIndex & parent = QModelIndex(), bool persistRemove = true);

    bool moveTasks(int srcCatId, int destCatId, QModelIndexList &mil, bool persistRemove = true);

public slots:
	//inherient
	bool submit();
	//inherient
	void revert();

private:
	
	SqliteTaskModel(int cat_id, QObject *parent = 0);

	SqliteStorage *mStorage = NULL;
	QVector<QSqlRecord>  mModelData; // 主任务记录
    QMap<QString, QVector<QSqlRecord> > mChildModelData; // 子任务记录
    ModelTreeNode *mTaskRoot = NULL;
	QVector<QString> mTasksTableColumns;

	int mCatID;
};

#endif // SQLITETASKMODEL_H
