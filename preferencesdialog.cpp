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
	// mIsModified = false;

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

    QObject::connect(this->uiwin.checkBox_22, SIGNAL(toggled(bool)),
                     this, SLOT(onMonitorFirefox(bool)));

    QObject::connect(this->uiwin.pushButton, SIGNAL(clicked()),
                     this, SLOT(onApplyChange()));

    this->connectModifyControlSignal();
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

    this->generalModified = false;
    this->defaultPropertiesModified = false;
    this->connectionModified = false;
    this->monitorModified = false;
    this->graphLogModified = false;
    this->virtusModified = false;
    this->proxyModified = false;
    this->advancedModified = false;

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
            this->generalLoaded = true;
            this->generalModified = false;
        }
        // this->setWindowModified(this->generalModified);
        if (this->generalModified) {
            this->onSetGeneralTabModified();
        } else {
            this->onSetGeneralTabUnmodified();
        }
        break;
    case 1:
        if (!this->defaultPropertiesLoaded) {
            this->loadDefaultProperties();
            this->defaultPropertiesLoaded = true;
            this->defaultPropertiesModified = false;
        }
        // this->setWindowModified(this->defaultPropertiesModified);
        if (this->defaultPropertiesModified) {
            this->onSetDefaultPropertiesTabModified();
        } else {
            this->onSetDefaultPropertiesTabUnmodified();
        }
        break;
    case 2:
        if (!this->connectionLoaded) {
            this->loadConnectionOptions();
            this->connectionLoaded = true;
            this->connectionModified = false;
        }
        // this->setWindowModified(this->connectionModified);
        if (this->connectionModified) {
            this->onSetConnectionTabModified();
        } else {
            this->onSetConnectionTabUnmodified();
        }
        break;
    case 3:
        if (!this->monitorLoaded) {
            this->loadMonitorOptions();
            this->monitorLoaded = true;
            this->monitorModified = false;
        }
        // this->setWindowModified(this->monitorModified);
        if (this->monitorModified) {
            this->onSetMonitorTabModified();
        } else {
            this->onSetMonitorTabUnmodified();
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
            this->proxyLoaded = true;
            this->proxyModified = false;
        }
        // this->setWindowModified(this->proxyModified);
        if (this->proxyModified) {
            this->onSetProxyTabModified();
        } else {
            this->onSetProxyTabUnmodified();
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
void PreferencesDialog::saveStatus(QString msg, QString value)
{
    this->uiwin.label_5->setText(QString("Saveing %1=%2").arg(msg, value));
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
            this->storage->addDefaultOption(key, dvalue, "auto");
            ov = dvalue;
        } else {
            ov = optionValue;
        }
    } else {
        ov = optionValue;
    }

    return ov;
}

bool PreferencesDialog::saveKey(QString key, QString value, QString type)
{
    QString ov = QString::null;
    QString optionValue;

    this->saveStatus(key, value);
    optionValue = this->loadKey(key, "");
    if (optionValue != value) {
        this->storage->deleteUserOption(key);
        this->storage->addUserOption(key, value, type);
    }

    return true;
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

    optionValue = this->loadKey("monitorfirefox", "false");

    if (optionValue == "true") {
        this->uiwin.checkBox_22->setChecked(true);
    } else {
        this->uiwin.checkBox_22->setChecked(false);        
    }
}

