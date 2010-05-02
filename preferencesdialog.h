// preferencesdialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-16 21:52:11 +0800
// Version: $Id$
// 

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtCore>
#include <QtGui>

#include <QDialog>


#include "ui_preferencesdialog.h"
#include "ui_proxyinfodialog.h"

class SqliteStorage;

//////////////////////
class ProxyInfoDialog : public QDialog
{
    Q_OBJECT;
public:
    ProxyInfoDialog(QWidget *parent = 0);
    ~ProxyInfoDialog();

    QString getProxy();

private:
    Ui::ProxyInfoDialog uiwin;
};


///////////
////////
//////////
class PreferencesDialog : public QDialog
{
    Q_OBJECT;
public:
    PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

public slots:
    void setWindowModified(bool modified);

private:
    Ui::PreferencesDialog uiwin;
    // bool mIsModified;
    SqliteStorage *storage;

    bool generalLoaded;
    bool defaultPropertiesLoaded;
    bool connectionLoaded;
    bool monitorLoaded;
    bool graphLogLoaded;
    bool virtusLoaded;
    bool proxyLoaded;
    bool advancedLoaded;

    bool generalModified;
    bool defaultPropertiesModified;
    bool connectionModified;
    bool monitorModified;
    bool graphLogModified;
    bool virtusModified;
    bool proxyModified;
    bool advancedModified;

    QVector<QString> removedProxys;

private slots:
	void onPreferencesSelectChanged(int index);
    void loadStatus(QString msg);
    void saveStatus(QString msg, QString value);
    QString loadKey(QString key, QString dvalue);
    bool saveKey(QString key, QString dvalue, QString type = "auto");
    QVector<QPair<QString, QString> > loadKeyByType(QString type); // for proxy setting

    void loadGeneralOptions();
    void loadDefaultProperties();
    void loadConnectionOptions();
    void loadMonitorOptions();
    void loadProxyOptions();
    void onApplyAllChange();  // for all tab, maybe need an other button for apply all
    void onApplyChange(); // for current tab

    void onApplyDefaultPropertiesTabChange();
    void onApplyConnectionTabChange();
    void onApplyMonitorTabChange();
    void onApplyProxyTabChange();

    void onMonitorIE(bool checked);
    void onMonitorOpera(bool checked);
    void onMonitorOpera_2(bool checked);
    void onMonitorFirefox(bool checked);
    

    void onNoProxyChecked(bool checked);
    void onCustomProxyChecked();
    void onAddProxy();
    void onModifyProxy();
    void onDeleteProxy();
    void onApplyProxy();

    void connectModifyControlSignalRecursive(QObject *parent, const char *slot);
    void connectModifyControlSignal();
    void onSetGeneralTabModified();
    void onSetGeneralTabUnmodified();
    void onSetDefaultPropertiesTabModified();
    void onSetDefaultPropertiesTabUnmodified();
    void onSetConnectionTabModified();
    void onSetConnectionTabUnmodified();
    void onSetMonitorTabModified();
    void onSetMonitorTabUnmodified();
    void onSetProxyTabModified();
    void onSetProxyTabUnmodified();
};

#endif // PREFERENCESDIALOG_H
