// sqlitetaskmodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-07 11:21:17 +0800
// Version: $Id: sqlitetaskmodel.cpp 200 2013-10-04 07:13:26Z drswinghead $
// 

#include "simplelog.h"

#include "sqlitetaskmodel.h"

#include "sqlitetasktreemodel.h"


//static
QHash<int, SqliteTaskModel*> SqliteTaskModel::mHandle;

//static 
SqliteTaskModel *SqliteTaskModel::instance(int cat_id, QObject *parent)
{
	SqliteTaskModel * tm = 0;
	
	if (SqliteTaskModel::mHandle.contains(cat_id)) {
		tm = SqliteTaskModel::mHandle.value(cat_id);
	} else {
		//tm = new SqliteTaskModel(cat_id,parent);
		tm = new SqliteTaskModel(cat_id,0);
		SqliteTaskModel::mHandle.insert(cat_id,tm);
	}

	return tm ;
}

//static 
bool SqliteTaskModel::removeInstance(int cat_id)
{
	if (SqliteTaskModel::mHandle.contains(cat_id)) {
        SqliteTaskModel *model = SqliteTaskModel::mHandle.value(cat_id);
		SqliteTaskModel::mHandle.remove(cat_id);
        model->submit();
        delete model;
		return true;
	} else {
		return false;
	}
	return false;
}

SqliteTaskModel::SqliteTaskModel(int cat_id , QObject *parent)
	: QAbstractItemModel(parent)
{
	this->mCatID = cat_id ;
	this->mStorage = SqliteStorage::instance(parent);
//	this->mStorage->open();
	this->mModelData = this->mStorage->getTaskSet( this->mCatID );
	mTasksTableColumns = this->mStorage->getInternalTasksColumns();

    mTaskRoot = new ModelTreeNode;
    for (int i = 0; i < mModelData.size(); i++) {
        QSqlRecord rec = mModelData.at(i);
        QSqlRecord *prec = new QSqlRecord(rec);
        ModelTreeNode *pnode = new ModelTreeNode();
        pnode->_data = prec;
        pnode->_parent = mTaskRoot;
        mTaskRoot->_childs.append(pnode);
    }
}

SqliteTaskModel::~SqliteTaskModel()
{
	//this->mStorage->close();
}

int SqliteTaskModel::columnCount(const QModelIndex &/*parent*/) const
{
	//return this->mModelData.at(0).count() ;
	return this->mTasksTableColumns.count();
	return 0;
}
QVariant SqliteTaskModel::data(const QModelIndex &index, int role) const
{
	// qLogx()<<__FUNCTION__<<index<<role;
	if (!index.isValid())
		return QVariant();

	int col = index.column();
	int row = index.row();

    ModelTreeNode *node = (ModelTreeNode*)index.internalPointer();
    QSqlRecord *prec = (QSqlRecord*)node->_data;

	if (role == Qt::DecorationRole && index.column() == ng::tasks::task_id
        || role == Qt::DecorationRole && index.column() == ng::tasks::task_status)	{ // task_id and status column
        if (prec->value(ng::tasks::task_status) == "ready") {
            return QImage(qApp->applicationDirPath() + "/icons/status/flag-green.png").scaled(20, 20);
        } else if (prec->value(ng::tasks::task_status) == "active") {
            return QImage(qApp->applicationDirPath() + "/icons/status/media-playback-start.png").scaled(20, 20);
        } else if (prec->value(ng::tasks::task_status) == "waiting") {
            return QImage(qApp->applicationDirPath() + "/icons/status/user-away.png").scaled(20, 20);
        } else if (prec->value(ng::tasks::task_status) == "complete") {
            return QImage(qApp->applicationDirPath() + "/icons/status/task-complete.png").scaled(20, 20);
        } else if (prec->value(ng::tasks::task_status) == "removed") {
            return QImage(qApp->applicationDirPath() + "/icons/status/list-remove.png").scaled(20, 20);
        } else if (prec->value(ng::tasks::task_status) == "error") {
            return QImage(qApp->applicationDirPath() + "/icons/status/dialog-error.png").scaled(20, 20);
        } else if (prec->value(ng::tasks::task_status) == "pause") {
            return QImage(qApp->applicationDirPath() + "/icons/status/media-playback-pause.png").scaled(20, 20);
        } else {
            return QImage(qApp->applicationDirPath() + "/icons/status/unknown.png").scaled(20, 20);
        }
	}

    // QVariant cellData = this->mModelData.at(row).value(col);
    // QVariant cellData = ((QSqlRecord*)(mTaskRoot->_childs.at(row)->_data))->value(col);
    QVariant cellData = ((QSqlRecord*)node->_data)->value(col);
    QVariant rv;
    qint64 size;

    if (role == Qt::EditRole) {
        return cellData;
    }

	if (role != Qt::DisplayRole)
		return QVariant();

    switch (col) {
    case ng::tasks::file_size:
    case ng::tasks::abtained_length:
    case ng::tasks::left_length:
        size = cellData.toString().toLongLong();
        if (size >= 1000000000) {
            // use G
            rv = QString("%1 G").arg(QString::number(size/1000000000.0));
        } else if (size >= 1000000) {
            // use M
            rv = QString("%1 M").arg(QString::number(size/1000000.0));
        } else if (size >= 1000) {
            rv = QString("%1 K").arg(QString::number(size/1000.0));
        } else {
            rv = QString("%1 B").arg(QString::number(size));
        }
        return rv;
        break;
    case ng::tasks::current_speed:
    case ng::tasks::average_speed:
        rv = QString("%1 KB/s").arg(QString::number(cellData.toString().toInt() / 1000.0));
        return rv;
        break;
    }
	// return this->mModelData.at(row).value(col);
    return cellData;
}

