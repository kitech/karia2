// taskinfodlg.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-08 00:16:42 +0800
// Version: $Id$
// 
#include <QtCore>
#include <QtGui>
#include <QUrl>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QComboBox>
#include <QMessageBox>

#include <QListView>
#include <QListWidget>
#include <QSize>

#include "taskinfodlg.h"
#include "viewmodel.h"
#include "catmandlg.h"
#include "sqlitestorage.h"
#include "sqlitecategorymodel.h"
#include "dircompletermodel.h"

CategoryComboBoxItemDelegate::CategoryComboBoxItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
	mCatView = 0;	
	this->mCatView = new QTreeView(0);
}

CategoryComboBoxItemDelegate::~CategoryComboBoxItemDelegate()
{

}
QWidget *CategoryComboBoxItemDelegate::createEditor(QWidget *parent,
                                                    const QStyleOptionViewItem &option,
                                                    const QModelIndex &index ) const
{
	qDebug()<<__FUNCTION__;
	return this->mCatView;
}

void CategoryComboBoxItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option,
                                         const QModelIndex & index ) const
{
	qDebug()<<__FUNCTION__;
	//qDebug()<<option<<index.row()<<index.column();
}


QSize CategoryComboBoxItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const 
{
	qDebug()<<__FUNCTION__;
	QSize s;
	s.setHeight(100);
	//s.setWidth(30);
	return s;
}

///////////////////////////
TaskOption::TaskOption(QObject *parent)
{
}

TaskOption::~TaskOption()
{
}

void TaskOption::setDefaultValue()
{
	//general
	//QString mTaskUrl;
	mFindUrlByMirror = 1;
	//QString mReferrer;
	//QString mCategory;
    this->mCatId = 2; // downloaded
	mSavePath = ".";
	//QString mSaveName;
	mSplitCount = 5; 
	mNeedLogin = 0;
	mLoginUserName = "";
	mLoginPassword = "";
	mComment = "";
	mStartState = 1;	//0,1,2

	////////
	mAlterUrls.clear();
	
	//
	mAutoDownSubdirFromFtp = 1;
	mAutoCreateSubdirLocally = 1; 
	mAutoCreateCategory  = 1;
	
	mProxyTypeHttp = 0;
	mProxyTypeFtp = 0;
	mProxyTypeMedia = 0;	
}

void TaskOption::dump()
{
	//general
	qDebug()<<"mTaskUrl:"<<mTaskUrl;
	qDebug()<<"mReferrer:"<<mReferer;
    qDebug()<<"mCookies:"<<mCookies;
    qDebug()<<"mAgent:"<<mAgent;
	qDebug()<<"mCategory:"<<mCategory;
    qDebug()<<"mCatId:"<<mCatId;
	qDebug()<<"mSavePath:"<<mSavePath ;
	qDebug()<<"mSaveName:"<<mSaveName;
	qDebug()<<"mSplitCount:"<<mSplitCount ; 
	qDebug()<<"mNeedLogin:"<<mNeedLogin ;
	qDebug()<<"mLoginUserName:"<<mLoginUserName ;
	qDebug()<<"mLoginPassword:"<<mLoginPassword ;
	qDebug()<<"mComment:"<<mComment;
	qDebug()<<"mStartState:"<<mStartState;	//0,1,2

	////////
	qDebug()<<"mFindUrlByMirror:"<<mFindUrlByMirror ;
	//mAlterUrls.clear();
	
	//
	qDebug()<<"mAutoDownSubdirFromFtp:"<<mAutoDownSubdirFromFtp;
	qDebug()<<"mAutoCreateSubdirLocally:"<<mAutoCreateSubdirLocally; 
	qDebug()<<"mAutoCreateCategory:"<<mAutoCreateCategory;
	
	qDebug()<<"mProxyTypeHttp:"<<mProxyTypeHttp;
	qDebug()<<"mProxyTypeFtp:"<<mProxyTypeFtp;
	qDebug()<<"mProxyTypeMedia:"<<mProxyTypeMedia;		
}

/*
  package format:
  0, TaskOption
  1, url
  2, refer
  3, cookies
 */
