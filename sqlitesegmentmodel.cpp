// sqlitesegmentmodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-07 14:50:35 +0800
// Version: $Id$
// 
#include <cassert>

#include "sqlitesegmentmodel.h"

//static 
QMap<int , SqliteSegmentModel*> SqliteSegmentModel::mHandle ;

//static

SqliteSegmentModel * SqliteSegmentModel::instance ( int task_id , QObject *parent )
{
	SqliteSegmentModel * tm = 0 ;
	
	if( SqliteSegmentModel::mHandle.contains(task_id) )
	{
		tm = SqliteSegmentModel::mHandle.value(task_id);
	}
	else
	{
		tm = new SqliteSegmentModel(task_id,0);
		SqliteSegmentModel::mHandle.insert(task_id,tm);
	}

	return tm ;
}

//static 
bool SqliteSegmentModel::removeInstance(int task_id)
{
	if( SqliteSegmentModel::mHandle.contains(task_id) )
	{
		SqliteSegmentModel::mHandle.remove(task_id);
		return true ;
	}
	else
	{
		return false ;
	}	
	return false ;
}

//static 
bool SqliteSegmentModel::containsInstance( int task_id ) 
{
	bool ret = false ;
	if( SqliteSegmentModel::mHandle.contains(task_id) )
	{		
		ret =  true ;
	}
	else
	{
		ret =  false ;
	}	
	return ret ;
}

SqliteSegmentModel::SqliteSegmentModel( int task_id , QObject *parent)
	: QAbstractItemModel(parent)
{
	this->mStorage = new SqliteStorage( parent );
	this->mStorage->open();
	this->mSegmentsTableColumns = this->mStorage->getInternalSegmentsColumns();

	this->mModelData = this->mStorage->getSementSet( task_id );
	this->mTaskId = task_id ;

	//初始化到分割数列	
	quint64 file_size = this->mStorage->getFileSizeById( task_id);
	//assert( file_size !=(quint64)(-1) );
	if( file_size !=(quint64)(-1) )
	{
		this->mMemCapacity = file_size ;
		for( int i = 0 ; i < this->mModelData.count() ; i ++ )
		{
			MPT mpt ;
			quint64 sp = 0 ;
			quint64 cp = 0 ;
			sp = this->mModelData.at(i).value((int)ng::segs::start_offset).toULongLong() ;
			cp = this->mModelData.at(i).value((int)ng::segs::abtained_length).toULongLong() ;
			mpt.SP = sp ;
			mpt.CP = cp ;
			mpt.busy = false ;
			qDebug()<< "sp :"<< sp <<"  cp:"<< cp << " file_size:"<< file_size  ;

			this->mPointerMap.append(mpt);
		}
	}
}

SqliteSegmentModel::~SqliteSegmentModel()
{	
	this->mStorage->close();
	delete this->mStorage ;
	this->mModelData.clear();
}

int SqliteSegmentModel::columnCount(const QModelIndex &/*parent*/) const
{
	//return this->mModelData.at(0).count() ;
	int cc = this->mSegmentsTableColumns.count();

	return cc ;
	return 0;
}
QVariant SqliteSegmentModel::data(const QModelIndex &index, int role) const
{
	//qDebug()<<__FUNCTION__;
	if (!index.isValid())
		return QVariant();
	if( role == Qt::DecorationRole && index.column() == 0 )	//只有第一列显示图片就可以了。
	{
		return QImage("icons/crystalsvg/16x16/filesystems/folder_violet_open.png");
	}

	if (role != Qt::DisplayRole)
		return QVariant();

	int col = index.column();
	int row = index.row();

	QVariant var = this->mModelData.at(row).value(col);

	return var ;


	return QVariant() ;
}

Qt::ItemFlags SqliteSegmentModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	//return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SqliteSegmentModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
				//case 0:
				//	return tr("Cat Name");
				//case 1:
				//	return tr("Cat Path");
				//case 2:
				//	return tr("Cat Attributes");
				default:
					return this->mStorage->getSegmentsColumns().at(section);
					//return this->mSegmentsTableColumns.at(section);
					return QVariant();
		}
	}

	return QVariant();
}

