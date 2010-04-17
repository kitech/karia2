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

    this->generalLoaded = false;;
    this->defaultPropertiesLoaded = false;
    this->connectionLoaded = false;
    this->monitorLoaded = false;
    this->graphLogLoaded = false;
    this->virtusLoaded = false;
    this->advancedLoaded = false;

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
    
    QApplication::setOverrideCursor(Qt::WaitCursor);
    switch(index) {
    case 0:
        if (!this->generalLoaded) {
            this->loadGeneralOptions();
        }
        break;
    case 1:
        if (!this->defaultPropertiesLoaded) {
            this->loadDefaultProperties();
        }
        break;
    case 2:
        if (!this->connectionLoaded) {
            this->loadConnectionOptions();
        }
        break;
    case 3:
        break;
    case 4:
        break;
    case 5:
        break;
    case 6:
        break;
    default:
        Q_ASSERT(1 == 2);
        break;
    };
    QApplication::restoreOverrideCursor();
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

    this->generalLoaded = true;
}

void PreferencesDialog::loadDefaultProperties()
{
    QString optionName;
    QString optionValue;
    
    optionValue = this->loadKey("defaultrefer", "http://www.qtchina.net");
    this->uiwin.lineEdit_3->setText(optionValue);

    optionValue = this->loadKey("taskstartschedule", "imidiate");
    if (optionValue == "manual") {
        this->uiwin.radioButton_3->setChecked(true);
    } else {
        this->uiwin.radioButton_4->setChecked(true);
    }
    
    optionValue = this->loadKey("maxsegmenteverytask", "567");
    this->uiwin.spinBox_8->setValue(optionValue.toInt());

    this->defaultPropertiesLoaded = true;
}

void PreferencesDialog::loadConnectionOptions()
{
    QString optionName;
    QString optionValue;

    optionValue = this->loadKey("connecttimeout", "98");
    this->uiwin.spinBox_2->setValue(optionValue.toInt());

    optionValue = this->loadKey("readdatatimeout", "97");
    this->uiwin.spinBox_3->setValue(optionValue.toInt());

    optionValue = this->loadKey("retrydelaytimeout", "16");
    this->uiwin.spinBox_4->setValue(optionValue.toInt());

    optionValue = this->loadKey("maxsimulatejobs", "7");
    this->uiwin.spinBox_5->setValue(optionValue.toInt());

    this->connectionLoaded = true;
}

void PreferencesDialog::saveAllOptions()
{

}
