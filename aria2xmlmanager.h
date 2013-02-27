// aria2xmlmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-27 21:55:36 +0000
// Version: $Id$
// 
#ifndef _ARIA2XMLRPCMANAGER_H_
#define _ARIA2XMLRPCMANAGER_H_

#include "aria2rpcmanager.h"

class Aria2XmlManager : public Aria2RpcManager
{
    Q_OBJECT;
public:
    Aria2XmlManager();
    virtual ~Aria2XmlManager();


};

#endif /* _ARIA2XMLRPCMANAGER_H_ */
