// sqlitecategorymodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-14 16:23:47 +0800
// Version: $Id$
// 

#include "sqlitestorage.h"

#include "sqlitecategorymodel.h"

/////////////////////////////////////
//
/////////////////////////////////////

SqliteCategoryModel * SqliteCategoryModel::mHandle = 0 ;

SqliteCategoryModel * SqliteCategoryModel::instance(QObject *parent) 
{
	if (SqliteCategoryModel::mHandle == 0 ) {
		SqliteCategoryModel::mHandle = new SqliteCategoryModel(parent);
	}

	return SqliteCategoryModel::mHandle;
}

SqliteCategoryModel::SqliteCategoryModel( QObject *parent)
  : QAbstractItemModel(parent) 
{
	mStorage = new SqliteStorage(parent);
	this->mStorage->open();
	mModelData = mStorage->getCatSet();
	mCatsTableColumns = mStorage->getCatsColumns();

}


SqliteCategoryModel::~SqliteCategoryModel()
{
	this->mStorage->close();
	delete this->mStorage;
	this->mModelData.clear();
}

bool  SqliteCategoryModel::reload()	//重新从配置文件中读取
{
	//mConfigDB = ConfigDatabase::instance();

	//catRootNode = mConfigDB->getCategoryTree(catRootNode);

	//DomItem *tmpRoot = rootItem ;
	//      rootItem = new DomItem( catRootNode , 0);
	//delete tmpRoot ; tmpRoot = 0 ;

	return true ;
}

int SqliteCategoryModel::getCatIdByModel(QModelIndex &index)
{
    if (!index.isValid()) {
        return -1;
    } else {
        return index.internalId();
    }
}

int SqliteCategoryModel::columnCount(const QModelIndex &/*parent*/) const
{
	if (this->mModelData.size() == 0) {
		int cols = ng::cats::dirty;
		return cols;
	} else {
		return this->mModelData.at(0).count() ;
	}
	return 3;
	// return 3 ;
}
QVariant SqliteCategoryModel::data(const QModelIndex &index, int role) const
{
	//qDebug()<<__FUNCTION__;
	if (!index.isValid())
		return QVariant();
	if (role == Qt::DecorationRole && index.column() == 0 ) { //只有第一列显示图片就可以了。
		return QImage(qApp->applicationDirPath()+"/"+"icons/crystalsvg/16x16/filesystems/folder_violet_open.png");
	}

	if (role != Qt::DisplayRole)
		return QVariant();

	//    DomItem *item = static_cast<DomItem*>(index.internalPointer());

	//    QDomNode node = item->node();
	//    QStringList attributes;
	//    QDomNamedNodeMap attributeMap = node.attributes();

	//    switch (index.column()) {
	//        case 0:
	//            //return node.nodeName();
	//return node.toElement().attribute("name");
	//        case 2:
	//            for (int i = 0; (unsigned int)(i) < attributeMap.count(); ++i) {
	//                QDomNode attribute = attributeMap.item(i);
	//                attributes << attribute.nodeName() + "=\""
	//                              +attribute.nodeValue() + "\"";
	//            }
	//            return attributes.join(" ");
	//        case 1:
	//            //return node.nodeValue().split("\n").join(" ");
	//return node.toElement().attribute("path");
	//        default:
	//            return QVariant();
	//    }

	int col = index.column();
	int row = index.row();

	int catID = index.internalId();
	
	QModelIndex pmi = index.parent();
	int pcol , prow ;

	if (! pmi.isValid()) {
		QSqlRecord rec ;
		for (int i = 0 ; i < this->mModelData.count() ; i ++ ) {
			rec = this->mModelData.at(i);
			if (rec.value("cat_id") == "0") {
				QVariant rv = QVariant();
				if (col == 0) {
					rv = rec.value("display_name");
				} else if (col == 1 ) {
					rv = rec.value("cat_id");
				}
				//else if ( col == 2 )
				//{
					//rv = rec.value("display_name").toString() + "=" + rec.value("path").toString(); 
					//rv = rec.value("cat_id");
				//}
				else {
					rv = rec.value(col);
				}
				return rv;
				//break ;
			} else {
				//rec.clear():
			}
		} // end for
	}  // end if (! pmi.isValid() )
	else {
		for (int i = 0 ; i < this->mModelData.count() ; i ++) {
			if (this->mModelData.at(i).value("cat_id").toInt() == catID) {
				QSqlRecord rec = this->mModelData.at(i) ;
				return rec.value(index.column());
			}
		}
	}

	qDebug()<<" model error";
	assert(1 == 2);
	return QVariant();
}