Qt::ItemFlags SqliteTaskModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SqliteTaskModel::headerData(int section, Qt::Orientation orientation, int role) const
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
					return this->mStorage->getTasksColumns().at(section);
					//return this->mTasksTableColumns.at(section);
					//return this->mModelData.at(0).fieldName(section) ;
					return QVariant();
		}
	}

	return QVariant();
}

QModelIndex SqliteTaskModel::index(int row, int column, const QModelIndex &parent)   const
{
	// qLogx()<<__FUNCTION__ << row <<":"<< column  ;
    if (parent.isValid()) {
        ModelTreeNode *pnode = (ModelTreeNode*)parent.internalPointer();
        ModelTreeNode *node = (ModelTreeNode*)pnode->_childs.at(row);
        if (node == NULL) {
            return QModelIndex();
        }
        Q_ASSERT(node != NULL);
        return createIndex(row, column, (void*)node);
    } else {
        ModelTreeNode *node = (ModelTreeNode*)mTaskRoot->_childs.at(row);
        Q_ASSERT(node != NULL);
        return createIndex(row, column, (void*)node);
    }

	//assert( ! parent.isValid()  ) ;

	//int parentCatID = -1 ;

    //int taskID = this->mModelData.at(row).value("task_id").toInt(); 
    // return createIndex( row , column , (void*)0 );
    return createIndex( row , column , (quintptr)0);

	if (parent.isValid() == false) {

	} else {
		assert( 1==2);
	}
	//    parentCatID = -1 ;
	//else
	//	parentCatID = parent.internalId(); //static_cast<int>(parent.internalPointer());

	////qLogx()<< __FUNCTION__ << parentCatID ;

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
	//			//qLogx()<< __FUNCTION__ << parentCatID <<" is parent of "<<childCatID ;
	//			break ;
	//		}
	//		counter += 1 ;
	//	}
	//}
	////qLogx()<< __FUNCTION__ << row <<" "<<column <<" " << parentCatID <<" is parent of "<<childCatID ;
	////
	//if ( childCatID != -1 )
	//    return createIndex( row , column , childCatID );
	//else
	//    return QModelIndex();

	return QModelIndex();
}

QModelIndex SqliteTaskModel::parent(const QModelIndex &child) const
{
	//qLogx()<<__FUNCTION__ ;
	
	if (!child.isValid())
		return QModelIndex();

    ModelTreeNode *node = (ModelTreeNode*)child.internalPointer();
    if (node->_parent == mTaskRoot) {
        return QModelIndex();
    } else {
        return createIndex(0, 0, (void*)node->_parent);
    }

	return QModelIndex();
}

