// taskitemdelegate.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-22 22:48:02 +0800
// Version: $Id$
// 

#include "sqlitestorage.h"

#include "taskitemdelegate.h"


TaskItemDelegate::TaskItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    
}

TaskItemDelegate::~TaskItemDelegate()
{
}
QWidget *TaskItemDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    qDebug()<<parent<<option<<index;
    if (index.column() == 1) {
        QCheckBox *cb = new QCheckBox(parent);
        return cb;
    } else {
        return NULL;
    }
}

void TaskItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index ) const
{
    if (index.column() == 1) {
        // QCheckBox *cb = static_cast<QCheckBox*>(editor);
        // cb->setChecked( index.data().toString() == "true" ? true : false);
    }
}

void TaskItemDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    // ng::tasks::abtained_percent
    if (index.column() == ng::tasks::abtained_percent) {
        double percent = index.data().toString().left(3).trimmed().toDouble();

        // QStyleOptionButton checkBoxOption;
        // checkBoxOption.rect = option.rect;
        // checkBoxOption.state = (seleted == "true"  ? QStyle::State_On : QStyle::State_Off)
        //     | QStyle::State_Enabled | QStyle::State_Editing 
        //     ;
        // checkBoxOption.text = seleted;

        // QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkBoxOption, painter);

        QStyleOptionProgressBarV2 progressBarOption;
        progressBarOption.rect = option.rect;
        progressBarOption.maximum = 100;
        progressBarOption.minimum = 0;
        progressBarOption.progress = (int)percent;
        progressBarOption.text = QString("%1").arg(index.data().toString());
        progressBarOption.textVisible = true;
        progressBarOption.state = QStyle::State_Enabled;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

// If you reimplement this you must also reimplement paint().
// QSize TaskItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index)
// {
//     QSize size;

//     size.setHeight(50);
    
//     return size;
// }

