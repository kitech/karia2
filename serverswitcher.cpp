#include "serverswitcher.h"

ServerSwitcher* ServerSwitcher::mHandle = 0 ;

ServerSwitcher * ServerSwitcher::instance()
{
	if( ServerSwitcher::mHandle == 0 )
	{
		ServerSwitcher::mHandle = new ServerSwitcher();
	}
	return ServerSwitcher::mHandle ;
}
ServerSwitcher::ServerSwitcher(QObject *parent)
	: QThread(parent)
{
	//确定是否是我自己的电脑来看是否执行真正的网上搜索。
	QList<QNetworkInterface> ii = QNetworkInterface::allInterfaces () ;
	bool found = false ;
	for( int i = 0 ; i < ii.count() ; i ++ )
	{
		qDebug()<< ii.at(i).name() << ii.at(i).hardwareAddress() ;
		if( ii.at(i).hardwareAddress().compare("00:11:2F:F7:BE:CE") == 0)
		{
			found = true ;
			break ;
		}
	}

	if( found )
	{
		mServer = "http://localhost/nowgo/" ;	//分开处理可以更快。
	}
	else
	{
		mServer = "http://gzl.1500mb.com/nowgo/" ;
	}	
}

ServerSwitcher::~ServerSwitcher()
{

}

void ServerSwitcher::run() 
{
	QString tmp ;
	QUrl url(this->mServer + this->mRequest) ;
	QTcpSocket sock ;
	sock.connectToHost(url.host(),url.port(80));
	
	if( sock.waitForConnected() )
	{
		tmp = "GET /nowgo/%1 HTTP/1.0\r\n"
			"Host: %2\r\n"
			"User-Agent: NullGet\r\n"
			"Connection: close\r\n"
			"\r\n";
		tmp = tmp.arg(this->mRequest).arg(url.host());
		sock.write(tmp.toAscii());
		if( sock.waitForBytesWritten() )
		{
			QByteArray ball ;
			while( sock.waitForReadyRead() )
			{
				if( ball.isEmpty())
					ball = sock.readAll();
				else
					ball += sock.readAll();
			}
			qDebug()<< ball ;
		}
	}
}
//username 可以是未知的，但同样也能做其他的一些信息,可能叫做nullget
void ServerSwitcher::userLogin(QString username)
{	
	if( ! this->isRunning()  )
	{
		this->mRequest = QString("uli.php?n=%1").arg(username);
		this->start(QThread::IdlePriority);
	}
}
void ServerSwitcher::userLogout(QString username)
{	
	if( ! this->isRunning()  )
	{
		this->mRequest = QString("ulo.php?n=%1").arg(username);
		this->start(QThread::IdlePriority);
	}
}
void ServerSwitcher::userOnline(QString username , QString eclapsetime)
{	
	if( ! this->isRunning()  )
	{
		this->mRequest = QString("uol.php?n=%1&es=%2").arg(username).arg(eclapsetime);
		this->start(QThread::IdlePriority);
	}
}
void ServerSwitcher::userGotFile(QString username , QString fname , QString fsize , 
								 QString speed , QString usetime , int from )
{
	if( ! this->isRunning()  )
	{
		this->mRequest = QString("ugf.php?n=%1&fn=%2&fs=%3&s=%4&ut=%5&t=%6").arg(username).arg(fname)
		.arg(fsize).arg(speed).arg(usetime).arg(from);
		this->start(QThread::IdlePriority);
	}
}