QModelIndex SqliteSegmentModel::index(int row, int column, const QModelIndex &parent)   const
{
	//qDebug()<<__FUNCTION__ ;


	QModelIndex mix = this->createIndex( row , column , 0 ) ;
	
	return mix ;
	//int parentCatID = -1 ;

	//if (!parent.isValid())
	//    parentCatID = -1 ;
	//else
	//	parentCatID = parent.internalId(); //static_cast<int>(parent.internalPointer());

	////qDebug()<< __FUNCTION__ << parentCatID ;

	//int childCatID = -1 ;
	//int counter = 0 ;
	////查找在第row行的子类
	//for( int i = 0 ; i < this->mModelData.count() ;i ++ )
	//{
	//	QSqlRecord rec = this->mModelData.at(i);
	//	if( rec.value("parent_cat_id").toInt() == parentCatID )
	//	{			
	//		if( row == counter )
	//		{
	//			childCatID = rec.value("cat_id").toInt();
	//			//qDebug()<< __FUNCTION__ << parentCatID <<" is parent of "<<childCatID ;
	//			break ;
	//		}
	//		counter += 1 ;
	//	}
	//}
	////qDebug()<< __FUNCTION__ << row <<" "<<column <<" " << parentCatID <<" is parent of "<<childCatID ;
	////
	//if ( childCatID != -1 )
	//    return createIndex( row , column , childCatID );
	//else
	//    return QModelIndex();

	return QModelIndex();
}

QModelIndex SqliteSegmentModel::parent(const QModelIndex &child) const
{
	//qDebug()<<__FUNCTION__ ;
	
	if (!child.isValid())
		return QModelIndex();



	return QModelIndex();
}

int SqliteSegmentModel::rowCount(const QModelIndex &parent) const
{
	//qDebug()<<__FUNCTION__ ;

	int count = 0 ;
	if( parent.isValid() )
	{
		count =  0 ;
	}
	else
	{
		count = this->mModelData.count() ;
	}

	////qDebug()<<__FUNCTION__ <<":"<< count ;
	return count ;
	return 0;
}

bool SqliteSegmentModel::insertRows ( int row, int count, const QModelIndex & parent  )
{
	//qDebug()<<__FUNCTION__<<row;

	assert( ! parent.isValid() ) ;

	emit layoutAboutToBeChanged ();
	beginInsertRows( parent , row , row+count -1 );//////////

	int insCurrRow = row ;
	for( int c = 0 ; c < count ; c ++)
	{
		QSqlRecord rec ;

		for( int i = 0 ; i < this->mSegmentsTableColumns.count() ; i ++)
		{
			QSqlField currField;
			currField.setName( this->mSegmentsTableColumns.at(i) );
			currField.setValue(QVariant());
			rec.append(currField);
		}
		//this->mModelData.append(rec);
		this->mModelData.insert(row+c,rec);
	}
	endInsertRows();//////////

	return true ;
}

bool SqliteSegmentModel::removeRows ( int row, int count, const QModelIndex & parent  )
{
	//qDebug()<<__FUNCTION__<<row;

	int parentCatID = parent.internalId();
	int atrow = 0 ;
	int delete_begin = row ;
	int delete_end = row + count - 1 ;

	beginRemoveRows  ( parent, delete_begin, delete_end ) ;	

	for( int i = delete_end  ; i >= delete_begin  ; i -- )
	{
		int taskID = this->mModelData.at(i).value("task_id").toInt();
		int segID = this->mModelData.at(i).value("seg_id").toInt();
		this->mModelData.remove(row);
		this->mStorage->deleteSegment(taskID,segID);
		emit layoutChanged () ;	//这是必须，否则视图不能正常画出模型。
	}

	endRemoveRows () ;

	return true ;
}

bool SqliteSegmentModel::setData ( const QModelIndex & index , const QVariant & value, int role )
{
	//qDebug()<<__FUNCTION__<<index.row()<<index.column()<<index.data();

	int row = index.row();
	int col = index.column();

	if( col == this->mSegmentsTableColumns.count()-1 )
	{
		
	}
	//else if( col == ng::segs::abtained_length )
	//{

	//}
	else
	{
		this->mModelData[row].setValue(col,value);
		//qDebug()<<row<<":"<<col <<" " << value ;
	}
	this->mModelData[row].setValue(this->mSegmentsTableColumns.count()-1,"true");
	emit dataChanged(index,index);
	//this->reset();	//reset 比　layoutChanged()效率好一点，但是界面闪烁的很厉害
	//this->changePersistentIndex(index,index);
	if( col == ng::segs::abtained_length )	
	{
		//this->data(index);
		//emit layoutChanged();		//这个效率太差了。
	}
	
	return true ;
}