QByteArray TaskOption::toRawData()
{
    QByteArray ba;

    ba = "TaskOption";
    ba += '|' + this->mTaskUrl.toAscii().toHex();
    ba += '|' + this->mReferer.toAscii().toHex();
    ba += '|' + this->mCookies.toAscii().toHex();
    ba += '|' + this->mAgent.toAscii().toHex();

    return ba;
}
QString TaskOption::toBase64Data()
{
    QByteArray ba = this->toRawData();

    return ba.toBase64();
}

// static 
TaskOption TaskOption::fromRawData(QByteArray ba)
{
    TaskOption option;

    QList<QByteArray> elts = ba.split('|');
    if (elts.count() != 5 || elts.at(0) != "TaskOption") {
        qDebug()<<"Invalid TaskOption package";
        return TaskOption();
    }

    option.mTaskUrl = QString(QByteArray::fromHex(elts.at(1)));
    option.mReferer = QString(QByteArray::fromHex(elts.at(2)));
    option.mCookies = QString(QByteArray::fromHex(elts.at(3)));
    option.mAgent = QString(QByteArray::fromHex(elts.at(4)));

    return option;
}

// static 
TaskOption TaskOption::fromBase64Data(QString data)
{
    QByteArray ba = QByteArray::fromBase64(data.toAscii());
    return TaskOption::fromRawData(ba);
}


// static
TaskOption *TaskOption::fromModelRow(QAbstractItemModel *model, int row)
{
    Q_ASSERT(model != NULL);
    TaskOption *option = NULL;

    option = new TaskOption();
    option->mTaskUrl = model->data(model->index(row, ng::tasks::org_url)).toString();
    option->mReferer = model->data(model->index(row, ng::tasks::org_url)).toString();
    option->mCookies = model->data(model->index(row, ng::tasks::comment)).toString();
    // option->mCategory = 
    option->mCatId = model->data(model->index(row, ng::tasks::user_cat_id)).toInt();
    option->mSaveName = model->data(model->index(row, ng::tasks::file_name)).toString();
    option->mSavePath = model->data(model->index(row, ng::tasks::save_path)).toString();
    option->mSplitCount = model->data(model->index(row, ng::tasks::split_count)).toInt();
    option->mComment = model->data(model->index(row, ng::tasks::comment)).toString();

    return option;
}

