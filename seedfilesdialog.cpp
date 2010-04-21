// seedfilesdialog.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-11 16:22:25 +0800
// Version: $Id$
// 

#include "seedfilesdialog.h"


SeedFileModel::SeedFileModel(QObject *parent)
    : QAbstractTableModel(parent)
{

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
    return true;
}

bool SeedFileModel::removeRows(int row, int count, const QModelIndex & parent)
{
    this->beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row + count - 1; i >= row; i --) {
        this->mFiles.removeAt(i);
    }
    this->endRemoveRows();
    return true;
}

bool SeedFileModel::setData ( const QModelIndex & index, const QVariant & value, int role)
{

    if (index.column() == 1) {
        QMap<QString, QVariant> file = this->mFiles.at(index.row()).toMap();
        file.insert(QString("selected"), (value.toBool()) ? "true" : "false");
        this->mFiles.replace(index.row(), file);
        emit dataChanged(index, index);
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

///////////////////////////////
/////////////////////////////////////

SeedFileItemDelegate::SeedFileItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    
}

SeedFileItemDelegate::~SeedFileItemDelegate()
{
}
QWidget *SeedFileItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    qDebug()<<parent<<option<<index;
    if (index.column() == 1) {
        QCheckBox *cb = new QCheckBox(parent);
        return cb;
    } else {
        return NULL;
    }
}

void SeedFileItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index ) const
{
    if (index.column() == 1) {
        QCheckBox *cb = static_cast<QCheckBox*>(editor);
        cb->setChecked( index.data().toString() == "true" ? true : false);
    }
}

void SeedFileItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    if (index.column() == 1) {
        QString seleted = index.data().toString();

        QStyleOptionButton checkBoxOption;
        checkBoxOption.rect = option.rect;
        checkBoxOption.state = (seleted == "true"  ? QStyle::State_On : QStyle::State_Off)
            | QStyle::State_Enabled | QStyle::State_Editing 
            ;
        checkBoxOption.text = seleted;

        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

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

