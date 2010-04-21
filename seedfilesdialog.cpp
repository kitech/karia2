// seedfilesdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-11 16:22:25 +0800
// Version: $Id$
// 


#include "seedfilemodel.h"

#include "seedfilesdialog.h"



/////////////////////////////////
////////////////////////////////

SeedFilesDialog::SeedFilesDialog(QWidget *parent)
    : QDialog(parent)
{
    this->uiwin.setupUi(this);
    
    QObject::connect(this->uiwin.pushButton_2, SIGNAL(clicked()),
                     this, SLOT(accept()));
    QObject::connect(this->uiwin.pushButton_3, SIGNAL(clicked()),
                     this, SLOT(reject()));

    this->itemDelegate = new SeedFileItemDelegate();
    this->uiwin.tableView->setItemDelegate(this->itemDelegate);

    this->seedFileModel = new SeedFileModel();
    this->uiwin.tableView->setModel(this->seedFileModel);

}

SeedFilesDialog::~SeedFilesDialog()
{
}

void SeedFilesDialog::setFiles(QVariantList files, bool selectAll)
{
    this->seedFileModel->setData(files, selectAll);
}

void SeedFilesDialog::setTorrentInfo(QVariantMap statusInfo, QVariantMap torrentInfo)
{
    QString comment;
    QString creationDate;
    QString mode;
    QString name;
    QString totalLength;

    comment = torrentInfo.value("comment").toString();
    creationDate = torrentInfo.value("creationDate").toString();
    mode = torrentInfo.value("mode").toString();
    name = torrentInfo.value("info").toMap().value("name").toString();

    totalLength = statusInfo.value("totalLength").toString();

    this->uiwin.lineEdit->setText(name);
    this->uiwin.label_4->setText(totalLength);
    
    QVariantList trackers = torrentInfo.value("announceList").toList();
    QVariantList d2Trackers;
    QString  tracker;

    QVector<QString> vTrackers;
    for (int i = 0 ; i < trackers.count(); ++i) {
        d2Trackers = trackers.at(i).toList();
        for (int j = 0 ; j < d2Trackers.count(); j++) {
            tracker = d2Trackers.at(j).toString();
            vTrackers.append(tracker);
            this->uiwin.plainTextEdit->appendPlainText(tracker);
        }
    }
    
}

QString SeedFilesDialog::getSelectedFileIndexes()
{
    QString selectedList;

    QModelIndex beginModel = this->seedFileModel->index(0, 1);
    QModelIndexList mil = this->seedFileModel->match(beginModel, Qt::DisplayRole, QString("true"),
                                                     this->seedFileModel->rowCount(QModelIndex()),
                                                     Qt::MatchExactly | Qt::MatchWrap);
    
    QStringList indexList;
    for (int i = 0 ; i < mil.count(); i ++) {
        indexList << this->seedFileModel->data(this->seedFileModel->index(mil.at(i).row(), 0)).toString();
    }

    selectedList = indexList.join(",");

    qDebug()<<"selectedList: "<< selectedList;
    return selectedList;
}