////////////////////
// 这些变量名起的太飘然了
taskinfodlg::taskinfodlg(QWidget *parent)
    : QDialog(parent)
{
	uiwin.setupUi(this);

	//signals
	QObject::connect(uiwin.tid_g_le_url, SIGNAL(textChanged(QString )), this, SLOT(onUrlBoxChange(QString )));
	//QObject::connect(uiwin.tid_g_le_cb_category,SIGNAL(currentIndexChanged(int)),this,SLOT(onCategoryBoxChange(int)));
	QObject::connect(uiwin.tid_g_le_cb_category, SIGNAL(editTextChanged(const QString &)),
                     this, SLOT(onCategoryBoxChange(const QString &)));
	QObject::connect(uiwin.tid_au_pb_add, SIGNAL(clicked()), this, SLOT(onAddAlterUrl()));
	QObject::connect(uiwin.tid_au_pb_delete, SIGNAL(clicked()), this, SLOT(onDeleteAlterUrl()));
	QObject::connect(uiwin.tid_g_le_pb_show_dir, SIGNAL(clicked()), this, SLOT(onChangeSaveDirectory()));
	// QObject::connect(uiwin.tid_g_le_pb_category_info, SIGNAL(clicked()), this, SLOT(onShowCategoryInfo()));

	//如果剪贴板上是有效的URL，则认为这就是要添加的任务，直填写到任务URL栏中。
	QClipboard *cb = QApplication::clipboard();
	QString cbstr = cb->text();	

    if (cbstr.startsWith("nullget://")) {
        qDebug()<<__FUNCTION__<<cbstr;   
        TaskOption options = TaskOption::fromBase64Data(cbstr.right(cbstr.length() - 10));
        this->uiwin.tid_g_le_url->setText(options.mTaskUrl);
        this->uiwin.tid_g_le_referrer->setText(options.mReferer);
        this->uiwin.tid_g_le_te_comment->setText(options.mCookies);
    } else {
        QUrl u(cbstr);
        qDebug()<<__FUNCTION__<<cbstr<<u<<u.isValid()<<u.scheme();   
        if (u.isValid() && u.scheme().length() > 0) {
            this->uiwin.tid_g_le_url->setText(cbstr);
        } if (cbstr.toLower().startsWith("magnet:?")) {
            this->uiwin.tid_g_le_url->setText(cbstr);
        } else {
            this->uiwin.tid_g_le_url->setText(this->uiwin.tid_g_le_url->text());
        }
    }

	this->uiwin.tid_g_le_url->selectAll();

	//拿到全局单一实例的分类数据模型。
	//this->mCatModel = CategoryModel::instance(0);
	this->mCatModel = SqliteCategoryModel::instance(this);
	this->uiwin.tid_g_le_cb_category->setModel(this->mCatModel);	//这是必须的一步，setView方法的限制

	//创建分类树结构
	this->mCatView = new QTreeView(0);
	//this->mCatView->setSelectionMode(QAbstractItemView::SingleSelection);
	//this->mCatView->setSelectionBehavior(QAbstractItemView::SelectItems);
	//this->mCatView->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	this->mCatView->setMinimumHeight(200);
    this->mCatView->setMinimumWidth(360);

	this->mCatView->setSelectionBehavior(QAbstractItemView::SelectRows);
	this->uiwin.tid_g_le_cb_category->setView(this->mCatView);

    this->uiwin.tid_g_le_cb_category->setRootModelIndex(this->mCatModel->index(0, 0));	//very important

	this->expandAll(this->mCatModel->index(0, 0));
	//this->mCatView->header()->setHidden(true);		//隐藏Tree Header
	//this->mCatView->setColumnHidden(1,true);
	
    this->mCatId = 2; //  this->mCatModel->index(0, 0, this->mCatModel->index(0, 0)).internalId();
	this->mSwapValue = this->mCatModel->index(0, 0, this->mCatModel->index(0,0)).data().toString();
	this->uiwin.tid_g_le_cb_category->setEditText(this->mSwapValue);
    this->uiwin.tid_g_le_cb_category->setEditText(tr("downloaded"));
	this->uiwin.tid_g_cb_save_to->clear();	
	this->uiwin.tid_g_cb_save_to->addItem(this->mCatModel->index(0,1,this->mCatModel->index(0,0) ).data().toString());
	this->uiwin.tid_g_cb_save_to->addItem(this->mCatModel->index(1,1,this->mCatModel->index(0,0) ).data().toString());
	this->uiwin.tid_g_cb_save_to->setEditText(this->mCatModel->index(0,1,this->mCatModel->index(0,0) ).data().toString());

	QObject::connect(this->mCatView, SIGNAL(pressed(const QModelIndex & )), 
                     this, SLOT(onCatListClicked(const QModelIndex &)));

	QObject::connect(this->mCatView->selectionModel(),
                     SIGNAL(selectionChanged ( const QItemSelection & , const QItemSelection &)),
                     this, SLOT(onCatListSelectChange(const QItemSelection &, const QItemSelection &)));

	//QObject::connect(this->mCatView->selectionModel(),SIGNAL(selectionChanged ( const QItemSelection & , const QItemSelection &   )),
	//	this,SLOT(onCatListSelectChange( const QItemSelection & , const QItemSelection &   ) ) );

#ifdef WIN32
	this->uiwin.tid_g_cb_save_to->setEditText("C:/NGDownload");
#else
	this->uiwin.tid_g_cb_save_to->setEditText( "~/NGDownload" );
#endif

	QCompleter * completer = new QCompleter(this);
	DirCompleterModel *dirModel = new DirCompleterModel(completer);
	completer->setModel(dirModel);
	this->uiwin.tid_g_cb_save_to->setCompleter(completer);

    this->show();
    // this->onShowMoreInfo();
    QObject::connect(this->uiwin.toolButton, SIGNAL(clicked()),
                     this, SLOT(onShowMoreInfo()));
}

