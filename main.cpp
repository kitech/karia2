﻿// main.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-18 12:50:15 +0800
// Version: $Id: main.cpp 227 2013-10-20 04:30:51Z drswinghead $
// 


#include <QtCore>
#include <QtGui>
#include <QApplication>

#include <QTranslator>
#include <QLocale>

#include "simplelog.h"

#include "karia2application.h"
#include "karia2.h"

QString packArguments(QCoreApplication *app, int argc, char **argv)
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    int cargc = 0;
    QString cmdLine;
    QStringList args;

    args = app->arguments();
#ifdef Q_OS_WIN
    // the string given by arguments().at(0) might not be the program name on Windows, 
    // depending on how the application was started.
    // args.prepend(app->applicationFilePath());
    if (args.count() == 0) {
        args.append(app->applicationFilePath());
    } else if (!args.at(0).endsWith("karia2.exe", Qt::CaseInsensitive)) {
        args.prepend(app->applicationFilePath());
    } else {
        // ok, get argument's mechinism the same as *nix, go on 
    }
#endif

    cargc = args.count();
    
    cmdLine = args.join(" ");
    
    qLogx()<<cmdLine;
    // qDebug()<<__FUNCTION__<<cmdLine;
    return cmdLine;
}
// unpack is not need, because getopt4 support QStringList format cmdLine.
// int unpackArguments(QString cmdLine, char **argv);

int main(int argc, char *argv[])
{
	//Q_INIT_RESOURCE(styles);
    setenv("QT_MESSAGE_PATTERN", "[%{type}] %{appname} (%{file}:%{line}) - %{message} ", 1);
    Karia2Application app(argc, argv);
    app.addLibraryPath(app.applicationDirPath() + "/plugins");

	app.setQuitOnLastWindowClosed(false);

    if (app.isRunning()) {
        qLogx()<<"Another instance of karia2 is running.";
        QString msg("sayhello: hi master, are you ok?");
        bool sendok = app.sendMessage(msg);
        msg = "cmdline:" + packArguments(&app, argc, argv);
        sendok = app.sendMessage(msg);

        if (sendok) {
            
        } else {

        }
        return 0;
    }

    Karia2 win(argc, argv);

	//这种方法只能在启动的时候用，其他时候更改无效。
	//QTranslator translator;
	//translator.load("karia2_zh_CN");
	//a.installTranslator(&translator);
	//在这里使用与动态切换怎么是冲突的呢。
	//QTranslator qtTranslatorInit;
	//qtTranslatorInit.load("qt_" + QLocale::system().name());
	//a.installTranslator(&qtTranslatorInit);	//load the qt translator file

	//QString locale = QLocale::system().name();
	//qDebug()<<"Switch Langague to: "<<locale;
	//QTranslator appTranslatorInit;
	//appTranslatorInit.load(QString("karia2_") + locale);
	//////translator.load(QString("karia2_") + "en_US");
	//a.installTranslator(&appTranslatorInit);

    //w.show();	
	//w.showMinimized();	
	//w.close();
	//w.showMinimized();	
	//w.showNormal();
	
	//w.setFocus();
	//w.raise();
	//w.activateWindow();
	win.show();

    QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
                     &win, SLOT(onOtherKaria2MessageRecived(const QString&)));
    QObject::connect(&app, SIGNAL(messageReceived(const QString&)), 
                     &app, SLOT(handleMessage(const QString &)));

	//w.initialMainWindow() ;

    //app.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));	//这就是问题所在。2006-9-14
    return app.exec();
}
