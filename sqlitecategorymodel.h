// sqlitecategorymodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-14 16:23:14 +0800
// Version: $Id$
// 

#ifndef SQLITECATEGORYMODEL_H
#define SQLITECATEGORYMODEL_H

#include <QtCore>
#include <QtSql>
#include <QAbstractItemModel>

class SqliteStorage;

/////category model , like QDirModol , but the item source is not file system directorys
class SqliteCategoryModel : public QAbstractItemModel   
{
	Q_OBJECT;
public:
	static SqliteCategoryModel *instance( QObject *parent = 0);
	static SqliteCategoryModel *mHandle;

	bool reload();	//重新从配置文件中读取
	virtual ~SqliteCategoryModel();

    int getCatIdByModel(QModelIndex &index);

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &child) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() ) ;
	virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() ) ;
	bool rowMoveTo( const QModelIndex & from , const QModelIndex & to );

public slots:
	//inherient
	bool submit () ;
	//inherient
	void revert () ;
private:

	SqliteCategoryModel ( QObject *parent = 0);
	//QDomDocument domDocument;
	//QDomNode catRootNode ;

	//DomItem *rootItem;

	//ConfigDatabase *mConfigDB;
	SqliteStorage * mStorage ;
	QVector<QSqlRecord>  mModelData ;
	QVector<QString>   mCatsTableColumns ;

	//可添加一个标识有数据修改的成员变量，检查这个变量比检查整个分类目录树效率要高
	//可在初始化时把二维表变成一个树结构，并在有数据修改时重新做这颗树，以提高该model的效率

};




#endif // SQLITECATEGORYMODEL_H
