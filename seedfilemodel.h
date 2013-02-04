﻿// seedfilemodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-21 20:41:06 +0800
// Version: $Id: seedfilemodel.h 168 2010-09-12 10:14:08Z drswinghead $
// 

#ifndef _SEEDFILEMODEL_H_
#define _SEEDFILEMODEL_H_

#include <QtCore>

namespace ng {
    namespace seedfile {
        enum {index = 0, selected, path, length };
    };
};

class SeedFileModel : public QAbstractTableModel
{
    Q_OBJECT;
public:
    SeedFileModel(QObject *parent = 0);
    ~SeedFileModel();

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

    Qt::ItemFlags flags ( const QModelIndex & index ) const ;
    bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    bool setData(QVariantList &files, bool selectAll);
    bool updateSelectFile(QString selected);

    QString clearReselectFiles();
private:
    QString currentSelectedIndexes();

private:
    QVariantList mFiles;
    char *columnHeaders;
    QMap<int, bool> reselectFiles;
    QString oldSelectedIndexes; // no update if no change, like a snapshot
    bool batchUpdate;

signals:
    void reselectFile(int row, bool selected);
};



#endif /* _SEEDFILEMODEL_H_ */