bool SqliteSegmentModel::submit () 
{
	//把已经修改或者添加了的数据写入到数据库中。
	int dirtycnt = 0 ;
	int rowcnt = this->mModelData.count();

	for( int i = rowcnt - 1 ; i >=0 ; i -- )
	{		
		if(this->mModelData.at(i).value("dirty").toString() == "true")
		{
			dirtycnt ++;
			QSqlRecord rec = this->mModelData.at(i);

			QString   seg_id = rec.value("seg_id").toString();
			QString   task_id = rec.value("task_id").toString();

			QString start_offset = rec.value("start_offset").toString();
			QString create_time = rec.value("create_time").toString();
			QString finish_time = rec.value("finish_time").toString();
			QString total_length = rec.value("total_length").toString();
			QString abtained_length = rec.value("abtained_length").toString();
			QString current_speed = rec.value("current_speed").toString();
			QString average_speed = rec.value("average_speed").toString();
			QString abtained_percent = rec.value("abtained_percent").toString();
			QString segment_status = rec.value("segment_status").toString();
			QString total_packet = rec.value("total_packet").toString();
			QString abtained_packet = rec.value("abtained_packet").toString();
			QString left_packet = rec.value("left_packet").toString();
			QString total_timestamp = rec.value("total_timestamp").toString();
			QString finish_timestamp = rec.value("finish_timestamp").toString();
			QString left_timestamp = rec.value("left_timestamp").toString();
			QString dirty = rec.value("dirty").toString();

			if( this->mStorage->containsSegment(task_id.toInt(),seg_id.toInt() ))
			{
				//update 
				this->mStorage->updateSegment( seg_id.toInt() ,  task_id.toInt() ,  start_offset,create_time,finish_time,total_length,abtained_length,current_speed,average_speed,abtained_percent,segment_status,total_packet,abtained_packet,left_packet,total_timestamp,finish_timestamp,left_timestamp,dirty   );
			}
			else
			{
				//insert
				this->mStorage->addSegment(  seg_id.toInt() ,  task_id.toInt() ,  start_offset,create_time,finish_time,total_length,abtained_length,current_speed,average_speed,abtained_percent,segment_status,total_packet,abtained_packet,left_packet,total_timestamp,finish_timestamp,left_timestamp,dirty  );
			}

			this->mModelData[i].setValue("dirty","false");	//清除dirty 标记

		}
	}

	qDebug()<<"there is about "<< dirtycnt <<" dirty rows to submit "<< this->mModelData.count() ;

	return true ;
}

void SqliteSegmentModel::revert()
{
	int dirtycnt = 0 ;
	int rowcnt = this->mModelData.count();
	for( int i = 0 ; i < rowcnt ; i ++ )
	{
		if( this->mModelData.at(i).contains("dirty") && this->mModelData.at(i).value("dirty").toString() == "true")
		{
			dirtycnt ++;
		}
	}

	qDebug()<<"there is about "<< dirtycnt <<" dirty rows to revert ";
	return ;
}