Qt::ItemFlags SqliteCategoryModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SqliteCategoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	//qDebug()<<__FUNCTION__ << rand() ;
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
		switch (section) {
				//case 0:
				//	return tr("Cat Name");
				//case 1:
				//	return tr("Cat Path");
				//case 2:
				//	return tr("Cat Attributes");
				default:
					return mStorage->getCatsColumns().at(section) ;
					//return this->mModelData.at(0).fieldName(section) ;
					return QVariant();
		}
	}

	return QVariant();
}

QModelIndex SqliteCategoryModel::index(int row, int column, const QModelIndex &parent)   const
{
	//qDebug()<<__FUNCTION__ ;

	//DomItem *parentItem;
	
	//if (!parent.isValid())
	//    parentItem = rootItem;
	//else
	//    parentItem = static_cast<DomItem*>(parent.internalPointer());

	//DomItem *childItem = parentItem->child(row);
	//if (childItem)
	//    return createIndex(row, column, childItem);
	//else
	//    return QModelIndex();

	//DomItem *parentItem;
	int parentCatID = -1 ;

	if (!parent.isValid())
	    parentCatID = -1 ;
	else
		parentCatID = parent.internalId(); //static_cast<int>(parent.internalPointer());

	//qDebug()<< __FUNCTION__ << parentCatID ;

	//DomItem *childItem = parentItem->child(row);
	int childCatID = -1 ;
	int counter = 0 ;
	//查找在第row行的子类
	for (int i = 0 ; i < this->mModelData.count() ;i ++ )
	{
		QSqlRecord rec = this->mModelData.at(i);
		if (rec.value("parent_cat_id").toInt() == parentCatID )
		{			
			if (row == counter )
			{
				childCatID = rec.value("cat_id").toInt();
				//qDebug()<< __FUNCTION__ << parentCatID <<" is parent of "<<childCatID ;
				break ;
			}
			counter += 1 ;
		}
	}
	//qDebug()<< __FUNCTION__ << row <<" "<<column <<" " << parentCatID <<" is parent of "<<childCatID ;
	//
	if ( childCatID != -1 )
	    return createIndex( row , column , childCatID );
	else
	    return QModelIndex();

	return QModelIndex();

}

QModelIndex SqliteCategoryModel::parent(const QModelIndex &child) const
{
	//qDebug()<<__FUNCTION__ ;
	
	if (!child.isValid())
		return QModelIndex();

	int childCatID = child.internalId();
	int parentCatID = -1 ;
	int ppCatID = -1 ;	// parent's parent cat ID
	int atrow = 0 ;
	for (int i = 0 ; i < this->mModelData.count() ; i ++ )
	{
		if (this->mModelData.at(i).value("cat_id").toInt() == childCatID )
		{
			//parentCatID = this->mModelData.at(i).value("cat_id").toInt() ;
			parentCatID = this->mModelData.at(i).value("parent_cat_id").toInt() ;
		}
	}
	//DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
	//DomItem *parentItem = childItem->parent();

	//if (!parentItem || parentItem == rootItem)
	//    return QModelIndex();

	//qDebug()<<__FUNCTION__<<":"<<__LINE__<<":"<<parentCatID<<" is parent of "<< childCatID ;

	int parentRow = 0 ;
	if (parentCatID == -1 )
	{
		parentRow = 0 ;
		return QModelIndex();
	}
	else
	{		
		//查找ppCatID 的值
		//找　parent 的 parent 的ID
		for (int i = 0 ; i < this->mModelData.count() ; i ++ )
		{
			if (this->mModelData.at(i).value("cat_id").toInt() == parentCatID )
			{				
				ppCatID = this->mModelData.at(i).value("parent_cat_id").toInt() ;
			}
		}

		//////////
		atrow = 0 ;
		for (int i = 0 ; i < this->mModelData.count() ; i ++ )
		{			
			if (this->mModelData.at(i).value("parent_cat_id").toInt() == ppCatID  )
			{				
				if (this->mModelData.at(i).value("cat_id").toInt() == parentCatID )
				{					
					break ;	// counter 就是 parentCatID 在 ppCatID 分类中的行号
				}
				////
				atrow += 1 ;
			}
		}
		
	}

	//qDebug()<<__FUNCTION__<<":"<<__LINE__<<":"<<ppCatID << "  "<<parentCatID<<" is parent of "
	//	<< childCatID <<" at row "<< atrow ;

	return createIndex( atrow , 0 , parentCatID );

	return QModelIndex();
}