int SqliteTaskModel::rowCount(const QModelIndex &parent) const
{
	// qLogx()<<__FUNCTION__  << parent;

	int count = 0;

    if (parent.isValid()) {
        ModelTreeNode *node = (ModelTreeNode*)parent.internalPointer();
        count = node->_childs.size();
    } else {
        count = mTaskRoot->_childs.size();
    }

    return count;

	if (parent.isValid()) {
        return 0; // TODO real tree model
        if (parent.parent().isValid()) {
            count = 0;
        } else {
            QVariant vv = this->data(parent);
            QString vs = vv.toString();
            if (this->mChildModelData.contains(vs)) {
                count = this->mChildModelData[vs].size();
                count = 1;
                qLogx()<<parent<<parent.parent()<<vv<<count;
            } else {
                count = 0;
            }
        }
	} else {
		count = this->mModelData.count();
	}
	
	// qLogx()<<__FUNCTION__ <<":"<< count ;
	return count;

	return 0;
}

bool SqliteTaskModel::insertRows(int row, int count, const QModelIndex & parent  )
{
	//qLogx()<<__FUNCTION__<<row;

    if (parent.isValid()) {
        ModelTreeNode *pnode = (ModelTreeNode*)parent.internalPointer();
        qLogx()<<parent<<pnode;

        beginInsertRows(parent, row, row+count - 1);//////////
        QSqlRecord rec;
        for (int i = 0 ; i < this->mTasksTableColumns.count(); i ++) {
            QSqlField currField;
            currField.setName(this->mTasksTableColumns.at(i));
            currField.setValue(QVariant());
            rec.append(currField);
        }

        ModelTreeNode *node = new ModelTreeNode;
        node->_parent = pnode;
        node->_data = new QSqlRecord(rec);
        pnode->_childs.insert(row, node);

        endInsertRows(); //////////
    } else {
        beginInsertRows(parent, row, row+count - 1);//////////

        for (int c = 0 ; c < count ; c ++) {
            QSqlRecord rec;
            for (int i = 0 ; i < this->mTasksTableColumns.count() ; i ++) {
                QSqlField currField;
                currField.setName( this->mTasksTableColumns.at(i) );
                currField.setValue(QVariant());
                rec.append(currField);
            }
            ModelTreeNode *node = new ModelTreeNode;
            node->_parent = mTaskRoot;
            node->_data = new QSqlRecord(rec);
            mTaskRoot->_childs.insert(c, node);
        }
        endInsertRows(); //////////
    }

    return true;

	// assert(!parent.isValid()); 针对任务列表模式，二维表格
    if (parent.isValid()) {
        QString v = this->data(parent).toString();
        beginInsertRows(parent, row, row+count - 1);//////////
        for (int c = 0 ; c < count ; c ++) {
            QSqlRecord rec;
            for (int i = 0 ; i < this->mTasksTableColumns.count() ; i ++) {
                QSqlField currField;
                currField.setName( this->mTasksTableColumns.at(i) );
                currField.setValue(QVariant());
                rec.append(currField);
            }
            if (this->mChildModelData.contains(v)) {
                this->mChildModelData[v].insert(row + c, rec);
            } else {
                QVector<QSqlRecord> vrec;
                vrec << rec;
                this->mChildModelData[v] = vrec;
            }
        }
        endInsertRows(); //////////
    } else {
        beginInsertRows(parent, row, row+count - 1);//////////

        for (int c = 0 ; c < count ; c ++) {
            QSqlRecord rec;
            for (int i = 0 ; i < this->mTasksTableColumns.count() ; i ++) {
                QSqlField currField;
                currField.setName( this->mTasksTableColumns.at(i) );
                currField.setValue(QVariant());
                rec.append(currField);
            }
            // this->mModelData.append(rec);
            this->mModelData.insert(row + c, rec);
        }
        endInsertRows(); //////////
    }
	return true ;
}