///////////////////////////////////////////
///
///////////////////////////////////////////
bool SqliteSegmentModel::sortMMList() 
{
	MPT mpt1,mpt2 , swap ;
	if( this->mPointerMap.count() == 1 ) 
	{
		return true ;	//no sort needed
	}
	for( int i = 0 ; i < this->mPointerMap.count()-1 ; i ++ )
	{
		mpt1= this->mPointerMap.at(i);
		mpt2 =this->mPointerMap.at(i+1);
		if( mpt1.SP > mpt2.SP )
		{
			swap = this->mPointerMap[i];
			this->mPointerMap[i] = this->mPointerMap[i+1] ;
			this->mPointerMap[i+1] = swap ;
		}
	}

	return true ;
}
/**
* -1 表示还没有设置最大长度。????
* 可能的返回值 0 ,无内存，>0 并且 <size ,没有总够的内存，=size有足够多的内存，
* 返回的值就是指针起点。
*/
quint64 SqliteSegmentModel::malloc( /*quint64 size*/ )
{
	quint64 malloced_pointer = MM_NOT_EXSIT ;
	int p1 = -1 , p2 = -1 ;
	quint64 maxInterval = 0 ;
	int splitElemIndex = 0 ;

	this->mAtomMutex.lock();

	if( this->mPointerMap.count() == 0 )
	{
		//will return MM_NOT_EXSIT
		this->mAtomMutex.unlock();
		return MM_NOT_EXSIT ;
	}
	else
	{
		this->sortMMList();	//先给这个进行排序

		for( int i = 0 ; i < this->mPointerMap.count() ; i ++ )
		{
			MPT mpt = this->mPointerMap.at( i ) ;
			if( mpt.busy == false )
			{				
				//break ;//也许可以再对它进行分割
				if( i == this->mPointerMap.count()-1)
				{
					if( this->mMemCapacity - mpt.CP -mpt.SP  > maxInterval)
					{
						maxInterval = this->mMemCapacity - mpt.CP -mpt.SP ;
						splitElemIndex = i ;
					}
					else
					{
						//不需要处理
					}
				}
				else
				{	//将下一个的开始位置与当前的元素的当前指针位置进行比较
					MPT tmp_mpt = this->mPointerMap.at(i+1);
					if( tmp_mpt.SP - mpt.CP -mpt.SP  > maxInterval )
					{
						maxInterval = tmp_mpt.SP - mpt.CP - mpt.SP  ;
						splitElemIndex = i ;
					}
					else
					{
						//不需要处理
					}
				}				
			}
			else	//busy,也要处理啊。不过可能注意not busy 的要有优先权,现在先做成一样吧
			{
				//break ;//也许可以再对它进行分割
				if( i == this->mPointerMap.count()-1)
				{
					if( this->mMemCapacity - mpt.CP  -mpt.SP > maxInterval)
					{
						maxInterval = this->mMemCapacity - mpt.CP - mpt.SP ;
						splitElemIndex = i ;
					}
					else
					{
						//不需要处理
					}
				}
				else
				{	//将下一个的开始位置与当前的元素的当前指针位置进行比较
					MPT tmp_mpt = this->mPointerMap.at(i+1);
					if( tmp_mpt.SP - mpt.CP  -mpt.SP > maxInterval )
					{
						maxInterval = tmp_mpt.SP - mpt.CP - mpt.SP ;
						splitElemIndex = i ;
					}
					else
					{
						//不需要处理
					}
				}
			}	// end else	//busy
		}	// end for( int i = 0 ; i < this->mPointerMap.count() ; i ++ )
		//找第一个点
	}	// end else if( this->mPointerMap.count() == 0 )

	if( splitElemIndex >= 0 && maxInterval > 100*1024 )	//有需要分割的段
	{
		MPT mpt = this->mPointerMap.at(splitElemIndex);
		MPT new_mpt ;
		new_mpt.busy = true ;
		new_mpt.CP = 0 ;

		quint64 outLength = maxInterval/2  ;
		if( this->mPointerMap.at(splitElemIndex).busy == false)
		{
			new_mpt.SP = mpt.SP +mpt.CP ;
		}
		else
		{
			if( splitElemIndex == this->mPointerMap.count()-1)
			{
				new_mpt.SP = this->mMemCapacity - outLength ;
			}
			else
			{
				new_mpt.SP = this->mPointerMap.at(splitElemIndex+1).SP - outLength ;
			}
		}
		//
		if( this->mPointerMap.at(splitElemIndex).busy == false  
			&& mpt.CP == 0 )
		{
			malloced_pointer = new_mpt.SP ;
		}
		else
		{
			malloced_pointer = new_mpt.SP ;
			this->mPointerMap.insert(splitElemIndex+1,new_mpt);
			//int rowCnt = this->rowCount() ;
			int rowCnt = splitElemIndex+1 ;
			this->insertRows(rowCnt,1);
			this->setData(this->index(rowCnt,ng::segs::seg_id),new_mpt.SP);
			this->setData(this->index(rowCnt,ng::segs::start_offset),new_mpt.SP);
			this->setData(this->index(rowCnt,ng::segs::task_id),this->mTaskId);
			this->setData(this->index(rowCnt,ng::segs::segment_status),new_mpt.busy);
			this->setData(this->index(rowCnt,ng::segs::abtained_length),new_mpt.CP );
		}
	}
	else if( splitElemIndex >= 0 
		&& this->mPointerMap.at(splitElemIndex).busy == false )	//有停止了的块
	{
		MPT mpt = this->mPointerMap.at(splitElemIndex);
		MPT new_mpt ;
		new_mpt.busy = true ;
		new_mpt.CP = 0 ;

		new_mpt.SP = mpt.SP + mpt.CP ;

		//
		if( this->mPointerMap.at(splitElemIndex).busy == false  
			&& mpt.CP == 0 )
		{
			malloced_pointer = new_mpt.SP ;
		}
		else
		{
			malloced_pointer = new_mpt.SP ;
			this->mPointerMap.insert(splitElemIndex+1,new_mpt);
			//int rowCnt = this->rowCount() ;
			int rowCnt = splitElemIndex+1 ;
			this->insertRows(rowCnt,1);
			this->setData(this->index(rowCnt,ng::segs::seg_id),new_mpt.SP);
			this->setData(this->index(rowCnt,ng::segs::start_offset),new_mpt.SP);
			this->setData(this->index(rowCnt,ng::segs::task_id),this->mTaskId);
			this->setData(this->index(rowCnt,ng::segs::segment_status),new_mpt.busy);
			this->setData(this->index(rowCnt,ng::segs::abtained_length),new_mpt.CP );
		}
	}
	else
	{
		malloced_pointer = MM_NULL ;
		
	}
	this->mAtomMutex.unlock();

	return malloced_pointer ;
	return 0;
}

