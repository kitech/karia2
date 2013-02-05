// abstractui.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-05 13:45:47 +0000
// Version: $Id$
// 

#include "karia2.h"
#include "abstractui.h"

AbstractUi::AbstractUi(Karia2 *pwin)
    : QThread(pwin)
{
    this->mui = pwin->mainUI;
    this->mpwin = pwin;
}

AbstractUi::~AbstractUi()
{

}

void AbstractUi::run()
{

}

bool AbstractUi::init()
{
    return true;
}
