// seedfilesdialog.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-11 16:21:47 +0800
// Version: $Id$
// 

#ifndef _SEEDFILESDIALOG_H_
#define _SEEDFILESDIALOG_H_

#include <QtCore>
#include <QtGui>


#include "ui_seedfilesdialog.h"


class SeedFileModel;
class SeedFileItemDelegate;

class SeedFilesDialog : public QDialog
{
    Q_OBJECT;
public:
    SeedFilesDialog(QWidget *parent = 0);
    ~SeedFilesDialog();
    void setFiles(QVariantList files, bool selectAll);
    void setTorrentInfo(QVariantMap statusInfo, QVariantMap torrentInfo);
    QString getSelectedFileIndexes();

private:
    Ui::SeedFilesDialog uiwin;

    SeedFileModel *seedFileModel;
    SeedFileItemDelegate *itemDelegate;
};


#endif /* _SEEDFILESDIALOG_H_ */
