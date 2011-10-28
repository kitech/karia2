// emaria2c.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-10-23 06:54:12 -0700
// Version: $Id$
// 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>

#include <cstring>
#include <ostream>

#include "RequestGroupMan.h"
#include "DownloadEngine.h"
#include "LogFactory.h"
#include "Logger.h"
#include "RequestGroup.h"
#include "prefs.h"
#include "DownloadEngineFactory.h"
#include "RecoverableException.h"
#include "message.h"
#include "util.h"
#include "Option.h"
#include "StatCalc.h"
#include "CookieStorage.h"
#include "File.h"
#include "Netrc.h"
#include "AuthConfigFactory.h"
#include "SessionSerializer.h"
#include "TimeA2.h"
#include "fmt.h"
#include "SocketCore.h"
#include "OutputFile.h"
#ifdef ENABLE_SSL
# include "TLSContext.h"
#endif // ENABLE_SSL


#include "emaria2c.h"

void test_emaria2c()
{
    std::vector<aria2::SharedHandle<aria2::RequestGroup> > requestGroups_;
    aria2::Option *opt;


    aria2::DownloadEngineHandle de = aria2::DownloadEngineFactory().newDownloadEngine(opt, requestGroups_);
}

EAria2Man *EAria2Man::m_instance = NULL;
EAria2Man::EAria2Man(QObject *parent)
    :QObject(parent)
{

}

EAria2Man::~EAria2Man()
{

}

EAria2Man *EAria2Man::instance()
{
    if (EAria2Man::m_instance == NULL) {
        EAria2Man::m_instance = new EAria2Man();
    }
    return EAria2Man::m_instance;
}

/////////////////

EAria2Worker::EAria2Worker(QObject *parent)
    : QThread(parent)
{
}

EAria2Worker::~EAria2Worker()
{

}

void EAria2Worker::run()
{

}

