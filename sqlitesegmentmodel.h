// sqlitesegmentmodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-07 14:50:40 +0800
// Version: $Id$
// 
#ifndef SQLITESEGMENTMODEL_H
#define SQLITESEGMENTMODEL_H

#include <QAbstractItemModel>

#include "sqlitestorage.h"


class SqliteSegmentModel : public QAbstractItemModel
{
	Q_OBJECT

	//段分配
private:
	struct _mem_pointer_s
	{
		quint64 SP ;	//起始位置		
		quint64 CP ;	//当前位置
		//quint64 EP ;	//未尾位置	,默认为下一指针的起始位置，或者为0，或者为内存总容量。由该类维护管理。
		bool   busy ;	//指的是当前这一段有没有拥有者。
	};
	typedef struct _mem_pointer_s MPT ;

    quint64  mMemCapacity ;	
	QVector< MPT > mPointerMap ;
	QMutex  mAtomMutex ;
	//minBlock == 1 
	int mMaxBlock ;

	bool sortMMList() ;

public:
	static SqliteSegmentModel * instance( int task_id , QObject * parent );
   	static bool removeInstance(int task_id);
	static QMap<int , SqliteSegmentModel*> mHandle ;
	static bool containsInstance( int task_id ) ;	//是否有这一实例

    ~SqliteSegmentModel();

	QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
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

	//段分配相关
	enum{MM_NOT_EXSIT=-1,MM_NULL=0} ;	
	
	/**
	 * -1 表示还没有设置最大长度。????
	 * 可能的返回值 0 ,无内存，>0 并且 <size ,没有总够的内存，=size有足够多的内存，
	 * 返回的值就是指针起点。
	 */
	quint64 malloc( /*quint64 size*/ );

	/**
	 * 可能的返回值　0　，　数据已经释放过了，表示内存释放完成，
	 * >0 并且 <size 部分数据释放过了，也表示内存释放完成
	 * == size ，内存释放正常，可能还有内存，需要继承试着翻译内存。
	 */
	quint64 free( quint64 SP , quint64 size  );
	
	/**
	 * 
	 */
	quint64 release( quint64 SP ) ;

	quint64 setCapacity(quint64 capacity );
	bool removeZeroAndNotBusyPointer();

	bool isMemFullAlloced();

	void setBlockBusy(quint64 SP, bool busy );
		
public slots:
	//inherient
	bool submit () ;
	//inherient
	void revert () ;
private:
	SqliteSegmentModel( int task_id , QObject *parent);

	SqliteStorage * mStorage ;
	QVector<QSqlRecord>  mModelData ;
	QVector<QString>  mSegmentsTableColumns ;
	int mTaskId ;

signals:
	void memoryOverLoad();
};

#endif // SQLITESEGMENTMODEL_H
