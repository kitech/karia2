// taskitemdelegate.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-22 22:42:35 +0800
// Version: $Id: taskitemdelegate.h 198 2013-02-16 03:59:01Z drswinghead $
// 

#ifndef _TASKITEMDELEGATE_H_
#define _TASKITEMDELEGATE_H_

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

class TaskItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT;
public:
    TaskItemDelegate(QObject *parent = 0);
    ~TaskItemDelegate();

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void 	setEditorData ( QWidget * editor, const QModelIndex & index ) const;
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    
    // QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index);
};


#endif /* _TASKITEMDELEGATE_H_ */