void PreferencesDialog::loadProxyOptions()
{
    QString optionName;
    QString optionValue;

    optionValue = this->loadKey("noproxy", "true");
    if (optionValue == "true") {
        this->onNoProxyChecked(true);
        this->uiwin.radioButton->setChecked(true);
    } else {
        this->onNoProxyChecked(false);
        this->uiwin.radioButton_2->setChecked(true);
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

            // update proto proxy select combobox
            QVector<QComboBox*> protoProxys;
            protoProxys<<this->uiwin.comboBox<<this->uiwin.comboBox_2
                       <<this->uiwin.comboBox_3<<this->uiwin.comboBox_4;
            for (int c = 0 ; c < protoProxys.count(); ++c) {
                if (protoProxys.at(c)->findText(proxys.at(i).first) == -1) {
                    protoProxys.at(c)->addItem(proxys.at(i).first);
                }
            }
        }

        // 
        optionValue = this->loadKey("httpproxy", "Direct");
        if (!optionValue.isEmpty()) {
            this->uiwin.comboBox->setCurrentIndex(this->uiwin.comboBox->findText(optionValue));
        }
        optionValue = this->loadKey("ftpproxy", "Direct");
        if (!optionValue.isEmpty()) {
            this->uiwin.comboBox_2->setCurrentIndex(this->uiwin.comboBox_2->findText(optionValue));
        }
        optionValue = this->loadKey("bittorrentproxy", "Direct");
        if (!optionValue.isEmpty()) {
            this->uiwin.comboBox_3->setCurrentIndex(this->uiwin.comboBox_3->findText(optionValue));
        }
        optionValue = this->loadKey("metalinkproxy", "Direct");
        if (!optionValue.isEmpty()) {
            this->uiwin.comboBox_4->setCurrentIndex(this->uiwin.comboBox_4->findText(optionValue));
        }

    }

    this->proxyLoaded = true;
}

void PreferencesDialog::onApplyDefaultPropertiesTabChange()
{
    this->saveKey("defaultrefer", this->uiwin.lineEdit_3->text());
    this->saveKey("taskstartschedule", this->uiwin.radioButton_3->isChecked() 
                  ? "manual" : "imidiate");
    this->saveKey("maxsegmenteverytask", QString("%1").arg(this->uiwin.spinBox_8->value()));
}

void PreferencesDialog::onApplyConnectionTabChange()
{
    this->saveKey("connecttimeout", QString().setNum(this->uiwin.spinBox_2->value()));
    this->saveKey("readdatatimeout", QString().setNum(this->uiwin.spinBox_3->value()));
    this->saveKey("retrydelaytimeout", QString().setNum(this->uiwin.spinBox_4->value()));
    this->saveKey("maxsimulatejobs", QString().setNum(this->uiwin.spinBox_5->value()));
}

void PreferencesDialog::onApplyMonitorTabChange()
{
    // QString optionName;
    // QString optionValue;

    this->saveKey("monitorie", this->uiwin.checkBox_18->isChecked() ? "true" : "false");
    this->saveKey("monitoropera", this->uiwin.checkBox_21->isChecked() ? "true" : "false");
    this->saveKey("monitorfirefox", this->uiwin.checkBox_22->isChecked() ? "true" : "false");
}

void PreferencesDialog::onApplyProxyTabChange()
{
    this->saveKey("noproxy", this->uiwin.radioButton->isChecked() 
                  ? "true" : "false");
    this->saveKey("customproxy", this->uiwin.radioButton_2->isChecked()
                  ? "true" : "false");

    // item = this->uiwin.tableWidget->item(0, 1);
    QString pname, phost, pport, pusername, ppassword, ptype, proxy;
    int proxyCount = this->uiwin.tableWidget->rowCount();
    for (int i = 0; i < proxyCount; ++i) {
        pname = this->uiwin.tableWidget->item(i, 0)->text();
        phost = this->uiwin.tableWidget->item(i, 1)->text();
        pport = this->uiwin.tableWidget->item(i, 2)->text();
        ptype = this->uiwin.tableWidget->item(i, 3)->text();
        pusername = this->uiwin.tableWidget->item(i, 4)->text();
        ppassword = this->uiwin.tableWidget->item(i, 5)->text();

        proxy = QString("%1://%2:%3@%4:%5/#%6")
            .arg(ptype, pusername, ppassword,
                 phost, pport, pname);

        this->saveKey(pname, proxy, "proxy");
    }

    this->saveKey("httpproxy", this->uiwin.comboBox->currentText());
    this->saveKey("ftpproxy", this->uiwin.comboBox_2->currentText());
    this->saveKey("bittorrentproxy", this->uiwin.comboBox_3->currentText());
    this->saveKey("metalinkproxy", this->uiwin.comboBox_4->currentText());

    for (int i = 0; i < this->removedProxys.count(); ++i) {
        this->storage->deleteUserOption(removedProxys.at(i));
    }
    this->removedProxys.clear();
}

