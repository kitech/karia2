#include <QtCore>
#include <QtGui>
#include <QApplication>

#include <QTranslator>
#include <QLocale>

#include "nullgetapplication.h"
#include "nullget.h"

int main(int argc, char *argv[])
{
	//Q_INIT_RESOURCE(styles);
    NullGetApplication app(argc, argv);
    app.addLibraryPath(app.applicationDirPath() + "/plugins");

	app.setQuitOnLastWindowClosed(false);

    if (app.isRunning()) {
        qDebug()<<"Another instance of karia2 is running.";
        QString msg("hi master, are you ok?");
        bool sendok = app.sendMessage(msg);
        if (sendok) {
            
        } else {

        }
        return 0;
    }

    NullGet w;

	//这种方法只能在启动的时候用，其他时候更改无效。
	//QTranslator translator;
	//translator.load("nullget_zh_CN");
	//a.installTranslator(&translator);
	//在这里使用与动态切换怎么是冲突的呢。
	//QTranslator qtTranslatorInit;
	//qtTranslatorInit.load("qt_" + QLocale::system().name());
	//a.installTranslator(&qtTranslatorInit);	//load the qt translator file

	//QString locale = QLocale::system().name();
	//qDebug()<<"Switch Langague to: "<<locale;
	//QTranslator appTranslatorInit;
	//appTranslatorInit.load(QString("nullget_") + locale);
	//////translator.load(QString("nullget_") + "en_US");
	//a.installTranslator(&appTranslatorInit);

    //w.show();	
	//w.showMinimized();	
	//w.close();
	//w.showMinimized();	
	//w.showNormal();
	
	//w.setFocus();
	//w.raise();
	//w.activateWindow();
	w.show();

    QObject::connect(&app, SIGNAL(messageReceived(const QString&)), 
                     &app, SLOT(handleMessage(const QString &)));

	//w.initialMainWindow() ;

    //app.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));	//这就是问题所在。2006-9-14
    return app.exec();
}
