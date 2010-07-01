// skypetracer.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-01 19:18:37 +0800
// Version: $Id$
// 
#ifndef _SKYPETRACER_H_
#define _SKYPETRACER_H_

#include <QtCore>
#include <QtGui>

#include "ui_skypetracer.h"

class SkypeTracer : public QDialog
{
    Q_OBJECT;
public:
    SkypeTracer(QWidget *parent = 0);
    virtual ~SkypeTracer();

public slots:
    void onCommandRequest(QString cmd);
    void onCommandResponse(QString cmd);

signals:
    void commandRequest(QString cmd);

private slots:
    void onSendRequest();

private:
    Ui::SkypeTracer uiw;
};

#endif /* _SKYPETRACER_H_ */
