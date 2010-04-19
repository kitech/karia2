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


//////////////////
ProxyInfoDialog::ProxyInfoDialog(QWidget *parent)
    : QDialog(parent)
{
    this->uiwin.setupUi(this);
}

ProxyInfoDialog::~ProxyInfoDialog()
{
}

QString ProxyInfoDialog::getProxy()
{
    QString type;
    if (this->uiwin.radioButton_2->isChecked()) {
        type = "SOCKS";
    } else {
        type = "HTTP";
    }
    if (this->uiwin.lineEdit_3->text().toInt() <= 0) {
        
    }
    QString proxy = QString("%1://%2:%3@%4:%5/#%6")
        .arg(type).arg(this->uiwin.lineEdit_4->text())
        .arg(this->uiwin.lineEdit_5->text())
        .arg(this->uiwin.lineEdit_2->text())
        .arg(this->uiwin.lineEdit_3->text())
        .arg(this->uiwin.lineEdit->text());

    qDebug()<<proxy;
    return proxy;
}


//////////////////////
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

    QObject::connect(this->uiwin.radioButton, SIGNAL(toggled(bool)),
                     this, SLOT(onNoProxyChecked(bool)));

    QObject::connect(this->uiwin.toolButton_2, SIGNAL(clicked()),
                     this, SLOT(onAddProxy()));
    QObject::connect(this->uiwin.toolButton_3, SIGNAL(clicked()),
                     this, SLOT(onModifyProxy()));
    QObject::connect(this->uiwin.toolButton_4, SIGNAL(clicked()),
                     this, SLOT(onDeleteProxy()));
    QObject::connect(this->uiwin.toolButton_5, SIGNAL(clicked()),
                     this, SLOT(onApplyProxy()));

    this->show();

    this->generalLoaded = false;;
    this->defaultPropertiesLoaded = false;
    this->connectionLoaded = false;
    this->monitorLoaded = false;
    this->graphLogLoaded = false;
    this->virtusLoaded = false;
    this->proxyLoaded = false;
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
        if (!this->proxyLoaded) {
            this->uiwin.tableWidget->setColumnCount(6);
            QStringList proxyHeaderText;
            proxyHeaderText<<tr("Proxy Name")<<tr("Host Name")<<tr("Port")
                           <<tr("Proxy Type")<<tr("User Name")<<tr("Password");
            this->uiwin.tableWidget->setHorizontalHeaderLabels(proxyHeaderText);
            this->loadProxyOptions();
        }
        break;
    case 6:
        break;
    case 7:
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

QVector<QPair<QString, QString> > PreferencesDialog::loadKeyByType(QString type)
{
    QVector<QPair<QString, QString> > lines;

    lines = this->storage->getUserOptionsByType(type);

    return lines;
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

void PreferencesDialog::loadProxyOptions()
{
    QString optionName;
    QString optionValue;

    optionValue = this->loadKey("noproxy", "true");
    if (optionValue == "true") {
        this->onNoProxyChecked(true);
    } else {
        this->onNoProxyChecked(false);
    }
    
    optionValue = this->loadKey("customproxy", "false");
    
    // name, proxy(scheme://user:pass@host:port)
    QVector<QPair<QString, QString> > proxys = this->loadKeyByType("proxy");
    if (proxys.count() > 0) {
        for (int i = 0 ; i < proxys.count() ; ++i) {
            QUrl pu(proxys.at(i).second);
            this->uiwin.tableWidget->insertRow(i);
            QTableWidgetItem *item = NULL;

            item = new QTableWidgetItem(proxys.at(i).first);
            this->uiwin.tableWidget->setItem(i, 0, item);

            item = new QTableWidgetItem(pu.host());
            this->uiwin.tableWidget->setItem(i, 1, item);

            item = new QTableWidgetItem(QString("%1").arg(pu.port()));
            this->uiwin.tableWidget->setItem(i, 2, item);

            item = new QTableWidgetItem(pu.scheme());
            this->uiwin.tableWidget->setItem(i, 3, item);

            item = new QTableWidgetItem(pu.userName());
            this->uiwin.tableWidget->setItem(i, 4, item);

            item = new QTableWidgetItem(pu.password());
            this->uiwin.tableWidget->setItem(i, 5, item);
        }
    }

    this->proxyLoaded = true;
}

void PreferencesDialog::saveAllOptions()
{

}

void PreferencesDialog::onNoProxyChecked(bool checked)
{
    if (checked) {
        this->uiwin.comboBox->setEnabled(false);
        this->uiwin.comboBox_2->setEnabled(false);
        this->uiwin.comboBox_3->setEnabled(false);
        this->uiwin.comboBox_4->setEnabled(false);
    } else {
        this->uiwin.comboBox->setEnabled(true);
        this->uiwin.comboBox_2->setEnabled(true);
        this->uiwin.comboBox_3->setEnabled(true);
        this->uiwin.comboBox_4->setEnabled(true);
    }
}

void PreferencesDialog::onCustomProxyChecked()
{

}

void PreferencesDialog::onAddProxy()
{
    ProxyInfoDialog *proxyDialog = new ProxyInfoDialog();
    QString proxy;
    
    if (proxyDialog->exec() == QDialog::Accepted) {
        proxy = proxyDialog->getProxy();
        QUrl pu(proxy);
        qDebug()<<pu;
        this->uiwin.tableWidget->insertRow(0);
        QTableWidgetItem *item = NULL;
        item = this->uiwin.tableWidget->item(0, 0);
        if (item == NULL) {
            item = new QTableWidgetItem(pu.fragment());
        }
        this->uiwin.tableWidget->setItem(0, 0, item);

        item = this->uiwin.tableWidget->item(0, 1);
        if (item == NULL) {
            item = new QTableWidgetItem(pu.host());
        }
        this->uiwin.tableWidget->setItem(0, 1, item);

        item = this->uiwin.tableWidget->item(0, 2);
        if (item == NULL) {
            item = new QTableWidgetItem(QString("%1").arg(pu.port()));
        }
        this->uiwin.tableWidget->setItem(0, 2, item);

        item = this->uiwin.tableWidget->item(0, 3);
        if (item == NULL) {
            item = new QTableWidgetItem(pu.scheme());
        }
        this->uiwin.tableWidget->setItem(0, 3, item);

        item = this->uiwin.tableWidget->item(0, 4);
        if (item == NULL) {
            item = new QTableWidgetItem(pu.userName());
        }
        this->uiwin.tableWidget->setItem(0, 4, item);

        item = this->uiwin.tableWidget->item(0, 5);
        if (item == NULL) {
            item = new QTableWidgetItem(pu.password());
        }
        this->uiwin.tableWidget->setItem(0, 5, item);
    }

    delete proxyDialog;
}

void PreferencesDialog::onModifyProxy()
{
    
}

void PreferencesDialog::onDeleteProxy()
{
    
}

void PreferencesDialog::onApplyProxy()
{
    
}
