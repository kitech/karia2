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
    QObject::connect(this->uiwin.checkBox_21, SIGNAL(toggled(bool)),
                     this, SLOT(onMonitorOpera(bool)));

    QObject::connect(this->uiwin.checkBox_18, SIGNAL(toggled(bool)),
                     this, SLOT(onMonitorIE(bool)));

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

#if !defined(Q_OS_WIN)
    this->uiwin.checkBox_18->setEnabled(false);
#endif
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
        if (!this->monitorLoaded) {
            this->loadMonitorOptions();
        }
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

void PreferencesDialog::loadMonitorOptions()
{
    QString optionName;
    QString optionValue;

    optionValue = this->loadKey("monitoropera", "false");

    if (optionValue == "true") {
        this->uiwin.checkBox_21->setChecked(true);
    } else {
        this->uiwin.checkBox_21->setChecked(false);        
    }
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

// need user open this program before click ie Download menu
void PreferencesDialog::onMonitorIE(bool checked)
{
    QSettings winreg("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\MenuExt",
                     QSettings::NativeFormat);

    if (checked) {
        winreg.setValue("Download by NullGet/.", QString("Z:\\cross\\karia2-svn\\browser\\iegeturl.html"));
        // winreg.setValue("Download by NullGet/contexts", 0x00000002); // is img
        winreg.setValue("Download by NullGet/contexts", 0x00000022); // is link
    } else {
        winreg.remove("Download by NullGet/.");
        winreg.remove("Download by NullGet/contexts");
    }

    winreg.sync();
}

void onMonitorOperaWin(bool checked)
{
    
}

// modify opera's operaprefs.ini and menu/xxxmenu.ini
void PreferencesDialog::onMonitorOpera(bool checked)
{
    qDebug()<<__FUNCTION__<<checked;

#if defined(Q_OS_WIN)
    QSettings winreg("HKEY_CURRENT_USER\\Software\\Opera Software", QSettings::NativeFormat);
    QString operaExecFile = winreg.value("Last CommandLine").toString();
    int exeNamePos = operaExecFile.indexOf("opera.exe", 0, Qt::CaseInsensitive);
    if (exeNamePos == -1) {
        operaExecFile = winreg.value("Last CommandLine v2").toString();
        exeNamePos = operaExecFile.indexOf("opera.exe", 0, Qt::CaseInsensitive);
    }
    if (exeNamePos == -1) {
        operaExecFile = winreg.value("Last CommandLine v3").toString();
        exeNamePos = operaExecFile.indexOf("opera.exe", 0, Qt::CaseInsensitive);
    }
    if (exeNamePos == -1) {
        qDebug()<<"Can not checking opera installation.";
        return;
    }

    QString operaDir = operaExecFile.left(exeNamePos);
    QString operaPersonalDir = QDir::homePath() + "/Application Data/Opera/Opera";
    if (!QDir().exists(operaPersonalDir)) {
        qDebug()<<"Can not find opera personal directory.";
        return;
    }
    QString execProgramValue = QString("Execute program,%1Z:\\cross\\karia2-svn\\NullGet.exe%1,%1--uri %l --refer %u%1,,%1nullget%1").arg(QString("\""));
#else
    QString operaDir = "/usr/share/opera";
    QString operaPersonalDir = QDir::homePath() + "/.opera";
    QString execProgramValue = QString("Execute program,%1xterm -e /home/gzleo/karia2-svn/NullGet%1,%1--uri %l --refer %u%1,,%1nullget%1").arg(QString("\""));
#endif
    QString operaPrefs = operaPersonalDir + "/operaprefs.ini";

    QString appPath = QApplication::applicationFilePath();
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    Q_ASSERT(codec != NULL);
    QStringList menuInis, nameFilters;
    // QSettings setting;
    bool newIni = false;

    nameFilters << "*.ini";

    QDir pcDir(operaPersonalDir);
    if (!pcDir.exists("menu")) {
        pcDir.mkdir("menu"); 
    } 
    
    pcDir.cd("menu");
    menuInis = pcDir.entryList(nameFilters);

    if (menuInis.count() == 0) {
        newIni = true;
        QFile::copy(operaDir + "/ui/standard_menu.ini", operaPersonalDir + "/menu/standard_menu.ini");
        menuInis = pcDir.entryList(nameFilters);
    }
    if (menuInis.count() > 0) {
        QString key = QString("Item, %1%2%1").arg("\"").arg(tr("Download By NullGet"));
        QByteArray line;
        QList<QByteArray> popMenus;
        qint64 overrideBegin = -1, overrideEnd = -1;

        for (int i = 0; i < menuInis.count() ; i ++) {
            QString curmenu = operaPersonalDir + "/menu/" + menuInis.at(i);
            QFile mfile(curmenu);
            mfile.open(QIODevice::ReadWrite);
            while (!mfile.atEnd()) {
                line = mfile.readLine();
                if (line.trimmed() == "[Link Popup Menu]") {
                    overrideBegin = mfile.pos() - line.length();
                    popMenus << line;
                    while (!mfile.atEnd()) {
                        line = mfile.readLine();
                        if (line.indexOf("NullGet") != -1 || line.indexOf("Nullget") != -1) {
                            continue;
                        } else if (line.startsWith("[")) {
                            overrideEnd = mfile.pos() - line.length();
                            break;
                        } else {
                            popMenus << line;
                        }
                    }
                    break;
                }
            }
            // qDebug()<<overrideBegin<<overrideEnd;
            // Q_ASSERT(overrideEnd > overrideBegin && overrideBegin >= 0);
            
            QByteArray spaceArray;
            if (overrideEnd > overrideBegin && overrideBegin >=0) {
                spaceArray = line;
            }
            spaceArray += mfile.readAll();

            if (overrideBegin >= 0 && overrideEnd == -1) {
                overrideEnd = mfile.size();
            }

            mfile.resize(overrideBegin);
            mfile.seek(overrideBegin);
            mfile.write(spaceArray);

            // mfile.seek(mfile.size());
            // mfile.write(QByteArray("\n"));
            for (int i = 0 ; i < popMenus.count() ; i++) {
                mfile.write(popMenus.at(i));
            }
            if (checked) {
                mfile.write(codec->fromUnicode(key));
                mfile.write(QString("=%1\n").arg(execProgramValue).toAscii());
            } 

        }
    }

    // 
    // [User Prefs]
    // Menu Configuration=$OPERA_PERSONALDIR/menu/standard_menu_1.ini
    if (newIni) {
        QString value = QString("$OPERA_PERSONALDIR/menu/standard_menu.ini");
        QString key = QString("Menu Configuration");
        QByteArray line;
        QList<QByteArray> popMenus;
        qint64 overrideBegin = -1, overrideEnd = -1;

        QFile mfile(operaPrefs);
        mfile.open(QIODevice::ReadWrite);
        while (!mfile.atEnd()) {
            line = mfile.readLine();
            if (line.trimmed() == "[User Prefs]") {
                overrideBegin = mfile.pos() - line.length();
                popMenus << line;
                while (!mfile.atEnd()) {
                    line = mfile.readLine();
                    if (line.trimmed().startsWith(key.toAscii())) {
                    } else if (line.startsWith("[")) {
                        overrideEnd = mfile.pos() - line.length();
                        break;
                    } else {
                        popMenus << line;
                    }
                }
                break;
            }
        }
        if (overrideBegin >= 0 && overrideEnd == -1) {
            overrideEnd = mfile.size();
        }
        // Q_ASSERT(overrideEnd > overrideBegin && overrideBegin >= 0);

        QByteArray spaceArray;
        if (overrideEnd > overrideBegin && overrideBegin >=0) {
            spaceArray = line;
        }
        spaceArray += mfile.readAll();

        if (overrideBegin >= 0 && overrideEnd == -1) {
            overrideEnd = mfile.size();
        }


        mfile.resize(overrideBegin);
        mfile.seek(overrideBegin);
        mfile.write(spaceArray);

        // mfile.seek(mfile.size());
        mfile.write(QByteArray("\n"));
        for (int i = 0 ; i < popMenus.count() ; i++) {
            mfile.write(popMenus.at(i));
        }
        mfile.write(codec->fromUnicode(key));
        mfile.write(QString("=%1\n").arg(value).toAscii());
            
    }

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