int SqliteCategoryModel::rowCount(const QModelIndex &parent) const
{
	//qDebug()<<__FUNCTION__ ;
	//DomItem *parentItem;

	//if (!parent.isValid())
	//    parentItem = rootItem;
	//else
	//    parentItem = static_cast<DomItem*>(parent.internalPointer());

	//return parentItem->node().childNodes().count();

	int count = 0 ;
	if (! parent.isValid() )
	{
		count =  1 ;
	}
	else
	{
		int parentCatID = -1;
		parentCatID = parent.internalId() ;
		for (int i = 0 ; i < this->mModelData.count() ; i ++ )
		{
			if (this->mModelData.at(i).value("parent_cat_id").toInt() == parentCatID )
			{
				count += 1; 
				//qDebug()<<__FUNCTION__<<":"<<__LINE__<<":"<<parentCatID <<this->mModelData.at(i).value("parent_cat_id").toInt()  ;
			}
		}
		//QModelIndex mi = this->index(parent.row(),2,parent.parent());
		//if (mi.data() == "0" )
		//{
		//	for (int i = 0 ; i < this->mModelData.count() ; i ++ )
		//	{
		//		qDebug()<<__FUNCTION__<<":"<<__LINE__<<":"<<count <<":data:"<< mi.data().toString() ;
		//		if (this->mModelData.at(i).value("cat_id") == "0" )
		//		{
		//			count ++ ;
		//		}
		//	}

		//	return count ;
		//}
		//return count ;
		//qDebug()<<__FUNCTION__<<":"<<__LINE__<<":"<<count <<":data:"<< mi.data().toString() ;
	}

	//qDebug()<<__FUNCTION__ <<":"<< count ;
	return count ;
	return 0;
}

bool SqliteCategoryModel::insertRows ( int row, int count, const QModelIndex & parent  )
{
	qDebug()<<__FUNCTION__<<row;

	//DomItem *item = static_cast<DomItem*>(parent.internalPointer());
	//QDomNode pnode = item->node();

	//QDomElement nelem ;
	//

	//nelem = this->mConfigDB->createNode("category").toElement() ;
	//qDebug()<<nelem.tagName();
	//nelem.setAttribute("folder","no");
	//pnode.appendChild(nelem);
	//
	//qDebug()<<item->row()<<nelem.tagName();
	//item->insertRows(count,new DomItem(nelem,row,item));

	//delete rootItem ;
	//rootItem = new DomItem( catRootNode , 0);
	
	int parentCatID = parent.internalId();
	QSqlRecord r0 = this->mModelData.at(0);
	int colcnt = r0.count();

    qDebug()<<r0;

	beginInsertRows( parent , row , row+count -1 );//////////

	for (int c = 0 ; c < count ; c ++ ) {
		QSqlRecord rec ;
		for (int n = 0 ; n < colcnt ; n ++ ) {
			QSqlField currField;
			currField.setName( r0.fieldName(n) );
			currField.setType( r0.field(n).type() ) ;
			currField.setValue(QVariant());

			if (r0.fieldName(n) == "cat_id") {
				int maxid = -1 ;
				int rowcnt = this->mModelData.count();
				for (int i = 0 ; i < rowcnt ; i ++ ) { //查找最大的cat id 号。
					//qDebug()<<"already existed rows:"<<this->mModelData.at(i);
					if (this->mModelData.at(i).value("cat_id").toInt() > maxid ) {
						maxid = this->mModelData.at(i).value("cat_id").toInt() ;
					}
				}
				maxid += 1 ;
				currField.setValue(QVariant(maxid));
			} else if (r0.fieldName(n) == "parent_cat_id" ) {				
				currField.setValue(QVariant(parentCatID));
			} else {
				//currField.setValue(QVariant());
				switch (n) {
                    // case 0:
                    // 	//this->mModelData[i].setValue(0,value);
                    // 	break;
                    // case 1:
                    // 	//this->mModelData[i].setValue(1,value);
                    // 	break;
                    // case 2:
                    // 	break;
                    // case 3:
                    // 	break;
				case ng::cats::can_child:
					currField.setValue("true");
					break;
                    //case 5:
					//currField.setValue(currField.value(1) );
					break;
				case ng::cats::folder:
					currField.setValue("no");
					break;
				case ng::cats::delete_flag:
					currField.setValue("false");
					break;
				case ng::cats::create_time:
					currField.setValue(QDateTime::currentDateTime().toString(Qt::SystemLocaleDate));
					break;
				case ng::cats::dirty:	//表示修改属性					
					currField.setValue(QVariant("true"));
					break ;
				default:
					//assert( 1== 2);
					currField.setValue(QVariant());
				}
			}
			rec.append( currField );
		}

        qDebug()<<rec;
		this->mModelData.append(rec);		
	}
	endInsertRows();//////////

	return true ;
}

