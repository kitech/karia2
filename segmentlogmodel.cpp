#include <cassert>

#include <QtCore>
#include <QStringList>
#include <QImage>


#include "segmentlogmodel.h"
#include "sqlitestorage.h"

//static 
QMap< QString , SegmentLogModel*> SegmentLogModel::mHandle ;

//static
SegmentLogModel * SegmentLogModel::instance ( int task_id , int seg_id  , QObject *parent )
{
	SegmentLogModel * tm = 0 ;
	QString key("%1,%2") ;
	key = key.arg(task_id).arg(seg_id);

	if( SegmentLogModel::mHandle.contains(key) )
	{
		tm = SegmentLogModel::mHandle.value(key);
	}
	else
	{
		//tm = new SegmentLogModel(task_id,seg_id , parent);
		tm = new SegmentLogModel(task_id,seg_id , 0);//把它变成没归属的孩子，那么在父构件注销时不会影响到它。
		SegmentLogModel::mHandle.insert(key , tm );
	}

	return tm ;
}

//static 
bool SegmentLogModel::removeInstance(int task_id , int seg_id )
{
	QString key("%1,%2") ;
	key = key.arg(task_id).arg(seg_id);

	if( SegmentLogModel::mHandle.contains(key) )
	{
		SegmentLogModel::mHandle.remove(key);
		return true ;
	}
	else
	{
		return false ;
	}	
	return false ;
}


SegmentLogModel::SegmentLogModel(int task_id , int seg_id ,QObject *parent)
	: QAbstractItemModel(parent)
{
	logCols = QT_TR_NOOP("log_type,add_time,log_content,task_id,seg_id,dirty");
	this->mColumnsLine = logCols ;
	QStringList sl = this->mColumnsLine.split(",");
	for( int i =0 ;i < sl.count() ; i ++ )
	{
		this->mLogColumns.append(sl.at(i));
	}
	
}

SegmentLogModel::~SegmentLogModel()
{
	this->mModelData.clear();
}



int SegmentLogModel::columnCount(const QModelIndex &/*parent*/) const
{
	//return this->mModelData.at(0).count() ;
	int cc = this->mLogColumns.count();

	return cc ;
	return 0;
}
QVariant SegmentLogModel::data(const QModelIndex &index, int role) const
{
	//qDebug()<<__FUNCTION__;
	if (!index.isValid())
		return QVariant();


	if (role != Qt::DisplayRole && role != Qt::DecorationRole)
		return QVariant();

	int col = index.column();
	int row = index.row();

	QVariant var = this->mModelData.at(row).at(col);

	QString iconPath = "icons/crystalsvg/32x32/actions/";
	if( role == Qt::DecorationRole && index.column() == 0 )	//只有第一列显示图片就可以了。
	{
		
		switch(var.toInt())
		{
		case ng::logs::DEBUG_MSG:
			return QImage(iconPath + "toggle_log.png").scaled(QSize(16,16)); break;
		case ng::logs::DOWN_MSG:
			return QImage(iconPath + "down.png").scaled(QSize(16,16)); break;
		case ng::logs::ERROR_MSG:
			return QImage(iconPath + "messagebox_critical.png").scaled(QSize(16,16)); break;
		case ng::logs::INFO_MSG:
			return QImage(iconPath + "messagebox_info.png").scaled(QSize(16,16)); break;
		case ng::logs::UP_MSG:
			return QImage(iconPath + "up.png").scaled(QSize(16,16)); break;
		case ng::logs::USER_MSG:
			return QImage(iconPath + "presence_online.png").scaled(QSize(16,16)); break;
		default:
			return QImage("icons/crystalsvg/16x16/filesystems/folder_violet_open.png");
		}		
	}

	return var ;
	return QVariant() ;
}

Qt::ItemFlags SegmentLogModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SegmentLogModel::headerData(int section, Qt::Orientation orientation, int role) const
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
					return QString(tr(logCols)).split(",").at(section);
					//return this->mLogColumns.at(section);
					return QVariant();
		}
	}

	return QVariant();
}