bool SqliteSegmentModel::removeZeroAndNotBusyPointer()
{
	
	return true ;
}

/**
* 可能的返回值　0　，　数据已经释放过了，表示内存释放完成，
* >0 并且 <size 部分数据释放过了，也表示内存释放完成
* == size ，内存释放正常，可能还有内存，需要继承试着翻译内存。
*/
quint64 SqliteSegmentModel::free( quint64 SP , quint64 size  )
{
	quint64 free_length = size ;
	bool found = false ;
	//qDebug()<<__FUNCTION__<<__LINE__<<" "<< "SP:"<< SP <<" size:"<<size;

	this->mAtomMutex.lock();

	for( int i = 0 ; i < this->mPointerMap.count() ; i ++ )
	{
		if( this->mPointerMap.at(i).SP == SP )
		{
			if( i == this->mPointerMap.count() -1 )
			{
				if( this->mPointerMap[i].SP + this->mPointerMap[i].CP + size  > this->mMemCapacity )
				{
					this->mPointerMap[i].CP = this->mMemCapacity - this->mPointerMap[i].SP;
					free_length = this->mMemCapacity - this->mPointerMap[i].SP - this->mPointerMap[i].CP ;					
				}
				else
				{
					this->mPointerMap[i].CP += size  ;					
					free_length = size ;
				}
			}
			else
			{
				if( this->mPointerMap[i].SP + this->mPointerMap[i].CP + size  > 
					this->mPointerMap[i+1].SP )
				{
					this->mPointerMap[i].CP = this->mPointerMap[i+1].SP - this->mPointerMap[i].SP;

					free_length = this->mPointerMap[i+1].SP - this->mPointerMap[i].SP - this->mPointerMap[i].CP ;
				}
				else
				{
					this->mPointerMap[i].CP += size  ;
					free_length = size ;
				}
			}
			//更新视图
			this->setData(this->index(i,ng::segs::abtained_length),this->mPointerMap[i].CP);
			
			//assert( this->mPointerMap[i].CP > this->mMemCapacity ) ;
			if( this->mPointerMap[i].CP > this->mMemCapacity )
			{
				qDebug()<<__FUNCTION__<<__LINE__<<" "<< "SP:"<< SP <<" size:"<<size
					<<" CP: "<< this->mPointerMap[i].CP <<" Cap:"<< this->mMemCapacity  ;
			}
			found = true ;
			break ;
		}//end if( this->mPointerMap.at(i).SP == SP )
	}
	
	this->mAtomMutex.unlock();

	assert( found == true ) ;

	if( free_length != size )
	{
		qDebug()<<"SP:"<<SP<<" free_length:"<< free_length << " size:"<< size ;
		for( int i = 0 ; i < this->mPointerMap .count() ; i ++ )
		{
			qDebug()<<"i :"<< i <<" \tSP:"<<this->mPointerMap.at(i).SP
				
				<<" \tCP:"<<this->mPointerMap.at(i).CP
				<<" \tbusy:"<<this->mPointerMap.at(i).busy ;
		}
	}
	assert( this->mMemCapacity > 0 );

	return free_length ;
	return 0;
}

