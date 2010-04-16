// preferencesdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-16 21:52:25 +0800
// Version: $Id$
// 

#include "sqlitestorage.h"

#include "preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
{
	this->uiwin.setupUi(this);

    this->storage = SqliteStorage::instance();

	////////
	mIsModified = false;

	//QObject::connect(this->ui.listWidget,SIGNAL( currentRowChanged ( int  )),
	//	this->ui.stackedWidget,SLOT( setCurrentIndex ( int  ) ) );
	QObject::connect(this->uiwin.listWidget, SIGNAL(currentRowChanged(int)),
                     this, SLOT(onPreferencesSelectChanged(int)));

    this->show();
    this->loadGeneralOptions();
}

PreferencesDialog::~PreferencesDialog()
{

}

void PreferencesDialog::onPreferencesSelectChanged(int index)
{
	QString iconPath;
	QString title;

	//const QPixmap *oldIcon = this->ui.label_6->pixmap()
	QIcon newIcon = this->uiwin.listWidget->item(index)->icon() ;
	this->uiwin.label_6->setPixmap(newIcon.pixmap(this->uiwin.label_6->size()));
	title = this->uiwin.listWidget->item(index)->text();
	this->uiwin.label_7->setText(title);
	this->uiwin.stackedWidget->setCurrentIndex(index);
}

void PreferencesDialog::loadStatus(QString msg)
{
    this->uiwin.label_5->setText(QString("Loading %1").arg(msg));
}

QString PreferencesDialog::loadKey(QString key, QString dvalue)
{
    QString ov = QString::null;
    QString optionValue;

    this->loadStatus(key);
    optionValue = this->storage->getUserOption(key);
    if (optionValue == QString::null) {
        optionValue = this->storage->getDefaultOption(key);
        if (optionValue == QString::null) {
            this->storage->addDefaultOption(key, dvalue, "v");
            ov = dvalue;
        } else {
            ov = optionValue;
        }
    } else {
        ov = optionValue;
    }

    return ov;
}

void PreferencesDialog::loadGeneralOptions()
{
    QString optionName;
    QString optionValue;

    optionValue = this->loadKey("minsegmentsize", "1234");
    this->uiwin.spinBox->setValue(optionValue.toInt());

    optionValue = this->loadKey("autosavetaskinterval", "156");
    this->uiwin.lineEdit->setText(optionValue);

    optionValue = this->loadKey("writedatasize", "12345");
    this->uiwin.lineEdit_2->setText(optionValue);
}

void PreferencesDialog::saveAllOptions()
{

}
