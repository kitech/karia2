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
    NullGetApplication a(argc, argv);
    a.addLibraryPath(a.applicationDirPath() + "/plugins");

	a.setQuitOnLastWindowClosed(false);

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

	//w.initialMainWindow() ;

    //a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));	//这就是问题所在。2006-9-14
    return a.exec();
}
