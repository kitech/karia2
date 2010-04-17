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

class SqliteStorage;

class PreferencesDialog : public QDialog
{
    Q_OBJECT;
public:
    PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

private:
    Ui::PreferencesDialogClass uiwin;
    bool  mIsModified;
    SqliteStorage *storage;

    bool generalLoaded;
    bool defaultPropertiesLoaded;
    bool connectionLoaded;
    bool monitorLoaded;
    bool graphLogLoaded;
    bool virtusLoaded;
    bool advancedLoaded;

private slots:
	void onPreferencesSelectChanged(int index);
    void loadStatus(QString msg);
    QString loadKey(QString key, QString dvalue);
    void loadGeneralOptions();
    void loadDefaultProperties();
    void loadConnectionOptions();
    void saveAllOptions();
};

#endif // PREFERENCESDIALOG_H
