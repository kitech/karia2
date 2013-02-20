// aria2manager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-20 22:17:55 +0000
// Version: $Id$
// 

#include "aria2manager.h"

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

