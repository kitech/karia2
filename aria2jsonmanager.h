// aria2jsonmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-26 22:35:48 +0000
// Version: $Id$
// 
#ifndef _ARIA2JSONRPCMANAGER_H_
#define _ARIA2JSONRPCMANAGER_H_

#include "aria2rpcmanager.h"

/**
 * 负责解析Json格式结果，转换成统一格式返回给显示层
 * 负责组装Json格式请求，发送给后端处理
 */
class Aria2JsonManager : public Aria2RpcManager
{
    Q_OBJECT;
public:
    Aria2JsonManager();
    virtual ~Aria2JsonManager();


};


#endif /* _ARIA2JSONRPCMANAGER_H_ */
