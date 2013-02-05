// abstractui.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-05 13:38:43 +0000
// Version: $Id$
// 

#ifndef _ABSTRACTUI_H_
#define _ABSTRACTUI_H_

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

namespace Ui {
    class Karia2;
};
class Karia2;

/**
 * UI 元素与控制太多，分治
 */
class AbstractUi : public QThread
{
    Q_OBJECT;
 public:
    explicit AbstractUi(Karia2 *pwin);
    virtual ~AbstractUi();

    virtual void run();

    virtual bool init();

 protected:
    Ui::Karia2 *mui;
    Karia2 *mpwin;
};


#endif /* _ABSTRACTUI_H_ */
