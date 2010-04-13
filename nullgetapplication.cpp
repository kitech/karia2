#include <QtCore>

#ifdef Q_OS_WIN32
#include <windows.h>
#else
#endif

#include "nullgetapplication.h"

NullGetApplication::NullGetApplication(int & argc, char ** argv)
	: QApplication(argc,argv)
{

}

NullGetApplication::~NullGetApplication()
{

}

#ifdef Q_OS_WIN32
bool NullGetApplication::winEventFilter ( MSG * msg, long * result )
{
	//qDebug()<<__FUNCTION__<<__LINE__<<rand();

	//qDebug()<<msg->message ;

	return QApplication::winEventFilter(msg,result);

}
#else
#ifdef Q_OS_MAC
bool NullGetApplication::macEventFilter(EventHandlerCallRef caller, EventRef event )
{
    return QApplication::macEventFilter(caller, event);
}
#else
bool NullGetApplication::x11EventFilter ( XEvent * event )
{
	return QApplication::x11EventFilter(event);
}

#endif
#endif
