// taskservermodel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-10 18:15:02 +0800
// Version: $Id$
// 

#ifndef _TASKSERVERMODEL_H_
#define _TASKSERVERMODEL_H_

#include <QtCore>
#include <QtGui>

class TaskServerModel : public QAbstractTableModel
{
    Q_OBJECT;
public:
    TaskServerModel(QObject *parent = 0);
    ~TaskServerModel();

    bool setData(QVariantList &servers);

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant 	headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

private:
    QVariantList mRawServers;
    QList<QMap<char, QString> > mServers; // char is 0, 1, 2, ennn, as int here
    char *columnHeaders;

    // index-n, speed, hostname, url
    // 1-0, 123, abcd,
    // 1-1, 111, efg,
    // 2-0, 123, joisf,
    // 2-1, 21, 32r23,
    // 2-2, idsfsd, owefe,
    // 3-0
};


#endif /* _TASKSERVERMODEL_H_ */
