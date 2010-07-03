// main.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-07-03 15:24:47 +0800
// Version: $Id$
// 


#include <QtGui/QApplication>
#include "skyserv.h"
#include "skyservapplication.h"

int main(int argc, char *argv[])
{
    SkyServApplication a(argc, argv);
    SkyServ w;
    // w.show();

    return a.exec();
}
