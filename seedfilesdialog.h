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
// class SeedFileItemDelegate;

class SeedFileItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT;
public:
    SeedFileItemDelegate(QObject *parent = 0);
    ~SeedFileItemDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void 	setEditorData ( QWidget * editor, const QModelIndex & index ) const;
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    
};

class SeedFilesDialog : public QDialog
{
    Q_OBJECT;
public:
    SeedFilesDialog(QWidget *parent = 0);
    ~SeedFilesDialog();
    void setFiles(QVariantList files, bool selectAll);
    void setTorrentInfo(QVariantMap statusInfo, QVariantMap torrentInfo);
    QString getSelectedFileIndexes();

private slots:
    void onAutoSelectFiles();
    void onRevertSelectFiles();
    void onSelectAllFiles();
    void onSelectVideoFiles();
    void onSelectAudioFiles();

private:
    Ui::SeedFilesDialog uiwin;

    SeedFileModel *seedFileModel;
    SeedFileItemDelegate *itemDelegate;
};


#endif /* _SEEDFILESDIALOG_H_ */
