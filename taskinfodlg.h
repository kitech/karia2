// taskinfodlg.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-08 00:16:48 +0800
// Version: $Id$
// 
#ifndef TASKINFODLG_H
#define TASKINFODLG_H

#include <QTreeView>
#include <QAbstractItemDelegate>
#include <QItemDelegate>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include "ui_taskinfodlg.h"

class CategoryComboBoxItemDelegate: public QAbstractItemDelegate
{
	Q_OBJECT;
public:
	CategoryComboBoxItemDelegate(QObject * parent = 0 );
	~CategoryComboBoxItemDelegate();
	virtual QWidget * createEditor ( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const ;
	virtual void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	virtual QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const ;

private:
	QTreeView * mCatView ;

};

class TaskOption
{
    //	Q_OBJECT;
public:
    enum {START_MANUAL, START_IMMIDATE};
public:
	TaskOption(QObject *parent=0);
	~TaskOption();

	void setDefaultValue();

	void setUrl(QString url) {
        this->mOldUrl = this->mTaskUrl;
        this->mTaskUrl = url;
    }
	
	void dump();

    static TaskOption *fromModelRow(QAbstractItemModel *model, int row);

    QByteArray toRawData();
    QString toBase64Data();
    static TaskOption fromRawData(QByteArray ba);
    static TaskOption fromBase64Data(QString data);

    // use in app level, full data 
    QHash<QString, QString> toHash();
    static TaskOption fromHash(QHash<QString, QString> oph);

//private:

	//general
	QString mTaskUrl;
    QString mOldUrl;
	int mFindUrlByMirror;
	QString mReferer;
    QString mCookies;
    QString mAgent;
	QString mCategory;
    int mCatId;
	QString mSavePath;
	QString mSaveName;
	int mSplitCount; 
	int mNeedLogin;
	QString mLoginUserName;
	QString mLoginPassword;
	QString mComment;
	int mStartState;

	////////
	QStringList mAlterUrls;
	
	//
	int mAutoDownSubdirFromFtp;
	int mAutoCreateSubdirLocally; 
	int mAutoCreateCategory;
	
	int mProxyTypeHttp;
	int mProxyTypeFtp;
	int mProxyTypeMedia;

};

class taskinfodlg : public QDialog
{
    Q_OBJECT;
public:
    taskinfodlg(QWidget *parent = 0);
	taskinfodlg(TaskOption * param, QWidget *parent = 0);
    ~taskinfodlg();

public:
	QString taskUrl(){ return this->uiwin.tid_g_le_url->text(); }
	int		segmentCount() { return this->uiwin.tid_g_sb_split_file->value() ; } 

	void setTaskUrl (QString url) { this->uiwin.tid_g_le_url->setText(url); }
	void setSegmentCount(int segCount) { this->uiwin.tid_g_sb_split_file->setValue(segCount) ; }
	void setRename(QString rename) { this->uiwin.tid_g_le_le_rename->setText(rename) ; }
	
	TaskOption *getOption();

public slots:
    void onDownloadNow();
    void onDownloadManual();

private:
    Ui::taskinfodlg uiwin;

	CategoryComboBoxItemDelegate * mCCID;

	QAbstractItemModel * mCatModel;

	QTreeView *mCatView;
	QLineEdit *mCatLineEdit;

    int mCatId;
	// QString mSwapValue;
    int mStartState;

	///////////////
	void expandAll(QModelIndex  index );

private slots:
	void onUrlBoxChange(QString text=QString()) ;
	void onCategoryBoxChange(int index);
	// void onCategoryBoxChange(const QString & text );
	
	void onAddAlterUrl();
	void onDeleteAlterUrl() ;
	void onChangeSaveDirectory() ;
	void onShowCategoryInfo() ;

	// void onCatListClicked( const QModelIndex & index );
	void onCatListSelectChange( const QItemSelection & curr , const QItemSelection & prev  ) ;

    void onShowMoreInfo();
};



#endif // TASKINFODLG_H
