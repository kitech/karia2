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

class SeedFileModel : public QAbstractTableModel
{
    Q_OBJECT;
public:
    SeedFileModel(QObject *parent = 0);
    ~SeedFileModel();

    bool setData(QVariantList &files, bool selectAll);

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

    Qt::ItemFlags flags ( const QModelIndex & index ) const ;
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

private:
    QVariantList mFiles;
    char *columnHeaders;
};

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

private:
    Ui::SeedFilesDialog uiwin;

    SeedFileModel *seedFileModel;
    SeedFileItemDelegate *itemDelegate;
};


#endif /* _SEEDFILESDIALOG_H_ */