bool SqliteCategoryModel::removeRows ( int row, int count, const QModelIndex & parent  )
{
	qDebug()<<__FUNCTION__<<row;

	//DomItem *item = static_cast<DomItem*>(parent.internalPointer());
	//DomItem *ditem ;
	//QDomNode pnode = item->node();
	//QDomNode cnode , dnode ;

	//dnode = item->child(row)->node();

	////delete this->rootItem;
	//qDebug()<<pnode.childNodes().count() ;
	//pnode.removeChild(dnode) ;
	//qDebug()<<pnode.childNodes().count();
	//item->removeRow(row);
	int parentCatID = parent.internalId();
	int atrow = 0 ;
	int delete_begin = row ;
	int delete_end = row + count - 1 ;

	int rowcnt = this->mModelData.count();

	qDebug()<<"parentid"<<parentCatID<<" delete begin to end "<< delete_begin << " --> "<< delete_end ;

	beginRemoveRows  ( parent, delete_begin, delete_end ) ;	

	//因为使用了递归，这个从小到大遍历的方式也是必须的。
	for (int i = 0 ; i < this->mModelData.count()  ; i ++ )
	{
		if (this->mModelData.at(i).value("parent_cat_id").toInt() == parentCatID )
		{	
			qDebug()<<"atrow="<<atrow;
			if (atrow >= delete_begin && atrow <= delete_end )
			{
				this->mModelData[i].setValue("delete_flag","true");
				this->mModelData[i].setValue("dirty","true");

				int cat_id = this->mModelData.at(i).value("cat_id").toInt();
				qDebug()<< "cat_id"<<cat_id;

				QModelIndex tempParentModel = this->index(atrow,0,parent);
				qDebug()<<" last "<<(tempParentModel.model()->rowCount());
				if (tempParentModel.model()->rowCount() > 0 )
				{
					this->removeRows(0,tempParentModel.model()->rowCount() ,tempParentModel );
				}
				this->mModelData.remove(i);
				this->mStorage->deleteCategory(cat_id,true);

				emit layoutChanged () ;	//这是必须，否则视图不能正常画出模型。

			}
			atrow ++ ;
		}
	}
	endRemoveRows () ;

	return true ;
}

bool SqliteCategoryModel::setData ( const QModelIndex & index , const QVariant & value, int role )
{
	qDebug()<<__FUNCTION__<<index.row()<<index.column()<<index.data();

	//return true ;

	//      DomItem *item = static_cast<DomItem*>(index.internalPointer());
	//QDomElement cnode = item->node().toElement() ;
	//qDebug()<<cnode.tagName() ;

	//switch( index.column() )
	//{
	//case 0:
	//	cnode.setAttribute("name",value.toString());
	//	break;
	//case 1 :
	//	cnode.setAttribute("path",value.toString());
	//}
	
	int parentCatID = index.parent().internalId();
	QSqlRecord r0 = this->mModelData.at(0);
	int colcnt = r0.count();
	int rowcnt = this->mModelData.count();

	//display_name ,  path , cat_id , parent_cat_id  , can_child , raw_name , folder, delete_flag , 
	//create_time  from categorys order by parent_cat_id , cat_id

	QModelIndex catModelIndexAtCurrentRow = this->index( index.row() , 2 , index.parent() );
	int currentCatID = catModelIndexAtCurrentRow.data().toInt();

	for (int i = 0 ; i < rowcnt ;i ++ ) {
		if (this->mModelData.at(i).value("cat_id").toInt() == currentCatID )
		{
			switch (index.column()) {
			case ng::cats::display_name:
				this->mModelData[i].setValue(0, value);
				this->mModelData[i].setValue(5, this->mModelData[i].value(0) );
				break;
			case ng::cats::path:
				this->mModelData[i].setValue(1,value);
				break;
			case 2:
				break;
			case 3:
				break;
			case ng::cats::can_child:
				this->mModelData[i].setValue(4, "true");
				break;
			case 5:
				//this->mModelData[i].setValue(5,this->mModelData[i].value(1) );
				break;
			case ng::cats::folder:
				this->mModelData[i].setValue(6,"no");
				break;
			case ng::cats::delete_flag:
				this->mModelData[i].setValue(7,"false");
				break;
			case ng::cats::create_time:
				this->mModelData[i].setValue(8, QDateTime::currentDateTime().toString(Qt::SystemLocaleDate));
				break;
			case ng::cats::dirty:	//modify flag
				this->mModelData[i].setValue(9,"true");
				break;
			default:
				qDebug()<< "SqliteCategoryModel::setData " << index.column() ;
				assert( 1== 2);
			}

			emit dataChanged(index,index);
		}
	}

	return true ;
}