taskinfodlg::~taskinfodlg()
{

}

taskinfodlg::taskinfodlg(TaskOption *param , QWidget *parent )	
	: QDialog(parent)
{

	uiwin.setupUi(this);

	//signals
	QObject::connect(uiwin.tid_g_le_url, SIGNAL(textChanged(QString )), this, SLOT(onUrlBoxChange(QString )));
	QObject::connect(uiwin.tid_g_le_cb_category, SIGNAL(currentIndexChanged(int)),
                     this, SLOT(onCategoryBoxChange(int)));
	QObject::connect(uiwin.tid_au_pb_add, SIGNAL(clicked()), this, SLOT(onAddAlterUrl()));
	QObject::connect(uiwin.tid_au_pb_delete, SIGNAL(clicked()), this, SLOT(onDeleteAlterUrl()));

	//
	QClipboard *cb = QApplication::clipboard();
	QString cbstr = cb->text();	
    qDebug()<<__FUNCTION__<<cbstr;

	QUrl u(cbstr);
	if (u.isValid()) {
		this->uiwin.tid_g_le_url->setText(cbstr);
    }
	this->uiwin.tid_g_le_url->selectAll();

	//
	TaskOption * prm = param;
	if (prm != 0) {
		//general
		uiwin.tid_g_le_url->setText(prm->mTaskUrl);
		uiwin.tid_g_cb_seache_mirror->setChecked(param->mFindUrlByMirror==1?true:false);
		uiwin.tid_g_le_referrer->setText(prm->mReferer);
		uiwin.tid_g_le_cb_category->setEditText(prm->mCategory);

		uiwin.tid_g_cb_save_to->setEditText(prm->mSavePath);

		uiwin.tid_g_le_le_rename->setText(prm->mSaveName);
		
		uiwin.tid_g_sb_split_file->setValue(prm->mSplitCount);

		uiwin.tid_g_le_cb_login_to_server->setChecked(prm->mNeedLogin==1?true:false);
		
		uiwin.tid_g_le_le_user_name->setText(prm->mLoginUserName);
		uiwin.tid_g_le_le_password->setText(prm->mLoginPassword);
		uiwin.tid_g_le_te_comment->setPlainText(prm->mComment);
		if (prm->mStartState == 0 ) {
			uiwin.tid_g_le_rb_manual->setChecked(true);
		// } else if ( prm->mStartState == 2 ) {
		// 	uiwin.tid_g_le_rb_schedule->setChecked(true);
		} else {
			uiwin.tid_g_le_rb_immediately->setChecked(true);
		}
		
		////////
		
		int row = prm->mAlterUrls.count();
		for (int i = 0; i < row; ++i) {
			uiwin.tid_au_lw_alter_urls->addItem(prm->mAlterUrls.at(i));
		}
		
		//
		// uiwin.tid_ad_cb_down_subdir_from_ftp->setChecked(prm->mAutoDownSubdirFromFtp==1?true:false);
		// uiwin.tid_ad_cb_create_subdir_locally->setChecked(prm->mAutoCreateSubdirLocally==1?true:false);
		// uiwin.tid_ad_cb_create_category->setChecked(prm->mAutoCreateCategory==1?true:false);
		
		// uiwin.tid_ad_cb_proxy_type_http->setCurrentIndex(prm->mProxyTypeHttp);
		// uiwin.tid_ad_cb_proxy_type_ftp->setCurrentIndex(prm->mProxyTypeFtp);
		// uiwin.tid_ad_cb_proxy_type_media->setCurrentIndex(prm->mProxyTypeMedia);		
	}
}

void taskinfodlg::onUrlBoxChange(QString text)
{
	//this->uiwin.tid_g_le_le_rename->setText(uiwin.tid_g_le_url->text());	
	QUrl url(text);
	QFileInfo fi(url.path());
	QString fname = url.hasQuery() ?
        fi.fileName().left(fi.fileName().lastIndexOf('?')) 
        : fi.fileName();
	if (fname.isEmpty()) fname = "index.html";
	this->uiwin.tid_g_le_le_rename->setText(fname);
	uiwin.tid_au_lb_url->setText(text);

    //// simple hint
    if (text.isEmpty()) {
        this->uiwin.label_4->setText(tr("Please input URL."));
    } else if (!url.isValid()) {
        this->uiwin.label_4->setText(tr("Not a valide URL."));
    } 
}

