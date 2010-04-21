// taskservermodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-10 18:16:55 +0800
// Version: $Id$
// 


#include "taskservermodel.h"


TaskServerModel::TaskServerModel(QObject *parent)
    : QAbstractTableModel(parent)
{

    // Marks the string literal sourceText for dynamic translation in the current context (class),
    this->columnHeaders = QT_TR_NOOP("server, downloadSpeed, hostname, url");
}

TaskServerModel::~TaskServerModel()
{
}

QVariant TaskServerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::SizeHintRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    QVariant rv;
    if (orientation == Qt::Horizontal) {
        QString i10nHeaderString = QString(tr(this->columnHeaders));
        QStringList headerList = i10nHeaderString.split(',');
        rv = headerList.at(section);
    }
    return rv;
}

bool TaskServerModel::setData(QVariantList &servers)
{
	// check for servers.count()'s value, if 0, no operation needed
	if (servers.count() == 0) {
		return false;
	}

    this->mRawServers = servers;
    QList<QMap<char, QString> > tServers;
    QMap<char, QString>  tServer;
    QString serverIndex;
    QMap<QString, QVariant> server;
    QListIterator<QVariant> lit(servers);

    int i = 0;
    while(lit.hasNext()) {

        server = lit.next().toMap();
        serverIndex = server[QString("index")].toString();

        int sn = 0;
        QListIterator<QVariant> mit(server["servers"].toList());

        while(mit.hasNext()) {
            QMap<QString, QVariant> nServer = mit.next().toMap();
            tServer[1] = nServer["downloadSpeed"].toString();
            tServer[2] = QUrl(nServer["currentUri"].toString()).host();
            tServer[3] = nServer["currentUri"].toString();
            tServer[0] = QString("%1-%2").arg(serverIndex).arg(++sn);
            
            tServers.append(tServer);
            tServer.clear();
        }
    }

    qDebug()<<tServers<<this->mServers.count();

	if (this->mServers.count() > 0) {
		this->removeRows(0, this->mServers.count());
	}
    
    if (tServers.count() > 0) { 
        // if no this if, than below warning will show.
        // QTreeView::rowsInserted internal representation of the model has been corrupted, resetting. 
        this->beginInsertRows(QModelIndex(), 0, tServers.count());
        this->mServers = tServers;
        this->endInsertRows();
    }

    return true;
}

int TaskServerModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

int TaskServerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return this->mServers.count();
    }
}

QVariant TaskServerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    int row = index.row();
    int col = index.column();

    QMap<char, QString> server = this->mServers.at(row);
    
    QVariant rv;    
    if (col >= 0 && col < 4) {
        rv = server[col];
    }

    return rv;
}

bool TaskServerModel::insertRows(int row, int count, const QModelIndex & parent)
{
    this->beginInsertRows(QModelIndex(), row, row + count - 1);

    QMap<char, QString> vv;
    for (int i = row; i < row + count; i ++) {
        this->mServers.insert(i, vv);
    }
    this->endInsertRows();
    return true;
}

bool TaskServerModel::removeRows(int row, int count, const QModelIndex & parent)
{
	if (count <= 0) {
		return true;
	}
    this->beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row + count - 1; i >= row; i --) {
        this->mServers.removeAt(i);
    }
    this->endRemoveRows();
    return true;
}
