// seedfilesdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-11 16:22:25 +0800
// Version: $Id$
// 

#include "sqlitestorage.h"
#include "sqlitecategorymodel.h"
#include "seedfilemodel.h"

#include "seedfilesdialog.h"



SeedFileItemDelegate::SeedFileItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    
}

SeedFileItemDelegate::~SeedFileItemDelegate()
{
}
QWidget *SeedFileItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    qDebug()<<parent<<option<<index;
    if (index.column() == 1) {
        QCheckBox *cb = new QCheckBox(parent);
        return cb;
    } else {
        return NULL;
    }
}

void SeedFileItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index ) const
{
    if (index.column() == 1) {
        QCheckBox *cb = static_cast<QCheckBox*>(editor);
        cb->setChecked( index.data().toString() == "true" ? true : false);
    }
}

void SeedFileItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if (index.column() == 1) {
        QString seleted = index.data().toString();

        QStyleOptionButton checkBoxOption;
        checkBoxOption.rect = option.rect;
        checkBoxOption.state = (seleted == "true"  ? QStyle::State_On : QStyle::State_Off)
            | QStyle::State_Enabled | QStyle::State_Editing 
            ;
        checkBoxOption.text = seleted;

        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

/////////////////////////////////
////////////////////////////////

SeedFilesDialog::SeedFilesDialog(QWidget *parent)
    : QDialog(parent)
{
    this->uiwin.setupUi(this);
    
    QObject::connect(this->uiwin.pushButton_2, SIGNAL(clicked()),
                     this, SLOT(accept()));
    QObject::connect(this->uiwin.pushButton_3, SIGNAL(clicked()),
                     this, SLOT(reject()));


    this->itemDelegate = new SeedFileItemDelegate();
    this->uiwin.tableView->setItemDelegate(this->itemDelegate);

    this->seedFileModel = new SeedFileModel();
    this->uiwin.tableView->setModel(this->seedFileModel);

    QObject::connect(this->uiwin.toolButton_2, SIGNAL(clicked()),
                     this, SLOT(onAutoSelectFiles()));
    QObject::connect(this->uiwin.toolButton_3, SIGNAL(clicked()),
                     this, SLOT(onSelectAllFiles()));
    QObject::connect(this->uiwin.toolButton_4, SIGNAL(clicked()),
                     this, SLOT(onRevertSelectFiles()));
    QObject::connect(this->uiwin.toolButton_5, SIGNAL(clicked()),
                     this, SLOT(onSelectVideoFiles()));
    QObject::connect(this->uiwin.toolButton_6, SIGNAL(clicked()),
                     this, SLOT(onSelectAudioFiles()));


	//拿到全局单一实例的分类数据模型。
	//this->mCatModel = CategoryModel::instance(0);
	this->mCatModel = SqliteCategoryModel::instance(this);
	this->uiwin.comboBox->setModel(this->mCatModel);	//这是必须的一步，setView方法的限制

	//创建分类树结构
	this->mCatView = new QTreeView(0);
	//this->mCatView->setSelectionMode(QAbstractItemView::SingleSelection);
	//this->mCatView->setSelectionBehavior(QAbstractItemView::SelectItems);
	//this->mCatView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	this->mCatView->setMinimumHeight(200);
    this->mCatView->setMinimumWidth(360);

	this->mCatView->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->uiwin.comboBox->setView(this->mCatView);

    this->uiwin.comboBox->setRootModelIndex(this->mCatModel->index(0, 0));	//very important

	this->expandAll(this->mCatModel->index(0, 0));
	//this->mCatView->header()->setHidden(true);		//隐藏Tree Header
	//this->mCatView->setColumnHidden(1,true);
	
    this->mCatId = 2; //  this->mCatModel->index(0, 0, this->mCatModel->index(0, 0)).internalId();
    this->uiwin.comboBox->setEditText(tr("downloaded"));
	this->uiwin.comboBox_2->clear();	
	this->uiwin.comboBox_2->addItem(this->mCatModel->index(0,1,this->mCatModel->index(0,0) ).data().toString());
	this->uiwin.comboBox_2->addItem(this->mCatModel->index(1,1,this->mCatModel->index(0,0) ).data().toString());
	this->uiwin.comboBox_2->setEditText(this->mCatModel->index(0,1,this->mCatModel->index(0,0) ).data().toString());

	QObject::connect(uiwin.comboBox, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(onCategoryBoxChange(int)));
	QObject::connect(this->mCatView->selectionModel(),
                     SIGNAL(selectionChanged ( const QItemSelection & , const QItemSelection &)),
                     this, SLOT(onCatListSelectChange(const QItemSelection &, const QItemSelection &)));
    
}

SeedFilesDialog::~SeedFilesDialog()
{
}

void SeedFilesDialog::setFiles(QVariantList files, bool selectAll)
{
    this->seedFileModel->setData(files, selectAll);
}

void SeedFilesDialog::setTorrentInfo(QVariantMap statusInfo, QVariantMap torrentInfo)
{
    QString comment;
    QString creationDate;
    QString mode;
    QString name;
    QString totalLength;

    comment = torrentInfo.value("comment").toString();
    creationDate = torrentInfo.value("creationDate").toString();
    mode = torrentInfo.value("mode").toString();
    name = torrentInfo.value("info").toMap().value("name").toString();

    totalLength = statusInfo.value("totalLength").toString();

    this->uiwin.lineEdit->setText(name);
    this->uiwin.label_4->setText(totalLength);
    
    QVariantList trackers = torrentInfo.value("announceList").toList();
    QVariantList d2Trackers;
    QString  tracker;

    QVector<QString> vTrackers;
    for (int i = 0 ; i < trackers.count(); ++i) {
        d2Trackers = trackers.at(i).toList();
        for (int j = 0 ; j < d2Trackers.count(); j++) {
            tracker = d2Trackers.at(j).toString();
            vTrackers.append(tracker);
            this->uiwin.plainTextEdit->appendPlainText(tracker);
        }
    }
    
}

QString SeedFilesDialog::getSelectedFileIndexes()
{
    QString selectedList;

    QModelIndex beginModel = this->seedFileModel->index(0, 1);
    QModelIndexList mil = this->seedFileModel->match(beginModel, Qt::DisplayRole, QString("true"),
                                                     this->seedFileModel->rowCount(QModelIndex()),
                                                     Qt::MatchExactly | Qt::MatchWrap);
    
    QStringList indexList;
    for (int i = 0 ; i < mil.count(); i ++) {
        indexList << this->seedFileModel->data(this->seedFileModel->index(mil.at(i).row(), 0)).toString();
    }

    selectedList = indexList.join(",");

    qDebug()<<"selectedList: "<< selectedList;
    return selectedList;
}

void SeedFilesDialog::onAutoSelectFiles()
{
    this->onSelectAllFiles();
}

void SeedFilesDialog::onRevertSelectFiles()
{
    QModelIndex idx;
    int rowCount = this->seedFileModel->rowCount(QModelIndex());
    for (int row = rowCount - 1; row >= 0; row --) {
        idx = this->seedFileModel->index(row, ng::seedfile::selected);
        this->seedFileModel->setData(idx, QVariant(idx.data().toString() == "true" ? false : true));
    }
}

void SeedFilesDialog::onSelectAllFiles()
{
    QModelIndex idx;
    int rowCount = this->seedFileModel->rowCount(QModelIndex());
    for (int row = rowCount - 1; row >= 0; row --) {
        idx = this->seedFileModel->index(row, ng::seedfile::selected);
        this->seedFileModel->setData(idx, QVariant(true));
    }
}

void SeedFilesDialog::onSelectVideoFiles()
{
    QString ext;
    QStringList np;
    QModelIndex idx, idx2;
    int rowCount = this->seedFileModel->rowCount(QModelIndex());
    for (int row = rowCount - 1; row >= 0; row --) {
        idx = this->seedFileModel->index(row, ng::seedfile::selected);
        idx2 = this->seedFileModel->index(row, ng::seedfile::path);
        np =  idx2.data().toString().split(".");
        ext = np.at(np.count() - 1).toLower();
        if (ext == "rm" || ext == "rmvb" || ext == "mp4" || ext == "flv"
            || ext == "asf" || ext == "avi" || ext == "mpeg" || ext == "mpg"
            || ext == "mov" || ext == "ogg" || ext == "swf") {
            this->seedFileModel->setData(idx, QVariant(true));
        } else {
            this->seedFileModel->setData(idx, QVariant(false));
        }
    }
}

void SeedFilesDialog::onSelectAudioFiles()
{
    QString ext;
    QStringList np;
    QModelIndex idx,idx2;
    int rowCount = this->seedFileModel->rowCount(QModelIndex());
    for (int row = rowCount - 1; row >= 0; row --) {
        idx =  this->seedFileModel->index(row, ng::seedfile::selected);
        idx2 = this->seedFileModel->index(row, ng::seedfile::path);
        np =  idx2.data().toString().split(".");
        ext = np.at(np.count() - 1).toLower();
        if (ext == "rm" || ext == "wma" || ext == "mp3" || ext == "midi"
            || ext == "wav" || ext == "mpeg" || ext == "mpg") {
            this->seedFileModel->setData(idx, QVariant(true));
        } else {
            this->seedFileModel->setData(idx, QVariant(false));
        }
    }
}

void SeedFilesDialog::expandAll(QModelIndex  index )
{
	QModelIndexList mil;
	QModelIndex idx ,parent;
	const QAbstractItemModel * model = 0;
	int row , col;
	if (! index.isValid()) {
		model = this->mCatModel;
		index = model->index(0,0);
	} else {
		model = index.model();
	}
	row = model->rowCount(index);

	//qDebug()<<index.isValid()<<row<<model->data(index);	

	this->mCatView->expand(index);
	this->mCatView->resizeColumnToContents(0);

	for (int i = row-1; i >=0; i-- ) {
		idx = model->index(i, 0, index);

		this->expandAll(idx);
	}

}

void SeedFilesDialog::onCategoryBoxChange(int index)
{
	qDebug()<<__FUNCTION__ << index<< this->mCatView->currentIndex()<<this->mCatView->currentIndex().data();

    QModelIndex cidx = this->mCatView->currentIndex();    
    this->mCatId = cidx.internalId();

    QModelIndex pathIndex = this->mCatModel->index(cidx.row(), ng::cats::path, cidx.parent());
	this->uiwin.comboBox_2->setEditText(pathIndex.data().toString());
    this->uiwin.comboBox_2->setToolTip(pathIndex.data().toString());
}

// what's problem
void SeedFilesDialog::onCatListSelectChange(const QItemSelection & selection , const QItemSelection & previou  ) 
{
	QString dir;
	int size;
    int catId;
	QModelIndex idx;
	QModelIndexList mil;
    
	size = selection.size();
	for (int i = 0 ; i < size ; i ++) {
		mil = selection.indexes() ;
		// qDebug()<<mil<<mil.size() ;
		for(int j = 0 ; j < mil.size() ; j ++) {
			// qDebug()<<this->mCatModel->data(mil.at(j));
            if (j == ng::cats::cat_id) {
                catId = this->mCatModel->data(mil.at(j)).toInt();
            }
		}
		// if (this->mCatModel->data(mil.at(1)).toString().isEmpty()) {
        if (catId == ng::cats::cat_root || catId == ng::cats::downloading
            || catId == ng::cats::deleted) {
			//maybe selected system default cat, like NULLGET, downloading, deleted
            // this->mCatView->selectionModel()->select(previou, QItemSelectionModel::Clear);
            this->mCatView->selectionModel()->select(selection, QItemSelectionModel::Clear);
		}
	}
}