/**
* 
*/
quint64 SqliteSegmentModel::release( quint64 SP ) 
{
	bool found = false ;

	this->mAtomMutex.lock();
	for( int i = 0 ; i < this->mPointerMap.count() ; i ++ )
	{
		if( this->mPointerMap.at(i).SP == SP )
		{
			this->mPointerMap[i].busy = false ;
			break ;
		}
	}
	this->mAtomMutex.unlock();
	return 0;
}

quint64 SqliteSegmentModel::setCapacity(quint64 capacity )
{
	qDebug()<<__FUNCTION__<<__LINE__<<" "<< capacity ;
	//assert( capacity > 0 ) ;	
	if( capacity == 0 )
	{
		assert( this->mPointerMap.count() > 0 ) ;
	}

	if( this->mPointerMap.count() > 0 )
	{
		qDebug()<< " maybe already set omited " ;
		return 0 ;	//表示有问题
	}
	//
	this->mMemCapacity = capacity ;
	mMaxBlock= 30 ;
	MPT mpt ;
	mpt.busy = true ;
	mpt.SP = 0;
	mpt.CP = 0;

	assert( this->mPointerMap.count() == 0 ) ;
	this->mPointerMap.insert(0,mpt);

	this->insertRows(0,1);
	this->setData(this->index(0,ng::segs::seg_id),mpt.SP);
	this->setData(this->index(0,ng::segs::start_offset),mpt.SP);
	this->setData(this->index(0,ng::segs::task_id),this->mTaskId);
	this->setData(this->index(0,ng::segs::segment_status),mpt.busy);
	this->setData(this->index(0,ng::segs::abtained_length),mpt.CP );

	return capacity ;
	return 0;
}

bool SqliteSegmentModel::isMemFullAlloced()
{
	//对每一块的CP+SP都等于下一块的SP
	bool full = false ;
	this->sortMMList();
	for(int i = 0 ; i < this->mPointerMap.count() ; i ++ )
	{
		MPT mpt1,mpt2 ;
		mpt1.SP = this->mPointerMap.at(i).SP;
		mpt1.CP = this->mPointerMap.at(i).CP;
		mpt1.busy = this->mPointerMap.at(i).busy ;
		if( i== this->mPointerMap.count()-1)
		{
			mpt2.SP = this->mMemCapacity ;
			mpt2.CP = 0 ;
			mpt2.busy = false ;
		}
		else
		{
			mpt2.SP = this->mPointerMap.at(i+1).SP;
			mpt2.CP = this->mPointerMap.at(i+1).CP;
			mpt2.busy = this->mPointerMap.at(i+1).busy ;
		}
		
		if( mpt1.SP + mpt1.CP == mpt2.SP )
		{
			full = true ;
			continue;
		}
		else
		{
			full = false ;
			break ;
		}
	}

	for( int i = 0 ; i < this->mPointerMap .count() ; i ++ )
	{
		qDebug()<<"i :"<< i <<" \tSP:"<<this->mPointerMap.at(i).SP

			<<" \tCP:"<<this->mPointerMap.at(i).CP
			<<" \tbusy:"<<this->mPointerMap.at(i).busy ;
	}
	if( full == true )
	{
		//emit memoryOverLoad();
	}
	return full ;
	return false ;
}

void  SqliteSegmentModel::setBlockBusy(quint64 SP, bool busy )
{
	this->mAtomMutex.lock();
	for(int i = 0 ; i < this->mPointerMap.count() ; i ++ )
	{
		if(this->mPointerMap.at(i).SP == SP )
		{
			this->mPointerMap[i].busy = busy ;
			this->setData(this->index(i,ng::segs::segment_status),busy?"true":"false");
		}
	}
	this->mAtomMutex.unlock();
}