void PreferencesDialog::onApplyAllChange()
{

    QApplication::setOverrideCursor(Qt::WaitCursor);
    int currentIndex = this->uiwin.stackedWidget->currentIndex();
    int tabCount = this->uiwin.stackedWidget->count();

    for (int i = 0 ; i < tabCount; ++i) {
        this->uiwin.stackedWidget->setCurrentIndex(i);
        this->onApplyChange();
    }

    this->uiwin.stackedWidget->setCurrentIndex(currentIndex);
    QApplication::restoreOverrideCursor();
}

void PreferencesDialog::onApplyChange()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page) {

    } else if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_2) {
        this->onSetDefaultPropertiesTabUnmodified();
        this->onApplyDefaultPropertiesTabChange();
    } else if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_3) {
        this->onSetConnectionTabUnmodified();
        this->onApplyConnectionTabChange();
    } else if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_4) {
        this->onSetMonitorTabUnmodified();
        this->onApplyMonitorTabChange();
    } else if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_5) {

    } else if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_15) {
        this->onSetProxyTabUnmodified();
        this->onApplyProxyTabChange();
    } else if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_11) {

    } else if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_6) {

    }
    QApplication::restoreOverrideCursor();
}

// need user open this program before click ie Download menu
void PreferencesDialog::onMonitorIE(bool checked)
{
    QSettings winreg("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\MenuExt",
                     QSettings::NativeFormat);

    QString geturlFilePath = QCoreApplication::applicationDirPath() + "/browser/iegeturl.html";
    geturlFilePath = QDir::toNativeSeparators(geturlFilePath);
    if (checked) {
        // winreg.setValue("Download by NullGet/.", QString("Z:\\cross\\karia2-svn\\browser\\iegeturl.html"));
        winreg.setValue("Download by NullGet/.", geturlFilePath);
        // winreg.setValue("Download by NullGet/contexts", 0x00000002); // is img
        winreg.setValue("Download by NullGet/contexts", 0x00000022); // is link
    } else {
        winreg.remove("Download by NullGet/.");
        winreg.remove("Download by NullGet/contexts");
    }

    winreg.sync();
}

// http://www.crsky.com/soft/16475.html ok
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
    // QString execProgramValue = QString("Execute program,%1Z:\\cross\\karia2-svn\\NullGet.exe%1,%1--uri %l --refer %u%1,,%1nullget%1").arg(QString("\""));
    // QString execProgramValue = QString("Execute program,%1%2%1,%1--uri %l --refer %u%1,,%1nullget%1").arg(QString("\"")).arg(QCoreApplication::applicationFilePath());

#elif defined(Q_OS_MAC)
    QString operaDir = "/Applications/Opera.app/Contents/Resources";
    QString operaPersonalDir = QDir::homePath() + "/Library/Preferences/Opera Preferences";

    // QString execProgramValue = QString("Execute program,%1xterm -e %2,%1--uri %l --refer %u%1,,%1nullget%1")
    // .arg(QString("\"")).arg(QCoreApplication::applicationFilePath());

