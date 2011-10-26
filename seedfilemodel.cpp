// seedfilemodel.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-21 20:42:02 +0800
// Version: $Id$
// 

#include "utility.h"
#include "seedfilemodel.h"

SeedFileModel::SeedFileModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    this->batchUpdate = false;
    // Marks the string literal sourceText for dynamic translation in the current context (class),
    this->columnHeaders = QT_TR_NOOP("index, selected, path, length");
}

SeedFileModel::~SeedFileModel()
{
}

QVariant SeedFileModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool SeedFileModel::setData(QVariantList &files, bool selectAll)
{
    if (selectAll == true) {
        QMap<QString, QVariant> file;
        for (int i = 0; i < files.count(); i ++) {
            file = files.at(i).toMap();
            file.insert(QString("selected"), QString("true"));
            files.replace(i, file);
        }
    } else {
        
    }
    this->removeRows(0, this->mFiles.count());
 
    this->beginInsertRows(QModelIndex(), 0, files.count() - 1);
    this->mFiles = files;
    this->endInsertRows();

    this->oldSelectedIndexes = this->currentSelectedIndexes();

    return true;
}

bool SeedFileModel::updateSelectFile(QString selected)
{
    QStringList indexes = selected.split(",");

    this->batchUpdate = true;
    int rc = this->rowCount(QModelIndex());
    QVariantMap file;
    for (int i = rc - 1; i >= 0; --i) {
        file = this->mFiles.at(i).toMap();
        if (indexes.contains(file.value("index").toString())) {
            this->setData(this->index(i, ng::seedfile::selected), QVariant(true));
        } else {
            this->setData(this->index(i, ng::seedfile::selected), QVariant(false));
        }
    }
    this->batchUpdate = false;
    this->oldSelectedIndexes = this->currentSelectedIndexes();

    return true;
}

int SeedFileModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

int SeedFileModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return this->mFiles.count();
    }
}

// TODO add ip -> conutry icon
QVariant SeedFileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    int row = index.row();
    int col = index.column();

    QVariantMap file = this->mFiles.at(row).toMap();
    QStringList columns = QString(this->columnHeaders).split(",");

    QVariant rv;
    if (col >= 0 && col < 9) {
        rv = file[columns.at(col).trimmed()];
    }

    return rv;
}

bool SeedFileModel::insertRows(int row, int count, const QModelIndex & parent)
{
    this->beginInsertRows(QModelIndex(), row, row + count - 1);
    QVariant vv;
    for (int i = row; i < row + count; i ++) {
        this->mFiles.insert(i, vv);
    }
    this->endInsertRows();

    this->oldSelectedIndexes = this->currentSelectedIndexes();

    return true;
}

bool SeedFileModel::removeRows(int row, int count, const QModelIndex & parent)
{
    this->beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row + count - 1; i >= row; i --) {
        this->mFiles.removeAt(i);
    }
    this->endRemoveRows();

    this->oldSelectedIndexes = this->currentSelectedIndexes();

    return true;
}

bool SeedFileModel::setData ( const QModelIndex & index, const QVariant & value, int role)
{
    // q_debug()<<__FUNCTION__<<index<<value;
    if (index.column() == ng::seedfile::selected) {
        QMap<QString, QVariant> file = this->mFiles.at(index.row()).toMap();
        file.insert(QString("selected"), (value.toBool()) ? "true" : "false");
        this->mFiles.replace(index.row(), file);
        emit dataChanged(index, index);

        if (!this->batchUpdate) {
            this->reselectFiles[index.row()] = value.toBool();
            emit reselectFile(index.row(), value.toBool());
        }
        return true;
    } else {
        return false;
    }
}

Qt::ItemFlags SeedFileModel::flags(const QModelIndex & index) const 
{
    if (index.column() == 1) {
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    } else {
        return QAbstractTableModel::flags(index);
    }
}

QString SeedFileModel::clearReselectFiles()
{
    QString reselected;
    QStringList indexes;

    if (this->reselectFiles.count() == 0) {
    } else {
        QMap<QString, QVariant> file;
        for (int i = 0; i < this->mFiles.count(); i ++) {
            file = this->mFiles.at(i).toMap();
            // file.insert(QString("selected"), QString("true"));
            // files.replace(i, file);
            if (file.value("selected") == QString("true")) {
                indexes << QString("%1").arg(i);
            }
        }

        if (indexes.count() > 0) {
            reselected = indexes.join(",");
        }

        // no change, will return no change
        if (reselected == oldSelectedIndexes) {
            reselected = QString();
        }

        this->reselectFiles.clear();
    }
    return reselected;
}

QString SeedFileModel::currentSelectedIndexes()
{
    QString selected;
    QStringList indexes;
    QMap<QString, QVariant> file;

    for (int i = 0; i < this->mFiles.count(); i ++) {
        file = this->mFiles.at(i).toMap();
        if (file.value("selected") == QString("true")) {
            indexes << QString("%1").arg(i);
        }
    }
    if (indexes.count() > 0) {
        selected = indexes.join(",");
    }

    return selected;
}

///////////////////////////////
/////////////////////////////////////
