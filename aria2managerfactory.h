// aria2managerfactory.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-27 22:34:28 +0000
// Version: $Id$
// 

#ifndef _ARIA2MANAGERFACTORY_H_
#define _ARIA2MANAGERFACTORY_H_

#include <QMap>

class Aria2Manager;

class Aria2ManagerFactory
{
public:
    explicit Aria2ManagerFactory();
    virtual ~Aria2ManagerFactory();

    static Aria2Manager *createManager(int type);

private:
    static QMap<int, Aria2Manager*> mManagers;
    static QMutex m_inst_mutex;
};


#endif /* _ARIA2MANAGERFACTORY_H_ */
