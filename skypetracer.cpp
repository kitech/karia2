// skypetracer.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-01 19:18:54 +0800
// Version: $Id$
// 

#include "skypetracer.h"

SkypeTracer::SkypeTracer(QWidget *parent)
    : QDialog(parent)
{
    this->uiw.setupUi(this);

    QObject::connect(this->uiw.pushButton_8, SIGNAL(clicked()),
                     this, SLOT(onSendRequest()));
}

SkypeTracer::~SkypeTracer()
{
}

void SkypeTracer::onCommandRequest(QString cmd)
{
    qDebug()<<__FILE__<<__LINE__<<cmd;
    this->uiw.textBrowser->append("<P><FONT COLOR=blue>" + cmd.trimmed() + "</FONT></P>");
}

void SkypeTracer::onCommandResponse(QString cmd)
{
    qDebug()<<__FILE__<<__LINE__<<cmd;
    this->uiw.textBrowser->append("<P> -) " + cmd.trimmed() + "</P>");
}

void SkypeTracer::onSendRequest()
{
    QString cmd = this->uiw.comboBox->currentText();
    emit commandRequest(cmd);
}