void taskinfodlg::onCategoryBoxChange(int index)
{
	qDebug()<<__FUNCTION__ ;
	QString cat;

	cat = uiwin.tid_g_le_cb_category->itemText(index);

	cat = QString("C:\\") + cat;
	uiwin.tid_g_cb_save_to->setItemText(index, cat);
	uiwin.tid_g_cb_save_to->setCurrentIndex(index);

}

void taskinfodlg::onCategoryBoxChange(const QString & text)
{
	qDebug()<<__FUNCTION__ << text;
	if (text.isEmpty() || text.isNull() || text.length() == 0) {
		this->uiwin.tid_g_le_cb_category->setEditText(this->mSwapValue);
    }
	this->uiwin.tid_g_le_cb_category->lineEdit()->setWindowIcon(QIcon("icons/crystalsvg/16x16/filesystems/folder_violet_open.png"));
}

// what's problem
void taskinfodlg::onCatListSelectChange(const QItemSelection & selection , const QItemSelection & previou  ) 
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

void taskinfodlg::onShowMoreInfo()
{
    QWidget *w = this->uiwin.widget;
    if (this->uiwin.toolButton->arrowType() == Qt::UpArrow) {
        int heightDelta = w->height();
        QSize baseSize = this->size();
        w->setVisible(false);
        baseSize.setHeight(baseSize.height() - heightDelta);
        this->resize(baseSize);
        this->setFixedHeight(baseSize.height());
        this->uiwin.toolButton->setArrowType(Qt::DownArrow);
        this->uiwin.toolButton->setText(tr("Expand"));
    } else {
        int heightDelta = w->height();
        QSize baseSize = this->size();
        baseSize.setHeight(baseSize.height() + heightDelta);
        this->resize(baseSize);
        this->setFixedHeight(baseSize.height());
        w->setVisible(true);
        this->uiwin.toolButton->setArrowType(Qt::UpArrow);
        this->uiwin.toolButton->setText(tr("Collapse"));
    }
}

TaskOption * taskinfodlg::getOption()
{
	TaskOption * param = 0;

	param = new TaskOption();
	param->setDefaultValue();

	//
	//general
	param->mTaskUrl = uiwin.tid_g_le_url->text();
	param->mFindUrlByMirror = uiwin.tid_g_cb_seache_mirror->isChecked()?1:0;
	param->mReferer = uiwin.tid_g_le_referrer->text();
    param->mCatId = this->mCatId;
	param->mCategory = uiwin.tid_g_le_cb_category->currentText();

#ifdef WIN32
	param->mSavePath = QDir(uiwin.tid_g_cb_save_to->currentText()).absolutePath ();
#else
    param->mSavePath = uiwin.tid_g_cb_save_to->currentText();

    if (param->mSavePath.startsWith("~")) {
        param->mSavePath = QDir::homePath() + uiwin.tid_g_cb_save_to->currentText().right(param->mSavePath.length()-1);
    }
	qDebug()<<__FUNCTION__<<param->mSavePath<<QDir::isAbsolutePath(param->mSavePath);
#endif
	
	param->mSaveName = uiwin.tid_g_le_le_rename->text();
	if (param->mSaveName.isEmpty() ) {		
		param->mSaveName = QFileInfo(QUrl(param->mTaskUrl).path()).fileName();
	}

	param->mSplitCount = uiwin.tid_g_sb_split_file->value(); 
	param->mNeedLogin = uiwin.tid_g_le_cb_login_to_server->isChecked()?1:0;
	param->mLoginUserName = uiwin.tid_g_le_le_user_name->text();
	param->mLoginPassword = uiwin.tid_g_le_le_password->text() ;
    param->mCookies = uiwin.tid_g_le_te_comment->toPlainText();
	param->mComment = uiwin.tid_g_le_te_comment->toPlainText();
	if (uiwin.tid_g_le_rb_manual->isChecked())
		param->mStartState = 0	;	//0,1,2
	// else if ( uiwin.tid_g_le_rb_schedule->isChecked() )
	// 	param->mStartState = 2;
	else 
		param->mStartState = 1;

	////////
	param->mAlterUrls.clear();
	int row = uiwin.tid_au_lw_alter_urls->count();
	for (int i = 0; i < row; ++i) {
		param->mAlterUrls.append(uiwin.tid_au_lw_alter_urls->item(i)->text());
	}
	
	//
	// param->mAutoDownSubdirFromFtp = uiwin.tid_ad_cb_down_subdir_from_ftp->isChecked()?1:0;
	// param->mAutoCreateSubdirLocally = uiwin.tid_ad_cb_create_subdir_locally->isChecked()?1:0; 
	// param->mAutoCreateCategory  = uiwin.tid_ad_cb_create_category->isChecked() ? 1:0;
	
	// param->mProxyTypeHttp = uiwin.tid_ad_cb_proxy_type_http->currentIndex();
	// param->mProxyTypeFtp = uiwin.tid_ad_cb_proxy_type_ftp->currentIndex();
	// param->mProxyTypeMedia = uiwin.tid_ad_cb_proxy_type_media->currentIndex( );	

    // param->dump();

	return param;

}