QModelIndex SegmentLogModel::index(int row, int column, const QModelIndex &parent)   const
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

QModelIndex SegmentLogModel::parent(const QModelIndex &child) const
{
	//qDebug()<<__FUNCTION__ ;
	
	if (!child.isValid())
		return QModelIndex();

	//int childCatID = child.internalId();
	//int parentCatID = -1 ;
	//int ppCatID = -1 ;	// parent's parent cat ID
	//int atrow = 0 ;
	//for( int i = 0 ; i < this->mModelData.count() ; i ++ )
	//{
	//	if( this->mModelData.at(i).value("cat_id").toInt() == childCatID )
	//	{
	//		//parentCatID = this->mModelData.at(i).value("cat_id").toInt() ;
	//		parentCatID = this->mModelData.at(i).value("parent_cat_id").toInt() ;
	//	}
	//}

	////qDebug()<<__FUNCTION__<<":"<<__LINE__<<":"<<parentCatID<<" is parent of "<< childCatID ;

	//int parentRow = 0 ;
	//if( parentCatID == -1 )
	//{
	//	parentRow = 0 ;
	//	return QModelIndex();
	//}
	//else
	//{		
	//	//查找ppCatID 的值
	//	//找　parent 的 parent 的ID
	//	for( int i = 0 ; i < this->mModelData.count() ; i ++ )
	//	{
	//		if( this->mModelData.at(i).value("cat_id").toInt() == parentCatID )
	//		{				
	//			ppCatID = this->mModelData.at(i).value("parent_cat_id").toInt() ;
	//		}
	//	}

	//	//////////
	//	atrow = 0 ;
	//	for( int i = 0 ; i < this->mModelData.count() ; i ++ )
	//	{			
	//		if( this->mModelData.at(i).value("parent_cat_id").toInt() == ppCatID  )
	//		{				
	//			if( this->mModelData.at(i).value("cat_id").toInt() == parentCatID )
	//			{					
	//				break ;	// counter 就是 parentCatID 在 ppCatID 分类中的行号
	//			}
	//			////
	//			atrow += 1 ;
	//		}
	//	}
	//	
	//}

	////qDebug()<<__FUNCTION__<<":"<<__LINE__<<":"<<ppCatID << "  "<<parentCatID<<" is parent of "
	////	<< childCatID <<" at row "<< atrow ;

	//return createIndex( atrow , 0 , parentCatID );

	return QModelIndex();
}

int SegmentLogModel::rowCount(const QModelIndex &parent) const
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
	//else
	//{
	//	int parentCatID = -1;
	//	parentCatID = parent.internalId() ;
	//	for( int i = 0 ; i < this->mModelData.count() ; i ++ )
	//	{
	//		if( this->mModelData.at(i).value("parent_cat_id").toInt() == parentCatID )
	//		{
	//			count += 1; 
	//			//qDebug()<<__FUNCTION__<<":"<<__LINE__<<":"<<parentCatID <<this->mModelData.at(i).value("parent_cat_id").toInt()  ;
	//		}
	//	}
	//}

	////qDebug()<<__FUNCTION__ <<":"<< count ;
	return count ;
	return 0;
}

bool SegmentLogModel::insertRows ( int row, int count, const QModelIndex & parent  )
{
	//qDebug()<<__FUNCTION__<<row;

	assert( ! parent.isValid() ) ;

	beginInsertRows( parent , row , row+count -1 );//////////

	for( int c = 0 ; c < count ; c ++)
	{
		//QSqlRecord rec ;
		QVector<QVariant> rec(this->mLogColumns.count());	//初始化为适当的列数

		//for( int i = 0 ; i < this->mLogColumns.count() ; i ++)
		//{
		//	QSqlField currField;
		//	currField.setName( this->mLogColumns.at(i) );
		//	currField.setValue(QVariant());
		//	rec.append(currField);
		//}

		this->mModelData.append(rec);
	}
	endInsertRows();//////////

	return true ;
}

