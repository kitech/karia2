// aria2manager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-20 22:17:55 +0000
// Version: $Id$
// 

#include "aria2manager.h"

QAtomicInt Aria2Manager::doneCounter = -1;
QAtomicInt Aria2Manager::m_counter = 0;
Aria2Manager::Aria2Manager()
    : QThread(0)
{
}

Aria2Manager::~Aria2Manager()
{
}

void Aria2Manager::run()
{
}

uint64_t Aria2Manager::tid2gid(int tid)
{
    QString ugid = QString("%10000000000000000").arg(tid, 0, 10).left(16);
    uint64_t gid = ugid.toULongLong(NULL, 16);
    return gid;
}

QString  Aria2Manager::tid2hex(int tid)
{
    QString ugid = QString("%10000000000000000").arg(tid, 0, 10).left(16);
    return ugid;
}

