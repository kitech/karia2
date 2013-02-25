// aria2embedmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-25 21:55:29 +0000
// Version: $Id$
// 
#ifndef _ARIA2EMBEDMANAGER_H_
#define _ARIA2EMBEDMANAGER_H_

#include <QtCore>

#include "aria2manager.h"

class Aria2EmbedManager : public Aria2Manager
{
    Q_OBJECT;
public:
    Aria2EmbedManager();
    virtual ~Aria2EmbedManager();

protected:

};

#endif /* _ARIA2EMBEDMANAGER_H_ */
