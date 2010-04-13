#ifndef SERVERSWITCHER_H
#define SERVERSWITCHER_H

#include <QtCore>
#include <QtNetwork>
#include <QThread>
#include <openssl/ssl.h>

/**
 * 与服务器通信，交换信息的数据。
 * 负载将一些信息如记分或者下载的文件号之类的传送到服务器。
 */
class ServerSwitcher : public QThread
{
	Q_OBJECT

public:
    static ServerSwitcher* instance();
	
    ~ServerSwitcher();
	
	void run() ;
	//
	enum { USER_LOGIN , //用户的一次登陆操作。可能有空上用户的记录，可能没有。
		USER_LOGOUT,	//用户退出。
		USER_ONLINE ,	//用户的在线时间，这是每一时间段向服务器发送一次，以便更准确的记录用户的在线。
		USER_GOT_FILE_FROM_NET,	//用户下载了一个文件。从WWW上
		USER_GOT_FILE_FROM_SKYPE	//用户从好友的共享下载了一个文件。
		} ;

public slots:
	void userLogin(QString username);
	void userLogout(QString username);
	void userOnline(QString username , QString eclapsetime);
	void userGotFile(QString username , QString fname , QString fsize , QString speed , QString usetime , int from );

private:
	ServerSwitcher(QObject *parent = 0 );
	static ServerSwitcher * mHandle ;

	//假设它的格式为：http://mServer/nowgo/....
	QString mServer ;	//一个IP地址或者域名或者一段
	QStringList mRunList ;	//可能一次会给其一个运行队列
	
	QTimer mRunTimer ;		//用这个来调度运行队列。
	QString mRawData ;
	QString mRequest ;	//


signals:

};

#endif // SERVERSWITCHER_H