bool SqliteTaskModel::removeRows(int row, int count, const QModelIndex &parent, bool persistRemove)
{
	//qLogx()<<__FUNCTION__<<row;
	
	int atrow = 0 ;
	int delete_begin = row ;
	int delete_end = row + count - 1;

    ModelTreeNode *pnode = NULL;
    if (parent.isValid()) {
        pnode = (ModelTreeNode*)parent.internalPointer();
    } else {
        pnode = mTaskRoot;
    }
    
	beginRemoveRows  ( parent, delete_begin, delete_end ) ;	

	for (int i = delete_end; i >= delete_begin; i --) {
        // qLogx()<<pnode<<pnode->_childs.size()<<i;
        if (i <= pnode->_childs.size() - 1) {
            pnode->_childs.remove(i);
            emit layoutChanged () ;	//这是必须，否则视图不能正常画出模型。
        } else {
            // qLogx()<<"index exceed";
        }
	}

	endRemoveRows ();

	return true;

	beginRemoveRows  ( parent, delete_begin, delete_end );	

	for (int i = delete_end; i >= delete_begin ; i --) {
		int taskID = this->mModelData.at(i).value("task_id").toInt();
		this->mModelData.remove(row);
        if (persistRemove) {
            this->mStorage->deleteTask(taskID);
        }
        emit layoutChanged () ;	//这是必须，否则视图不能正常画出模型。
	}

	endRemoveRows ();

	return true ;
}

bool SqliteTaskModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	// qLogx()<<__FUNCTION__<<index.row()<<index.column()<<index.data()<<value ;

	int row = index.row();
	int col = index.column();

    ModelTreeNode *node = (ModelTreeNode*)index.internalPointer();
    Q_ASSERT(node != NULL);
    QSqlRecord *prec = (QSqlRecord*)node->_data;

	if (col == ng::tasks::dirty) {
		
	} else {
        prec->setValue(col, value);
		// this->mModelData[row].setValue(col, value);
		//qLogx()<<row<<":"<<col <<" " << value ;
	}
	// this->mModelData[row].setValue(this->mTasksTableColumns.count()-1,"true");
    // this->mModelData[row].setValue(ng::tasks::dirty, "true");
    prec->setValue(ng::tasks::dirty, "true");
	emit dataChanged(index,index);

	return true;
}


bool SqliteTaskModel::submit () 
{
    // new
    return this->submitTree(mTaskRoot);

	// sync dirty data to disk
	int dirtycnt = 0 ;
	int rowcnt = this->mModelData.count();

    this->mStorage->transaction();
	for (int i = rowcnt - 1 ; i >= 0 ; i --) {
		if (this->mModelData.at(i).value("dirty").toString() == "true") {
			dirtycnt ++;
			QSqlRecord rec = this->mModelData.at(i);

			int taskID = this->mModelData.at(i).value("task_id").toInt();
			
			QString  file_size = rec.value("file_size").toString();
			QString  retry_times = rec.value("retry_times").toString();
			QString  create_time = rec.value("create_time").toString();
			QString  current_speed = rec.value("current_speed").toString();
			QString  average_speed = rec.value("average_speed").toString();
			QString  eclapsed_time = rec.value("eclapsed_time").toString();
			QString  abtained_length = rec.value("abtained_length").toString();
			QString  left_length = rec.value("left_length").toString();
            QString  split_count = rec.value("split_count").toString();
			QString  block_activity = rec.value("block_activity").toString();
			QString  total_block_count = rec.value("total_block_count").toString();
			QString  active_block_count = rec.value("active_block_count").toString();
			QString  user_cat_id = rec.value("user_cat_id").toString();
			QString  comment = rec.value("comment").toString();
			QString  sys_cat_id = rec.value("sys_cat_id").toString();
            QString  save_path = rec.value("save_path").toString();
			QString  file_name = rec.value("file_name").toString();
            QString  select_file = rec.value("select_file").toString();
			QString  abtained_percent = rec.value("abtained_percent").toString();
			QString  org_url = rec.value("org_url").toString();
			QString  real_url = rec.value("real_url").toString();
            QString  referer = rec.value("referer").toString();
			QString  redirect_times = rec.value("redirect_times").toString();
			QString  finish_time = rec.value("finish_time").toString();
			QString  task_status = rec.value("task_status").toString();
			QString  total_packet = rec.value("total_packet").toString();
			QString  abtained_packet = rec.value("abtained_packet").toString();
			QString  left_packet = rec.value("left_packet").toString();
			QString  total_timestamp = rec.value("total_timestamp").toString();
			QString  abtained_time_stamp = rec.value("abtained_time_stamp").toString();
			QString  left_timestamp = rec.value("left_timestamp").toString();
			QString  file_length_abtained = rec.value("file_length_abtained").toString();
			QString  dirty = rec.value("dirty").toString();
            QString  aria_gid = rec.value("aria_gid").toString();

			if (this->mStorage->containsTask(taskID)) {
				//update 
				this->mStorage->updateTask(taskID, file_size, retry_times, create_time, current_speed, average_speed,
                                           eclapsed_time, abtained_length, left_length, split_count, block_activity,
                                           total_block_count, active_block_count, user_cat_id, comment, sys_cat_id,
                                           save_path, file_name, select_file, abtained_percent, org_url, real_url, referer, 
                                           redirect_times, finish_time,
                                           task_status, total_packet, abtained_packet, left_packet, total_timestamp,
                                           abtained_time_stamp, left_timestamp, file_length_abtained, dirty, aria_gid);
			} else {
				//insert
				this->mStorage->addTask(taskID, file_size, retry_times, create_time, current_speed,
                                        average_speed, eclapsed_time, abtained_length, left_length, 
                                        split_count, block_activity, total_block_count, active_block_count, user_cat_id,
                                        comment, sys_cat_id, save_path, file_name, select_file, abtained_percent, org_url, 
                                        real_url, referer,
                                        redirect_times, finish_time, task_status, total_packet, abtained_packet,
                                        left_packet, total_timestamp, abtained_time_stamp, left_timestamp,
                                        file_length_abtained, dirty);
			}

			this->mModelData[i].setValue("dirty", "false");	 //清除dirty 标记
		}
	}
    this->mStorage->commit();

	qLogx()<<"there is about "<<dirtycnt<<"dirty rows to submit"<<this->mModelData.count();

	return true;
}