#else
    QString operaDir = "/usr/share/opera";
    QString operaPersonalDir = QDir::homePath() + "/.opera";
    // QString execProgramValue = QString("Execute program,%1xterm -e /home/gzleo/karia2-svn/NullGet%1,%1--uri %l --refer %u%1,,%1nullget%1").arg(QString("\""));
    // QString execProgramValue = QString("Execute program,%1xterm -e %2%1, %1--uri %l --refer %u%1,,%1nullget%1")
    // .arg(QString("\"")).arg(QCoreApplication::applicationFilePath());
    // QString execProgramValue = 
    
#endif
    QFile file(QApplication::applicationDirPath() + "/browser/opera_go_to_page.dat");
    file.open(QIODevice::ReadOnly);
    QByteArray itemValue = file.readAll().trimmed();
    file.close();

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
                // mfile.write(QString("=%1\n").arg(execProgramValue).toAscii());
                mfile.write(QByteArray("=") + itemValue);
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

    // [File Types]
    // for mime type handler
    {
#if defined(Q_OS_WIN)
        // QString value = QString("3,%1 --metafile,,,,|").arg(QCoreApplication::applicationFilePath());
        QString value = QString("3,%1,,,,|").arg(QCoreApplication::applicationFilePath());
#elif defined(Q_OS_MAC)
        QString value = QString("3,xterm -e %1 --metafile,,,,|").arg(QCoreApplication::applicationFilePath());
#else
        QString value;
        if (QFile::exists("/usr/bin/konsole")) {
            value = QString("3,konsole -e %1 --metafile,,,,|")
                .arg(QCoreApplication::applicationFilePath());
        } else {
            value = QString("3,xterm -e %1 --metafile,,,,|")
                .arg(QCoreApplication::applicationFilePath());
        }
#endif
        QString key = QString("text/karia2_nullget_down");
        QByteArray line;
        QList<QByteArray> popMenus;
        qint64 overrideBegin = -1, overrideEnd = -1;

        QFile mfile(operaPrefs);
        mfile.open(QIODevice::ReadWrite);

        while (!mfile.atEnd()) {
            line = mfile.readLine();
            if (line.trimmed() == "[File Types]") {
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
            mfile.write(QString("=%1\n").arg(value).toAscii());
        }
    }

    // this->monitorModified = true;
    // qDebug()<<__FUNCTION__<<"set window modified: true";
    // this->setWindowModified(true);
}

// modify opera's operaprefs.ini and menu/xxxmenu.ini
void PreferencesDialog::onMonitorOpera_2(bool checked)
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
    // QString execProgramValue = QString("Execute program,%1Z:\\cross\\karia2-svn\\NullGet.exe%1,%1--uri %l --refer %u%1,,%1nullget%1").arg(QString("\""));
    QString execProgramValue = QString("Execute program,%1%2%1,%1--uri %l --refer %u%1,,%1nullget%1").arg(QString("\"")).arg(QCoreApplication::applicationFilePath());

#elif defined(Q_OS_MAC)
    QString operaDir = "/Applications/Opera.app/Contents/Resources";
    QString operaPersonalDir = QDir::homePath() + "/Library/Preferences/Opera Preferences";

    QString execProgramValue = QString("Execute program,%1xterm -e %2,%1--uri %l --refer %u%1,,%1nullget%1")
        .arg(QString("\"")).arg(QCoreApplication::applicationFilePath());

#else
    QString operaDir = "/usr/share/opera";
    QString operaPersonalDir = QDir::homePath() + "/.opera";
    // QString execProgramValue = QString("Execute program,%1xterm -e /home/gzleo/karia2-svn/NullGet%1,%1--uri %l --refer %u%1,,%1nullget%1").arg(QString("\""));
    QString execProgramValue = QString("Execute program,%1xterm -e %2%1, %1--uri %l --refer %u%1,,%1nullget%1")
        .arg(QString("\"")).arg(QCoreApplication::applicationFilePath());
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