bool SqliteCategoryModel::rowMoveTo( const QModelIndex & from , const QModelIndex & to )
{
	//QDomNode fnode , tnode ;
	//DomItem * fitem , * titem ;

	//bool isChild = false ;
	////see if to is child or sub child of from 
	//{
	//	QModelIndex idx , cidx;
	//	QModelIndexList mil ;
	//	mil.append(from);
	//	while( mil.count() > 0 )
	//	{
	//		idx = mil.takeFirst();
	//		for (int i = 0 ;i < idx.model()->rowCount(idx) ; i ++ )
	//		{
	//			cidx = idx.child(i,0);
	//			if (cidx == to ) 
	//			{
	//				isChild = true ; break ;
	//			}
	//			else
	//			{
	//				mil.append(cidx);
	//			}
	//		}
	//	}
	//}

	//qDebug()<<isChild;
	//if (isChild ) 
	//{
	//	qDebug()<<"Can't move parent to child cat";
	//	return false ;
	//}

	//fitem = static_cast<DomItem*>(from.internalPointer()); 
	//titem = static_cast<DomItem*>(to.internalPointer()); 

	//fnode = fitem->node();
	//tnode = titem->node();

	//tnode.appendChild(fnode);
	//titem->removeRow(0);
	//
	//this->removeRows(from.row(),1,from.parent());


	return true ;
}

bool SqliteCategoryModel::submit () 
{
	//把已经修改或者添加了的数据写入到数据库中。
	int dirtycnt = 0 ;
	int rowcnt = this->mModelData.count();
	for (int i = rowcnt - 1 ; i >=0 ; i -- ) {
		if (this->mModelData.at(i).value("delete_flag").toString() == "false" 
			&& this->mModelData.at(i).value("dirty").toString() == "true") {
			QSqlRecord rec = this->mModelData.at(i);

			dirtycnt ++;

			int cat_id = rec.value("cat_id").toInt();
			QString display_name = rec.value("display_name").toString();
			QString raw_name = rec.value("raw_name").toString();
			QString folder = rec.value("folder").toString();
			QString path = rec.value("path").toString();
			QString can_child = rec.value("can_child").toString();
			QString parent_cat_id = rec.value("parent_cat_id").toString();
			QString create_time = rec.value("create_time").toString();
			QString delete_flag = rec.value("delete_flag").toString();

			this->mStorage->addCategory(cat_id,display_name,raw_name,folder,path,can_child,
				parent_cat_id,create_time,delete_flag);

			this->mModelData[i].setValue("dirty","false");	//清除dirty 标记
		} else if (this->mModelData.at(i).value("delete_flag").toString() == "true" 
			&& this->mModelData.at(i).value("dirty").toString() == "true") {
			QSqlRecord rec = this->mModelData.at(i);
			dirtycnt ++;
			//int cat_id = rec.value("cat_id").toInt();
			//this->mModelData.remove(i);
			//
			//this->mStorage->deleteCategory(cat_id,true);
			//emit layoutChanged () ;
		}
	}
	qDebug()<<"there is about "<< dirtycnt <<" dirty rows to submit "<< this->mModelData.count();

	return true ;
}

void SqliteCategoryModel::revert()
{
	int dirtycnt = 0 ;
	int rowcnt = this->mModelData.count();
	for (int i = 0 ; i < rowcnt ; i ++ ) {
		if (this->mModelData.at(i).contains("dirty") && this->mModelData.at(i).value("dirty").toString() == "true") {
			dirtycnt ++;
		}
	}

	qDebug()<<"there is about "<< dirtycnt <<" dirty rows to revert ";
	return ;
}