bool SqliteTaskModel::submitTree (ModelTreeNode *pnode) 
{
	// sync dirty data to disk
	int dirtycnt = 0;
	int rowcnt = pnode->_childs.size();
    ModelTreeNode *node = NULL;
    QSqlRecord *prec = NULL;

    this->mStorage->transaction();
	for (int i = rowcnt - 1 ; i >= 0 ; i --) {
        node = (ModelTreeNode*)pnode->_childs.at(i);
        prec = (QSqlRecord*)node->_data;
		if (prec->value("dirty").toString() == "true") {
			dirtycnt ++;
			QSqlRecord rec(*prec);

			int taskID = rec.value("task_id").toInt();
			
			QString  file_size = rec.value("file_size").toString();
			QString  retry_times = rec.value("retry_times").toString();
			QString  create_time = rec.value("create_time").toString();
			QString  current_speed = rec.value("current_speed").toString();
			QString  average_speed = rec.value("average_speed").toString();
			QString  eclapsed_time = rec.value("eclapsed_time").toString();
			QString  abtained_length = rec.value("abtained_length").toString();
			QString  left_length = rec.value("left_length").toString();
            QString  split_count = rec.value("split_count").toString();
			QString  block_activity = rec.value("block_activity").toString();
			QString  total_block_count = rec.value("total_block_count").toString();
			QString  active_block_count = rec.value("active_block_count").toString();
			QString  user_cat_id = rec.value("user_cat_id").toString();
			QString  comment = rec.value("comment").toString();
			QString  sys_cat_id = rec.value("sys_cat_id").toString();
            QString  save_path = rec.value("save_path").toString();
			QString  file_name = rec.value("file_name").toString();
            QString  select_file = rec.value("select_file").toString();
			QString  abtained_percent = rec.value("abtained_percent").toString();
			QString  org_url = rec.value("org_url").toString();
			QString  real_url = rec.value("real_url").toString();
            QString  referer = rec.value("referer").toString();
			QString  redirect_times = rec.value("redirect_times").toString();
			QString  finish_time = rec.value("finish_time").toString();
			QString  task_status = rec.value("task_status").toString();
			QString  total_packet = rec.value("total_packet").toString();
			QString  abtained_packet = rec.value("abtained_packet").toString();
			QString  left_packet = rec.value("left_packet").toString();
			QString  total_timestamp = rec.value("total_timestamp").toString();
			QString  abtained_time_stamp = rec.value("abtained_time_stamp").toString();
			QString  left_timestamp = rec.value("left_timestamp").toString();
			QString  file_length_abtained = rec.value("file_length_abtained").toString();
			QString  dirty = rec.value("dirty").toString();
            QString  aria_gid = rec.value("aria_gid").toString();

			if (this->mStorage->containsTask(taskID)) {
				//update 
				this->mStorage->updateTask(taskID, file_size, retry_times, create_time, current_speed, average_speed,
                                           eclapsed_time, abtained_length, left_length, split_count, block_activity,
                                           total_block_count, active_block_count, user_cat_id, comment, sys_cat_id,
                                           save_path, file_name, select_file, abtained_percent, org_url, real_url, referer, 
                                           redirect_times, finish_time,
                                           task_status, total_packet, abtained_packet, left_packet, total_timestamp,
                                           abtained_time_stamp, left_timestamp, file_length_abtained, dirty, aria_gid);
			} else {
				//insert
				this->mStorage->addTask(taskID, file_size, retry_times, create_time, current_speed,
                                        average_speed, eclapsed_time, abtained_length, left_length, 
                                        split_count, block_activity, total_block_count, active_block_count, user_cat_id,
                                        comment, sys_cat_id, save_path, file_name, select_file, abtained_percent, org_url, 
                                        real_url, referer,
                                        redirect_times, finish_time, task_status, total_packet, abtained_packet,
                                        left_packet, total_timestamp, abtained_time_stamp, left_timestamp,
                                        file_length_abtained, dirty);
			}

            prec->setValue("dirty", "false");	 //清除dirty 标记
		}
	}
    this->mStorage->commit();

	qLogx()<<"there is about "<<dirtycnt<<"dirty rows to submit"<<this->mModelData.count();

	return true;
}