// now we can only assoc with flashgot, not really firefox
void PreferencesDialog::onMonitorFirefox(bool checked)
{
#if defined(Q_OS_WIN)
    QString ffProfileBase = QDir::homePath() + "/Application data/Mozilla/firefox";
#else
    QString ffProfileBase = QDir::homePath() + "/.mozilla/firefox";
#endif
    QString profileName, profileFile;

    QSettings setting(ffProfileBase + "/profiles.ini", QSettings::IniFormat);
    QStringList groups = setting.childGroups();

    for (int i = 0 ; i < groups.count(); i ++) {
        if (groups.at(i).startsWith("profile", Qt::CaseInsensitive)) {
            setting.beginGroup(groups.at(i));
            profileName = setting.value("Path").toString();
            // firefox use it, but not override it
            profileFile = ffProfileBase + QString("/%1/user.js").arg(profileName); 
            QFile file(profileFile);
            file.open(QIODevice::ReadWrite);
            if (file.exists(profileFile)) {
            } else {
            }
            /*
              user_pref("flashgot.custom", "NullGet3");
              user_pref("flashgot.custom.NullGet3.args", "-e /home/gzleo/karia2-svn/NullGet --uri [URL] --refer [REFE
              RER]");
              user_pref("flashgot.custom.NullGet3.exe", "/usr/bin/xterm");
              user_pref("flashgot.defaultDM", "NullGet3");
              user_pref("flashgot.detect.cache", "浏览器创建于,Wget,NullGet3,DTA (Turbo),DTA,cURL,Aria 2");
              user_pref("flashgot.dmsopts.GetRight.quiet", false);
              user_pref("flashgot.dmsopts.JDownloader.path", "");
              user_pref("flashgot.dmsopts.NullGet3.shownInContextMenu", true);
              user_pref("flashgot.extensions", "");
              user_pref("flashgot.hide-all", false);
              user_pref("flashgot.hide-buildGallery", false);
              user_pref("flashgot.hide-it", false);
              user_pref("flashgot.hide-media", false);
              user_pref("flashgot.hide-options", false);
              user_pref("flashgot.hide-sel", false);
              user_pref("flashgot.includeImages", true);
              user_pref("flashgot.omitCookies", false);
             */
            file.resize(0);
            file.write(QByteArray("user_pref(\"flashgot.custom\", \"NullGet\");\n"));
#if defined(Q_OS_WIN)
            file.write(QByteArray("user_pref(\"flashgot.custom.NullGet.exe\", \"")
                       + QByteArray(QCoreApplication::applicationFilePath().toAscii())
                       + QByteArray("\");\n"));
            file.write(QByteArray("user_pref(\"flashgot.custom.NullGet.args\", \" --uri [URL] --refer [REFERER]\");\n"));
#else
            // file.write(QByteArray("user_pref(\"flashgot.custom.NullGet.exe\", \"/usr/bin/xterm\");\n"));
            // file.write(QByteArray("user_pref(\"flashgot.custom.NullGet.args\", \"-e /home/gzleo/karia2-svn/NullGet --uri [URL] --refer [REFERER]\");\n"));

            file.write(QByteArray("user_pref(\"flashgot.custom.NullGet.exe\", \"/usr/bin/xterm\");\n"));
            file.write(QByteArray("user_pref(\"flashgot.custom.NullGet.args\", \"-e ")
                       + QByteArray(QCoreApplication::applicationFilePath().toAscii())
                       + QByteArray(" --uri [URL] --refer [REFERER]\");\n"));
#endif

            file.write(QByteArray("user_pref(\"flashgot.defaultDM\", \"NullGet\");\n"));
            file.write(QByteArray("user_pref(\"flashgot.dmsopts.NullGet.shownInContextMenu\", true);\n"));
            file.close();
        }
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

        // update proto proxy select combobox
        QVector<QComboBox*> protoProxys;
        protoProxys<<this->uiwin.comboBox<<this->uiwin.comboBox_2
                   <<this->uiwin.comboBox_3<<this->uiwin.comboBox_4;
        for (int i = 0 ; i < protoProxys.count(); ++i) {
            if (protoProxys.at(i)->findText(pu.fragment()) == -1) {
                protoProxys.at(i)->addItem(pu.fragment());
            }
        }
    }

    delete proxyDialog;
}