bool SegmentLogModel::removeRows ( int row, int count, const QModelIndex & parent  )
{
	//qDebug()<<__FUNCTION__<<row;

	int parentCatID = parent.internalId();
	int atrow = 0 ;
	int delete_begin = row ;
	int delete_end = row + count - 1 ;

	beginRemoveRows  ( parent, delete_begin, delete_end ) ;	

	for( int i = delete_end  ; i >= delete_begin  ; i -- )
	{
		this->mModelData.remove(row);
		//this->mModelData.remove(row,count); 这个更简单一点啊。
		emit layoutChanged () ;	//这是必须，否则视图不能正常画出模型。
	}

	endRemoveRows () ;

	return true ;
}

bool SegmentLogModel::setData ( const QModelIndex & index , const QVariant & value, int role )
{
	//qDebug()<<__FUNCTION__<<index.row()<<index.column()<<index.data();

	int row = index.row();
	int col = index.column();

	if( col == this->mLogColumns.count()-1 )
	{
		
	}
	else
	{
		//this->mModelData[row].setValue(col,value);
		this->mModelData[row][col] = (value);
		//this->mModelData[row][col] = (value);
		//qDebug()<<row<<":"<<col <<" " << value ;
	}
	//this->mModelData[row].setValue(this->mLogColumns.count()-1,"true");
	this->mModelData[row][ this->mLogColumns.count()-1] = ("true");
	emit dataChanged(index,index);

	return true ;
}


bool SegmentLogModel::submit () 
{
	//把已经修改或者添加了的数据写入到数据库中。
	int dirtycnt = 0 ;
	int rowcnt = this->mModelData.count();

	for( int i = rowcnt - 1 ; i >=0 ; i -- )
	{
		//if(this->mModelData.at(i).value("dirty").toString() == "true")
		if(this->mModelData.at(i).at(this->mLogColumns.count()-1).toString() == "true")
		{
			dirtycnt ++;
			//QSqlRecord rec = this->mModelData.at(i);

			//QString   seg_id = rec.value("seg_id").toString();
			//QString   task_id = rec.value("task_id").toString();
			//QString   startoffset = rec.value("startoffset").toString();
			//QString   totallength = rec.value("totallength").toString();
			//QString   validlength = rec.value("validlength").toString();
			//QString   gotlength = rec.value("gotlength").toString();
			//QString   createtime = rec.value("createtime").toString();
			//QString   completetime = rec.value("completetime").toString();
			//QString   dirty = rec.value("dirty").toString() ;

			//if( this->mStorage->containsSegment(task_id.toInt(),seg_id.toInt() ))
			//{
			//	//update 
			//	this->mStorage->updateSegment( seg_id.toInt() ,  task_id.toInt() ,  startoffset ,  createtime , 
			//					 completetime ,  totallength ,  validlength , 
			//					 gotlength  );
			//}
			//else
			//{
			//	//insert
			//	this->mStorage->addSegment(  seg_id.toInt() ,  task_id.toInt() ,  startoffset ,  createtime , 
			//					 completetime ,  totallength ,  validlength , 
			//					 gotlength  );
			//}
			
			//this->mModelData[i].setValue("dirty","false");	//清除dirty 标记
			this->mModelData[i][this->mLogColumns.count()-1] = ("false");	//清除dirty 标记
		}
	}

	qDebug()<<"there is about "<< dirtycnt <<" dirty rows to submit "<< this->mModelData.count() ;

	return true ;
}

void SegmentLogModel::revert()
{
	int dirtycnt = 0 ;
	int rowcnt = this->mModelData.count();
	for( int i = 0 ; i < rowcnt ; i ++ )
	{
		//if( this->mModelData.at(i).contains("dirty") && this->mModelData.at(i).value("dirty").toString() == "true")
		if( this->mModelData.at(i).contains("dirty") 
			&& this->mModelData.at(i).at(this->mLogColumns.count()-1).toString() == "true")
		{
			dirtycnt ++;
		}
	}

	qDebug()<<"there is about "<< dirtycnt <<" dirty rows to revert ";
	return ;
}







