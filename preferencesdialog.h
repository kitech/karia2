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

private:
    Ui::PreferencesDialog uiwin;
    bool  mIsModified;
    SqliteStorage *storage;

    bool generalLoaded;
    bool defaultPropertiesLoaded;
    bool connectionLoaded;
    bool monitorLoaded;
    bool graphLogLoaded;
    bool virtusLoaded;
    bool proxyLoaded;
    bool advancedLoaded;

private slots:
	void onPreferencesSelectChanged(int index);
    void loadStatus(QString msg);
    QString loadKey(QString key, QString dvalue);
    QVector<QPair<QString, QString> > loadKeyByType(QString type); // for proxy setting

    void loadGeneralOptions();
    void loadDefaultProperties();
    void loadConnectionOptions();
    void loadMonitorOptions();
    void loadProxyOptions();
    void saveAllOptions();

    void onMonitorIE(bool checked);
    void onMonitorOpera(bool checked);

    void onNoProxyChecked(bool checked);
    void onCustomProxyChecked();
    void onAddProxy();
    void onModifyProxy();
    void onDeleteProxy();
    void onApplyProxy();
};

#endif // PREFERENCESDIALOG_H