void PreferencesDialog::onModifyProxy()
{
    
}

void PreferencesDialog::onDeleteProxy()
{
    qDebug()<<__FUNCTION__<<__LINE__;
    QString pname;
    int irow;
    QTableWidgetItem *item = this->uiwin.tableWidget->currentItem();
    if (item) {
        pname = this->uiwin.tableWidget->item(item->row(), 0)->text();
        this->uiwin.tableWidget->removeRow(item->row());
        // update proto proxy select combobox
        QVector<QComboBox*> protoProxys;
        protoProxys<<this->uiwin.comboBox<<this->uiwin.comboBox_2
                   <<this->uiwin.comboBox_3<<this->uiwin.comboBox_4;
        for (int i = 0 ; i < protoProxys.count(); ++i) {
            if ((irow = protoProxys.at(i)->findText(pname)) != -1) {
                protoProxys.at(i)->removeItem(irow);
            }
        }
        this->removedProxys << pname;
    } else {
        this->loadStatus("No proxy row selected");
    }
}

void PreferencesDialog::onApplyProxy()
{
    
}

// only for widget type is checkbox, radiobutton, textbox, sphinbox
void PreferencesDialog::connectModifyControlSignalRecursive(QObject *parent, const char *slot)
{
    const QMetaObject *mo = 0;
    QObject *obj;
    QWidget *w;
    QObjectList childs = parent->children();
    for (int i = 0 ; i < childs.count(); ++i) {
        obj = childs.at(i);
        mo = obj->metaObject();
        if (obj->isWidgetType()) {
            w = static_cast<QWidget*>(obj);
            if (QString(mo->className()) == QString(QCheckBox::staticMetaObject.className())
                || QString(mo->className()) == QString(QPushButton::staticMetaObject.className())
                || QString(mo->className()) == QString(QRadioButton::staticMetaObject.className())
                || QString(mo->className()) == QString(QToolButton::staticMetaObject.className())
                ) {
                QAbstractButton *ab = static_cast<QAbstractButton*>(w);
                QObject::connect(ab, SIGNAL(toggled(bool)), this, slot);
            } else if (QString(mo->className()) == QString(QSpinBox::staticMetaObject.className())
                || QString(mo->className()) == QString(QDoubleSpinBox::staticMetaObject.className())
                || QString(mo->className()) == QString(QDateTimeEdit::staticMetaObject.className())) {
                QAbstractSpinBox *as = static_cast<QAbstractSpinBox*>(w);
                QObject::connect(as, SIGNAL(editingFinished()), this, slot);
            } else if (QString(mo->className()) == QString(QLineEdit::staticMetaObject.className())) {
                QLineEdit *le = static_cast<QLineEdit*>(w);
                QObject::connect(le, SIGNAL(editingFinished()), this, slot);
            } else if (QString(mo->className()) == QString(QComboBox::staticMetaObject.className())) {
                QComboBox *cb = static_cast<QComboBox*>(w);
                QObject::connect(cb, SIGNAL(currentIndexChanged(int)), this, slot);
            }
        }
        this->connectModifyControlSignalRecursive(obj, slot);
    }
}


void PreferencesDialog::connectModifyControlSignal()
{
    QWidget *w;
    int tabCount = this->uiwin.stackedWidget->count();
    for (int i = 0; i < tabCount; ++i) {
        w = this->uiwin.stackedWidget->widget(i);

        if (w == this->uiwin.page) {
            this->connectModifyControlSignalRecursive(w, SLOT(onSetGeneralTabModified()));
        }
        if (w == this->uiwin.page_2) {
            this->connectModifyControlSignalRecursive(w, SLOT(onSetDefaultPropertiesTabModified()));
        }

        if (w == this->uiwin.page_3) {
            this->connectModifyControlSignalRecursive(w, SLOT(onSetConnectionTabModified()));
        }

        if (w == this->uiwin.page_4) {
            this->connectModifyControlSignalRecursive(w, SLOT(onSetMonitorTabModified()));
        }

        if (w == this->uiwin.page_15) {
            this->connectModifyControlSignalRecursive(w, SLOT(onSetProxyTabModified()));
        }
    }
    
}

