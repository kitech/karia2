// aria2managerfactory.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-27 22:36:36 +0000
// Version: $Id$
// 

#include "aria2manager.h"
#include "aria2embedmanager.h"
#include "aria2libaria2manager.h"
#include "aria2xmlmanager.h"
#include "aria2jsonmanager.h"
#include "aria2wsjsonmanager.h"
#include "aria2managerfactory.h"

Aria2ManagerFactory::Aria2ManagerFactory()
{
}

Aria2ManagerFactory::~Aria2ManagerFactory()
{
}

QMutex Aria2ManagerFactory::m_inst_mutex;
QMap<int, Aria2Manager*> Aria2ManagerFactory::mManagers;
Aria2Manager* Aria2ManagerFactory::createManager(int type)
{
    Aria2Manager* manager = NULL;

    Aria2ManagerFactory::m_inst_mutex.lock();
    if (!Aria2ManagerFactory::mManagers.contains(type)) {
        switch(type) {
        case Aria2Manager::BT_EMBEDED:
            manager = new Aria2EmbedManager();
            Aria2ManagerFactory::mManagers[type] = manager;
            break;
        case Aria2Manager::BT_LIBARIA2:
            manager = new Aria2Libaria2Manager();
            Aria2ManagerFactory::mManagers[type] = manager;
            break;
        case Aria2Manager::BT_XMLRPC_HTTP:
            manager = new Aria2XmlManager();
            Aria2ManagerFactory::mManagers[type] = manager;
            break;
        case Aria2Manager::BT_JSONRPC_HTTP:
            manager = new Aria2JsonManager();
            Aria2ManagerFactory::mManagers[type] = manager;
            break;
        case Aria2Manager::BT_JSONRPC_WS:
            manager = new Aria2WSJsonManager();
            Aria2ManagerFactory::mManagers[type] = manager;
            break;
        default:
            break;
        }
    }

    manager = Aria2ManagerFactory::mManagers.value(type);
    Aria2ManagerFactory::m_inst_mutex.unlock();

    return manager;
}