void SqliteTaskModel::revert()
{
    return revertTree(mTaskRoot);

    /// 
	int dirtycnt = 0 ;
	int rowcnt = this->mModelData.count();
	for (int i = 0 ; i < rowcnt ; i ++) {
		if (this->mModelData.at(i).contains("dirty") && this->mModelData.at(i).value("dirty").toString() == "true") {
			dirtycnt ++;
		}
	}

	qLogx()<<"there is about "<< dirtycnt <<" dirty rows to revert ";
	return;
}

void SqliteTaskModel::revertTree(ModelTreeNode *pnode)
{
	int dirtycnt = 0 ;
	int rowcnt = pnode->_childs.size();
    ModelTreeNode *node = NULL;
    QSqlRecord *prec = NULL;

	for (int i = 0 ; i < rowcnt ; i ++) {
        node = (ModelTreeNode*)pnode->_childs.at(i);
        prec = (QSqlRecord*)node->_data;
        
		if (prec->contains("dirty") && prec->value("dirty").toString() == "true") {
			dirtycnt ++;
		}
	}

	qLogx()<<"there is about "<< dirtycnt <<" dirty rows to revert ";
	return;
}

bool SqliteTaskModel::moveTasks(int srcCatId, int destCatId, QModelIndexList &mil, bool persistRemove)
{
    SqliteTaskModel *srcModel = NULL;
    SqliteTaskModel *destModel = NULL;

    srcModel = SqliteTaskModel::instance(srcCatId);
    destModel = SqliteTaskModel::instance(destCatId);

    int columnCount = srcModel->columnCount();
    int moveRowCount = mil.count() / columnCount;

    QVector<QMap<int, QVariant> > rowItemData;
    for (int row = moveRowCount - 1; row >= 0; row --) {
        for (int col = 0; col < columnCount; col ++) {
            rowItemData.append(srcModel->itemData(mil.at(row * columnCount + col)));
        }

        if (srcModel->rowCount(mil.at(row * columnCount)) > 0) {
            qLogx()<<"Maybe need remove childs first";
        }

        // 
        srcModel->removeRows(mil.at(row * columnCount).row(), 1, QModelIndex(), false);
        destModel->insertRows(0, 1);
        Q_ASSERT(destModel->rowCount() > 0);

        for (int col = 0 ; col < columnCount; col ++) {
            QMap<int, QVariant> itemData = rowItemData.at(col);
            itemData[Qt::DisplayRole] = itemData[Qt::EditRole];

            if (col == ng::tasks::sys_cat_id) {
                itemData[Qt::DisplayRole] = QString("%1").arg(destCatId);
            }
            if (col == ng::tasks::dirty) {
                itemData[Qt::DisplayRole] = QString("true");
            }
            
            QModelIndex idx = destModel->index(0, col);
            Q_ASSERT(idx.isValid());
            destModel->setData(destModel->index(0, col), itemData[Qt::DisplayRole], Qt::DisplayRole);
        }
    }
    destModel->submit();

    return true;
}



