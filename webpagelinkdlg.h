// webpagelinkdlg.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-10 21:28:12 +0800
// Version: $Id$
// 
#ifndef WEBPAGELINKDLG_H
#define WEBPAGELINKDLG_H

#include <QDialog>
#include <QStringList>
#include <QString>

#include "ui_webpageurlinputdlg.h"
#include "ui_webpagelinkdlg.h"
#include "ui_webpagehostselectdlg.h"

class WebPageHostSelectDlg : public QDialog
{
    Q_OBJECT;
public:
    WebPageHostSelectDlg(QWidget *parent = 0);
    ~WebPageHostSelectDlg();

	void setHostList( QStringList host , QStringList ext);
	
	QList<QStringList> getResultHostList();

public slots:

	void onSelectAllHost();
	void onSelectAllExtension();

private:
	Ui::WebPageHostSelectClass ui;

	QStringList mAllHost;
	QStringList mAcceptedHost;
	QStringList mAllExtension;
	QStringList mAcceptedExtension ;


};

class WebPageLinkDlg : public QDialog
{
    Q_OBJECT;
public:
    WebPageLinkDlg(QWidget *parent = 0);
    ~WebPageLinkDlg();

	int exec () ;

	void setSourceUrlList(QStringList ul);
	QStringList getResultUrlList();

public slots:
	void onChangeDefaultMakeType();
	void onMarkHightLight();
	void onChooseHost();

private:
    Ui::WebPageLinkDlgClass ui;

	QStringList mSrcUrl;
	QStringList mResultUrl ;
	QStringList mAllHost;
	QStringList mAcceptedHost;
	QStringList mAllExtension;
	QStringList mAcceptedExtension ;
	QString mUserInputHost;

};



class WebPageUrlInputDlg : public QDialog
{
    Q_OBJECT

public:
    WebPageUrlInputDlg(QWidget *parent = 0);
    ~WebPageUrlInputDlg();

	QString getLinkList();

public slots:
	void onOpenFile();

private:
    Ui::WebPageUrlInputDlgClass ui;
};

#endif // WEBPAGELINKDLG_H
