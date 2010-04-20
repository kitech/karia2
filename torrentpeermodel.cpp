// torrentpeermodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-07 10:59:15 +0800
// Version: $Id$
// 

#include "torrentpeermodel.h"

TorrentPeerModel::TorrentPeerModel(QObject *parent)
    : QAbstractTableModel(parent)
{

    // Marks the string literal sourceText for dynamic translation in the current context (class),
    this->columnHeaders = QT_TR_NOOP("ip, port, downloadSpeed, uploadSpeed, bitfield, peerId, seeder, amChoking, peerChoking");
}

TorrentPeerModel::~TorrentPeerModel()
{
}

QVariant TorrentPeerModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool TorrentPeerModel::setData(QVariantList &peers)
{
    this->removeRows(0, this->mPeers.count());
    // this->insertRows(0, peers.count());

    this->beginInsertRows(QModelIndex(), 0, peers.count() - 1);
    this->mPeers = peers;
    this->endInsertRows();

    return true;
}

int TorrentPeerModel::columnCount(const QModelIndex &parent) const
{
    return 9;
}

int TorrentPeerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return this->mPeers.count();
    }
}

// TODO add ip -> conutry icon
QVariant TorrentPeerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    int row = index.row();
    int col = index.column();

    QVariantMap peer = this->mPeers.at(row).toMap();
    QStringList columns = QString(this->columnHeaders).split(",");

    QVariant rv;    
    if (col >= 0 && col < 9) {
        rv = peer[columns.at(col).trimmed()];
    }

    return rv;
}

bool TorrentPeerModel::insertRows(int row, int count, const QModelIndex & parent)
{
    this->beginInsertRows(QModelIndex(), row, row + count - 1);
    QVariant vv;
    for (int i = row; i < row + count; i ++) {
        this->mPeers.insert(i, vv);
    }
    this->endInsertRows();
    return true;
}

bool TorrentPeerModel::removeRows(int row, int count, const QModelIndex & parent)
{
    this->beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row + count - 1; i >= row; i --) {
        this->mPeers.removeAt(i);
    }
    this->endRemoveRows();
    return true;
}


////////////////////////////
////////
////////////////////////////

TorrentTrackerModel::TorrentTrackerModel(QObject *parent)
    : QAbstractTableModel(parent)
{

    // Marks the string literal sourceText for dynamic translation in the current context (class),
    this->columnHeaders = QT_TR_NOOP("Id, tracker");
}

TorrentTrackerModel::~TorrentTrackerModel()
{
}

QVariant TorrentTrackerModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool TorrentTrackerModel::setData(QVariantList &trackers)
{
    QVariantHash d1Trackers;
    QVariantList d2Trackers;
    QString  tracker;

    int row = 0;
    for (int i = 0 ; i < trackers.count(); ++i) {
        d2Trackers = trackers.at(i).toList();
        for (int j = 0 ; j < d2Trackers.count(); j++) {
            tracker = d2Trackers.at(j).toString();
            d1Trackers[QString("%1").arg(row++)] = tracker;
        }
    }

    this->removeRows(0, this->mTrackers.count());

    this->beginInsertRows(QModelIndex(), 0, d1Trackers.count() - 1);
    // this->mPeers = peers;
    this->mTrackers = d1Trackers;
    this->endInsertRows();

    return true;
}

int TorrentTrackerModel::columnCount(const QModelIndex &parent) const
{
    return 2;
}

int TorrentTrackerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return this->mTrackers.count();
    }
}

// TODO add ip -> conutry icon
QVariant TorrentTrackerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    int row = index.row();
    int col = index.column();

    // QVariantMap peer = this->mPeers.at(row).toMap();
    
    QStringList columns = QString(this->columnHeaders).split(",");

    QVariant rv;    
    if (col >= 0 && col < 2) {
        if (col == 0) {
            return row;
        } else {
            // rv = peer[columns.at(col).trimmed()];
            rv = this->mTrackers[QString("%1").arg(row)];
        }
    }

    return rv;
}

bool TorrentTrackerModel::insertRows(int row, int count, const QModelIndex & parent)
{
    this->beginInsertRows(QModelIndex(), row, row + count - 1);
    QVariant vv;
    for (int i = row; i < row + count; i ++) {
        // this->mPeers.insert(i, vv);
        this->mTrackers[QString("%1").arg(i)] = vv;
    }
    this->endInsertRows();
    return true;
}

bool TorrentTrackerModel::removeRows(int row, int count, const QModelIndex & parent)
{
    this->beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row + count - 1; i >= row; i --) {
        // this->mPeers.removeAt(i);
        this->mTrackers.remove(QString("%1").arg(i));
    }
    this->endRemoveRows();
    return true;
}