void taskinfodlg::onAddAlterUrl()
{
	QString surl = QInputDialog::getText(this, tr("Input mirror URL:"), tr("URL:"));
	if (! surl.isEmpty()) {
		
		int row = uiwin.tid_au_lw_alter_urls->count();
		for (int i = 0; i < row; ++ i ) {
			if (uiwin.tid_au_lw_alter_urls->item(i)->text().compare(surl) == 0 ) {
				QMessageBox::information(this,"hehe","existed");
				return;
			}
		}
		uiwin.tid_au_lw_alter_urls->addItem(surl);
	}

}
void taskinfodlg::onDeleteAlterUrl() 
{
		int row = uiwin.tid_au_lw_alter_urls->count();
		QList<QListWidgetItem*> si;
		QList<int> sii;

		for (int r = 0; r < row; r ++)
		{
			if (uiwin.tid_au_lw_alter_urls->isItemSelected(uiwin.tid_au_lw_alter_urls->item(r))) {
				sii.append(r);
			}
		}
		for (int i = sii.size()-1; i >=0; i --) {
			delete uiwin.tid_au_lw_alter_urls->takeItem(sii.at(i));			
		}
}
void taskinfodlg::onShowCategoryInfo() 
{
	CatManDlg * cmd = new CatManDlg(this);

	cmd->exec();

	delete cmd;
}

void taskinfodlg::onChangeSaveDirectory()
{
	QString saveDir = this->uiwin.tid_g_cb_save_to->currentText();

	QString newDir = QFileDialog::getExistingDirectory(this);

	if (newDir.isEmpty() || newDir.length() == 0) {
	} else {
		this->uiwin.tid_g_cb_save_to->setEditText(newDir);
	}
}

void taskinfodlg::onCatListClicked( const QModelIndex & index )
{
	qDebug()<<__FUNCTION__ << index.data();
	QModelIndex idx0 , idx;
	idx0 = index.model()->index(index.row(), 0, index.parent());
	idx = index.model()->index(index.row(), 1, index.parent());
	qDebug()<<this->uiwin.tid_g_le_cb_category->currentText();
	this->uiwin.tid_g_le_cb_category->setEditText(idx0.data().toString());
	qDebug()<<this->uiwin.tid_g_le_cb_category->currentText();

	mSwapValue = idx0.data().toString();
    this->mCatId = idx0.internalId();

	//this->mCatLineEdit->setText(index.model()->index(index.row(),0).data().toString());
	this->uiwin.tid_g_cb_save_to->setEditText(idx.data().toString());
	qDebug()<<this->uiwin.tid_g_le_cb_category->currentIndex()<<this->uiwin.tid_g_le_cb_category->count()
            <<this->uiwin.tid_g_le_cb_category->modelColumn ();
}

void taskinfodlg::expandAll(QModelIndex  index )
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