void PreferencesDialog::onSetGeneralTabModified()
{
    qDebug()<<__FUNCTION__<<__LINE__<<"haha";
    this->generalModified = true;
    if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page) {
        this->setWindowModified(true);
        this->uiwin.pushButton->setEnabled(true);
        this->uiwin.pd_pb_restore_default_setting->setEnabled(true);
    }
}

void PreferencesDialog::onSetGeneralTabUnmodified()
{
    this->generalModified = false;
    if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page) {
        this->setWindowModified(false);
        this->uiwin.pushButton->setEnabled(false);
        this->uiwin.pd_pb_restore_default_setting->setEnabled(false);
    }
}
void PreferencesDialog::onSetDefaultPropertiesTabModified()
{
   this->defaultPropertiesModified = true;
   if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_2) {
       this->setWindowModified(true);
       this->uiwin.pushButton->setEnabled(true);
       this->uiwin.pd_pb_restore_default_setting->setEnabled(true);
   }
}
void PreferencesDialog::onSetDefaultPropertiesTabUnmodified()
{
    this->defaultPropertiesModified = false;
    if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_2) {
        this->setWindowModified(false);
        this->uiwin.pushButton->setEnabled(false);
        this->uiwin.pd_pb_restore_default_setting->setEnabled(false);
    }

}

void PreferencesDialog::onSetConnectionTabModified()
{
   this->connectionModified = true;
   if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_3) {
       this->setWindowModified(true);
       this->uiwin.pushButton->setEnabled(true);
       this->uiwin.pd_pb_restore_default_setting->setEnabled(true);
   }
    
}

void PreferencesDialog::onSetConnectionTabUnmodified()
{
    this->connectionModified = false;
    if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_3) {
        this->setWindowModified(false);
        this->uiwin.pushButton->setEnabled(false);
        this->uiwin.pd_pb_restore_default_setting->setEnabled(false);
    }

}

void PreferencesDialog::onSetMonitorTabModified()
{
    this->monitorModified = true;
    if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_4) {
        this->setWindowModified(true);
        this->uiwin.pushButton->setEnabled(true);
        this->uiwin.pd_pb_restore_default_setting->setEnabled(true);
    }

}

void PreferencesDialog::onSetMonitorTabUnmodified()
{
    this->monitorModified = false;
    if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_4) {
        this->setWindowModified(false);
        this->uiwin.pushButton->setEnabled(false);
        this->uiwin.pd_pb_restore_default_setting->setEnabled(false);
    }

}

void PreferencesDialog::onSetProxyTabModified()
{
    this->proxyModified = true;
    if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_15) {
        this->setWindowModified(true);
        this->uiwin.pushButton->setEnabled(true);
        this->uiwin.pd_pb_restore_default_setting->setEnabled(true);
    }

}

void PreferencesDialog::onSetProxyTabUnmodified()
{
    this->proxyModified = false;
    if (this->uiwin.stackedWidget->currentWidget() == this->uiwin.page_15) {
        this->setWindowModified(false);
        this->uiwin.pushButton->setEnabled(false);
        this->uiwin.pd_pb_restore_default_setting->setEnabled(false);
    }

}

void PreferencesDialog::setWindowModified(bool modified)
{
    QDialog::setWindowModified(modified);
    QString wtitle = this->windowTitle();
    if (wtitle.contains("*")) {
        
    }
}
