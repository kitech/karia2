#include <QtCore>
#include <QtGui>
#include <QAction>
#include <QToolBar>
#include <QTreeView>
#include <QGraphicsView>
#include <QMainWindow>
#include <QMenu>
#include <QActionGroup>
#include <QStatusBar>
#include <QMessageBox>

#include <QDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QUrlInfo>
#include <QFileInfo>
#include <QList>
#include <QDataStream>
#include <QFile>
#include <QMenu>
#include <QRect>
#include <QSize>
#include <QProcess>
#include <QDockWidget>
#include <QHash>
#include <QToolTip>
#include <QTcpSocket>
#include <QDesktopServices>

//////
#include "libng/qtmd5.h"
#include "libng/html-parse.h"

#include "radarscanner.h"

#include "walksitewndex.h"

////////////////////////////////////
////////////////////////////////////
//所有线程列表。
QVector<WalkSiteThread *> WalkSiteThread::mAllThreadList ;

//the Char 是URL的状态 , R ready , E rror , D: done , C crawling
QHash<QString , char>  WalkSiteThread::mUrlHashList;
// char is A : accept  R : reject , 如果这里面没有的默认就是允许的了。
QHash<QString , char> WalkSiteThread::mExtHashList ;
//char is A : accept , R : reject 
QHash<QString , char > WalkSiteThread::mHostHashList ; 
QMutex WalkSiteThread::mUrlAccessMutex ;
QMutex WalkSiteThread::mExtAccessMutex ;
QMutex WalkSiteThread::mHostAccessMutex ;

// 200 nn 300 nn 400 nn 
QHash< int , quint64> WalkSiteThread::mRemoteResponseCount ;

///////////////////////
bool WalkSiteThread::mFollowFtp = true;	// = true ;
bool WalkSiteThread::mSpanHost = true ;		// = true ;
bool WalkSiteThread::mCmpTimeStamp = true ;	// = true ;
int	WalkSiteThread::mMaxRecursiveDepth = 5 ;	// = 5 
int	WalkSiteThread::mMaxCrawlSize = 1024 * 1024 *1024 ;		// = 1 G = 1024 * 1024 * 1024
int	WalkSiteThread::mMaxConnection = 20  ;	// = 20	相当于20个线程。
int WalkSiteThread::mMaxCrawlSpeed = 0 ;	//	最大限制速度。
QString	WalkSiteThread::mStartUrl = "" ;	//	= url set 

	//已经下载了的数据总大小。
quint64 WalkSiteThread::mCrawledSize = 0 ;
	//下载了的文件数
quint64 WalkSiteThread::mCrawledPageCount = 0  ;
	//下载文件保存根目录。
QString WalkSiteThread::mSavePath = "." ;	

	//当前速度
int WalkSiteThread::mCurrSpeed = 0 ;

////////////////////////////////////
////////////////////////////////////
WalkSiteThread::WalkSiteThread(QObject * parent  )
	:QThread(parent)
{
	//this->mCtrlSock = new QTcpSocket() ;
	this->mCtrlSock = 0 ;
	this->mDataSock = new QTcpSocket() ;
	this->isPaused = false ;
}
WalkSiteThread::~WalkSiteThread() 
{
	if( this->mCtrlSock != 0 )
	{
		delete this->mCtrlSock ;
		this->mCtrlSock = 0;
	}
	if( this->mDataSock != 0 )
	{
		delete this->mDataSock ;
		this->mDataSock = 0 ;
	}

}
void WalkSiteThread::safeDestroyLater()
{
	this->deleteLater();
}
void WalkSiteThread::run()
{
	qDebug()<<__FUNCTION__;
	
	while(  ! this->isPaused )
	{			
		if( getNextUrl() .isNull( ) || this->mCurrUrl.trimmed().length()<=0 ) break  ;		
		this->mCtrlSock = new QTcpSocket();
		crawlUrl()  ;
		this->usleep(200);
		delete this->mCtrlSock ;
		this->mCtrlSock = 0 ;

		this->mUrlAccessMutex.lock();	//取出来下一个URL
		//this->mUrlHashList[this->mCurrUrl] = 'D' ;
		this->mUrlHashList.remove(this->mCurrUrl) ;
		this->mUrlHashList[ qtMD5(this->mCurrUrl.toAscii())] = 'D' ;			
		this->mUrlAccessMutex.unlock();
	}
	//
	if( this->isPaused )
	{
		//qDebug()<<"deleteLater of this object";
		//this->deleteLater();
	}
	qDebug()<<"Leave :"<<__FUNCTION__ ;

}
void WalkSiteThread::crawlUrl() 
{	
	int	port = 80 ;
	int waits = 3000; //等待时间。

	QUrl uu( this->mCurrUrl );
	if( uu.scheme().toUpper().compare("HTTP")==0 ||uu.scheme().toUpper().compare("HTTPS")==0
		||uu.scheme().toUpper().compare("FTP") == 0 )
	{
			if( uu.scheme().toUpper().compare("HTTP") == 0 )
			{
				port = uu.port(80);
			}
			else if( uu.scheme().toUpper().compare("HTTPS") == 0 )
			{
				port = uu.port(443);
			}
			else if( uu.scheme().toUpper().compare("FTP") == 0 )
			{
				port = uu.port(21);
			}
			else
			{
				//error scheme
				qDebug()<<"not supported scheme" << this->mCurrUrl ;
				
				return ;
			}
	}
	else
	{
		qDebug()<<"Not supported Scheme"<< this->mCurrUrl <<__LINE__ ;
		return ;
	}

	/////////////
	//this->mCtrlSock = new QTcpSocket();
	this->mCtrlSock->connectToHost( uu.host() , port , QIODevice::ReadWrite);
	if( ! this->mCtrlSock->waitForConnected(waits) )
	{
		qDebug()<<uu.host()<<port<<this->mCtrlSock->errorString() ;
		return ;
	}
	//////////////
	QHttpRequestHeader hrh ;

	hrh.setValue("Host",uu.host() ) ;
	hrh.setValue("Range", QString("bytes=%1-").arg(0) ) ;
	hrh.setValue("Accept","*/*");
	hrh.setValue("User-Agent","Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)");
	hrh.setValue("Pragma","no-cache");
	hrh.setValue("Cache-Control","no-cache");
	hrh.setValue("Referer","");
	if( uu.encodedQuery().length()>0)
		hrh.setRequest("GET",uu.path()+"?"+uu.encodedQuery().data());
	else
			hrh.setRequest("GET",uu.path());
	hrh.setValue("Connection", "close" ) ;

	this->mCtrlSock->write(hrh.toString().toAscii());
	if( ! this->mCtrlSock->waitForBytesWritten(waits) )
	{
		return ;
	}
	/////////
	
	QString strBuff;
	char chBuff[8096] = {0};
	int readyBytes = 0 ;
	int oldReadBuffSize = 0 ;
	QHttpResponseHeader hsh ;
	
	//	
	if(! this->mCtrlSock->waitForReadyRead(waits ) )
	{
		return ;
	}
	//set read buffer size to 1 , then we can control the speed
	oldReadBuffSize = this->mCtrlSock->readBufferSize();
	this->mCtrlSock->setReadBufferSize(1);
	while( this->mCtrlSock->readLine(chBuff , sizeof(chBuff) - 1  ) > 0 )
	{
		//readLine is impl by getchar,so no problem here
		strBuff += QString(chBuff);
		//qDebug()<<rb;
		if( strBuff.endsWith(QString("\r\n\r\n")))
		{
			// qDebug()<<"read head end "<< strBuff ;				
			break;
		}
	}
	this->mCtrlSock->setReadBufferSize(oldReadBuffSize);	// reset the buff size to default value 
	hsh = strBuff ;
	if( hsh.statusCode() < 200 || hsh.statusCode() >= 400 )
	{

		return ;
	}
	if( hsh.statusCode() >= 300 && hsh.statusCode() < 400 )
	{
		//作为一个新的URL
		return ;
	}

	//打开文件
	//取得文件要存储的路径，如果路径不存在则创建之，并创建该文件句柄
	QString path , file ;
	QFileInfo fi(uu.path());
	path = QString("%1/%2/%3/").arg(this->mSavePath).arg(uu.host()).arg(fi.path());
	
	QDir dir(this->mSavePath);
	dir.setPath(path);
	dir.makeAbsolute();
	path = dir.path () ;
	qDebug()<<"mkdir for create file if need : "<< path << dir.path () ;
	if( dir.exists(path) )
	{
	}
	else
	{				
		if( ! dir.mkpath(path) )
		{
			qDebug()<< "mkdir error:"<< path ;
		}
	}

	//QFileInfo fi(curl);
	if( fi.fileName().isEmpty() || fi.fileName().isNull() )
	{
		file = path + QString("/index.html");
	}
	else
	{
		file = path +"/" + fi.fileName();
	}

	QFile hhFile (file);
	hhFile.open(QIODevice::WriteOnly );
	qDebug()<<"Open file for write"<<file ;

	//读取数据
	strBuff.clear() ;
	readyBytes = this->mCtrlSock->bytesAvailable();
	if( readyBytes > 0 )
	{
		//readyBytes = this->mCtrlSock->bytesAvailable();
		int left = readyBytes ;
		int nret = 0 ;
		while( left > 0 )
		{
			memset(chBuff,0,sizeof(chBuff));
			nret = this->mCtrlSock->read(chBuff,sizeof(chBuff)-1);
			if( nret <= 0 ) break ;
			//qDebug()<<chBuff ;
			left -= nret ;
			hhFile.write(chBuff,nret);
			strBuff += QString(chBuff) ;
		}
	}
	while( this->mCtrlSock->waitForReadyRead(waits) )	
	{
		readyBytes = this->mCtrlSock->bytesAvailable();
		int left = readyBytes ;
		int nret = 0 ;
		while( left > 0 )
		{
			memset(chBuff,0,sizeof(chBuff));
			nret = this->mCtrlSock->read(chBuff,sizeof(chBuff)-1);
			if( nret <= 0 ) break ;
			//qDebug()<<chBuff ;
			left -= nret ;
			hhFile.write(chBuff,nret);
			strBuff += QString(chBuff) ;
		}
	}
	this->mCtrlSock->close();
	this->mCtrlSock->disconnectFromHost();
	this->mCtrlSock->waitForDisconnected();
	this->mCtrlSock->abort();

	if( hhFile.isOpen() )
	{
		hhFile.flush();
		hhFile.close();
	}

	if( ! strBuff.isNull() && strBuff.length() > 0 )
	{
		//解析页面中的URL
		if( this->isNeedParsedUrl(this->mCurrUrl) )
			this->parseWebPage(this->mCurrUrl , strBuff);
		strBuff.clear();
	}

funend:
	
	return ;
}
QString WalkSiteThread::getNextUrl()
{	
	this->mCurrUrl = "" ;
	
	this->mUrlAccessMutex.lock();	//取出来下一个URL
	if( this->mUrlHashList.empty () )
	{
		this->mCurrUrl  = QString(QString::null);
	}
	else
	{
		QHash<QString , char>::iterator hit = this->mUrlHashList.begin();
		while( hit != this->mUrlHashList.end() )
		{
			if( hit.value() == 'R')
			{
				this->mUrlHashList[hit.key()] = 'C' ;
				this->mCurrUrl = hit.key() ;
				break ;
			}
			else
			{

			}
			hit ++ ;
		}
	}
	this->mUrlAccessMutex.unlock();

	return this->mCurrUrl  ;
}

////////
void WalkSiteThread::parseWebPage( QString & currUrl , QString & htmlcode)
{
	qDebug()<<__FUNCTION__;
	int linkCount = 0;
	char ** linkList = 0 ;

	QUrl uu(currUrl);
	QFileInfo fi(currUrl);
	QString path = uu.path();
	QUrl tmpUrl ;
	char md5val[36] = {0};
	QString md5str ;
	
	//QString absUrl = QString("%1://%2:%3/%4/").arg(uu.scheme()).arg(uu.host()).arg(uu.port(80)).arg(uu.path()) ;

	linkCount = 0;
	linkList = html_parse_get_all_link(htmlcode.toAscii().data() , &linkCount );
	qDebug()<<linkCount ;
	QHash<QString, char>::const_iterator hit ; 
	for( int j = 0 ; j < linkCount ; j ++ )
	{
		//srcList.append(QString(linkList[j]));
		//首先需要对取出来的URL进行判断，重写成绝对URL
		QUrl relative(linkList[j]);
		tmpUrl = uu.resolved(relative)  ;
		if( tmpUrl.hasFragment() )
		{
			//去掉链接上的fragment.
			tmpUrl = QUrl(tmpUrl.toString().left(tmpUrl.toString().length() - tmpUrl.fragment().length() -1) ) ;
		}
		this->mUrlAccessMutex.lock();	//取出来下一个URL
		//::MD5Encode(tmpUrl.toString().toAscii().data(),md5val)  ;
		
		//qDebug()<< "Caller str: "<< md5str << __FILE__<<__LINE__ ;
		hit = this->mUrlHashList.find( ::qtMD5(tmpUrl.toString().toAscii()) );
		if( hit != this->mUrlHashList.end() )
		{
			this->mUrlAccessMutex.unlock();	//取出来下一个URL
			continue;
		}
		this->mUrlAccessMutex.unlock();	//取出来下一个URL

		//this->putLog( tmpUrl );	//日志。
		//对URL的递归深度，HOST span限制，及文件时间限制进行处理。
		if( this->isAllowedUrl(tmpUrl.toString() ) )
		{
			this->mUrlAccessMutex.lock();	//取出来下一个URL
			this->mUrlHashList[  tmpUrl.toString()  ] = 'R';
			this->mUrlAccessMutex.unlock();	//取出来下一个URL
			emit foundLink(tmpUrl.toString());
		}
	}
	html_parse_free_link_mt(linkList,linkCount);

	//fetch the image of this page
	linkCount = 0 ;
	linkList = html_parse_get_all_image(htmlcode.toAscii().data() , &linkCount );
	qDebug()<<"iamge tag count:"<< linkCount ;
	for( int j = 0 ; j < linkCount ; j ++ )
	{
		//srcList.append(QString(linkList[j]));
		//首先需要对取出来的URL进行判断，重写成绝对URL
		QUrl relative(linkList[j]);

		QUrl tmpUrl = uu.resolved(relative) ;
		if( tmpUrl.hasFragment() )
		{
			//去掉链接上的fragment.
			tmpUrl = QUrl(tmpUrl.toString().left(tmpUrl.toString().length() - tmpUrl.fragment().length() -1) ) ;
		}
		
		this->mUrlAccessMutex.lock();	//取出来下一个URL
		hit = this->mUrlHashList.find( ::qtMD5(tmpUrl.toString().toAscii() ) );
		if( hit != this->mUrlHashList.end() )
		{
			this->mUrlAccessMutex.unlock();	//取出来下一个URL
			continue;
		}
		this->mUrlAccessMutex.unlock();	//取出来下一个URL

		//this->putLog(tmpUrl.toString());	//日志。
		//this->mReadyList.append( tmpUrl.toString() );
		this->mUrlAccessMutex.lock();	//取出来下一个URL
		if( tmpUrl.toString().trimmed().length() > 0 )
		{
			this->mUrlHashList[tmpUrl.toString()] = 'R' ;
			emit foundLink(tmpUrl.toString());
		}
		this->mUrlAccessMutex.unlock();	//取出来下一个URL

		//emit nextRetriver();	//继续下一个
	}
	html_parse_free_link_mt(linkList,linkCount);	
}

bool WalkSiteThread::isAllowedUrl(QString url)
{
	char *apache_list_query_strings [] = { "C=N;O=D" , "C=M;O=A","C=S;O=A","C=D;O=A" };

	if( url.trimmed().length() <=0 ) return false ;

	for( int i = 0 ; i < 4 ; i ++)
	{
		if( url.endsWith(apache_list_query_strings[i]) ) return false ;
	}

	
	return true ;
}

bool WalkSiteThread::isNeedParsedUrl(QString url)	//是否是需要解析的文档模式
{
	//再查看是不是HTML页面。
	char * html_ext[] = { "o","exe","dll","rar","tar","gz","tgz","bz2","a","mp3","mp4","mpeg","swf","rm","rmvb",
			"asf","wmv" } ;
	
	for( int i = 0 ; i < sizeof( html_ext)/sizeof(char*) ; i ++ )
	{
		if( url.toLower().endsWith(QString(".%1").arg(QString(html_ext[i]))) ) return false ;
	}

	return true ;
}

bool WalkSiteThread::isAcceptedHost(QString url) 	//是否接受的主机名字。
{
	return true ;
}
bool WalkSiteThread::isAcceptedExt(QString url) 	//是否需要下载的扩展名。
{
	return true ;
}

//////////////////////
WalkSiteWndEx::WalkSiteWndEx(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
	////
	this->mMainMenu = new QMenu("WS_MAIN_MENU",this);
	this->mMainMenu->addAction(this->ui.actionStart);
	this->mMainMenu->addAction(this->ui.actionPause);
	this->mMainMenu->addAction(this->ui.actionRestart);
	this->mMainMenu->addSeparator();
	this->mMainMenu->addAction(this->ui.action_Follow_Ftp);
	this->mMainMenu->addAction(this->ui.action_Span_Hosts);
	this->mMainMenu->addAction(this->ui.action_TimeStamp);
	this->mMainMenu->addSeparator();
	this->mMainMenu->addAction(this->ui.actionSave_As);
	this->mMainMenu->addAction(this->ui.actionLoad);
	this->mMainMenu->addAction(this->ui.actionOpen_Directory);
	this->mMainMenu->addSeparator();
	this->mMainMenu->addAction(this->ui.actionHelp);
	this->mMainMenu->addAction(this->ui.actionFloating);
	this->mMainMenu->addAction(this->ui.actionHidden);

	this->mSettingMenu = new QMenu("WS_SET_MENU",this);
	this->mSettingMenu->addAction(this->ui.action_Follow_Ftp);
	this->mSettingMenu->addAction(this->ui.action_Span_Hosts);
	this->mSettingMenu->addAction(this->ui.action_TimeStamp);

	///
	this->mState = WSREADY ;

	/////connect
	QObject::connect(this,SIGNAL(customContextMenuRequested ( const QPoint &  ) ) ,
		this,SLOT(onMainCustomContextMenuRequested ( const QPoint & ) ) );
	//QObject::connect(this->ui.wswe_setting_menu_button,SIGNAL(clicked()), this,SLOT(onSettingButtonClicked()) ) ;

	QObject::connect(this->ui.actionStart,SIGNAL(triggered()),this,SLOT(onStart()) );
	QObject::connect(this->ui.actionPause,SIGNAL(triggered()),this,SLOT(onPause()) );
	QObject::connect(this->ui.actionRestart,SIGNAL(triggered()),this,SLOT(onRestart()) );
	QObject::connect(this->ui.actionFloating,SIGNAL(triggered(bool)),this,SLOT(onShowFloat(bool )));

	QObject::connect(this->ui.wsw_tb_show_save_path,SIGNAL(clicked()),this,SLOT(onSelectSavePath()));
	
	QObject::connect(&this->mCronTimer,SIGNAL(timeout()),this,SLOT(onTimeout()));

	QObject::connect(this->ui.wswe_setting_menu_button,SIGNAL(clicked(bool)),
		this,SLOT(setStackWidgetCurrentIndex ( bool  )) );

}

WalkSiteWndEx::~WalkSiteWndEx()
{
	
}

void	WalkSiteWndEx::onMainCustomContextMenuRequested ( const QPoint & pos ) 
{
	this->mMainMenu->popup(QCursor::pos ());
}
void	WalkSiteWndEx::onSettingButtonClicked()
{
	this->mSettingMenu->popup(QCursor::pos() );
}

void	WalkSiteWndEx::onSelectSavePath()	//选择保存根目录
{
	//检测当前运行状态，如果在运行状态不能执行下面的操作。
	QString path = QFileDialog::getExistingDirectory(this,"Select Save Path",this->ui.wsw_cb_save_to->currentText());
	
	if( path.isEmpty() || path.isNull() || path.length() == 0 )
	{
	}
	else
	{
		WalkSiteThread::mSavePath = path ;
		this->ui.wsw_cb_save_to->setEditText(path);
	}
}

void	WalkSiteWndEx::onShowFloat(bool floating )  //
{
	QDockWidget * parent = static_cast<QDockWidget*>(this->parentWidget());
	parent->setFloating(!parent->isFloating());

}

	/////////////
void	WalkSiteWndEx::onStart () 
{	
	if( this->mState == WSPAUSED )
	{
		for( int i = 0 ; i < 5 ; i ++)
		{
			this->mOneThread = WalkSiteThread::mAllThreadList.at(i);
			this->mOneThread->isPaused = false ;
			this->mOneThread->start();
		}			
	}
	else
	{	

		WalkSiteThread::mStartUrl = this->ui.wsw_le_base_url->text();
		WalkSiteThread::mSavePath = this->ui.wsw_cb_save_to->currentText();
		WalkSiteThread::mUrlHashList[WalkSiteThread::mStartUrl] = 'R' ;

		for( int i = 0 ; i < 5 ; i ++)
		{
			this->mOneThread = new WalkSiteThread(this);
			this->mOneThread->isPaused = false ;
			WalkSiteThread::mAllThreadList.append(this->mOneThread);
			QObject::connect(this->mOneThread,SIGNAL(foundLink(QString)),this,SLOT(onFoundLink(QString)));

			this->mOneThread->start();
		}		
	}
	this->mState = WSRUNNING;
	this->mCronTimer.start(1000);
}
void WalkSiteWndEx::onTimeout()
{
	WalkSiteThread * t = 0 ;

	for( int i = 0 ; i < 5 ; i ++)
	{			
		t = WalkSiteThread::mAllThreadList.at(i);
		qDebug()<< i << t->isRunning() ;
		
	}
	for( int i = 0 ; i < 5 ; i ++)
	{			
		t = WalkSiteThread::mAllThreadList.at(i);
		//qDebug()<< i << t->isRunning() ;
		if( ! t->isRunning() )
		{			
			if( i ==4 )
			{
				qDebug()<< WalkSiteThread::mUrlHashList ;
				//任务完成通知本实例的其他方法。
				this->mCronTimer.stop();
				//exit(0);
			}
		}
		else
		{
			break ;
		}
	}	
	
}

void WalkSiteWndEx::onFoundLink(QString link)
{
	WalkSiteThread * t = 0 ;
	for( int i = 0 ; i < 5 ; i ++)
	{			
		t = WalkSiteThread::mAllThreadList.at(i);
		qDebug()<< i << t->isRunning() ;
		if( !  t->isRunning() )
		{
			t->start();
			break ;
		}
	}
	this->putLog( link ) ;
}

void	WalkSiteWndEx::onPause() 
{
	this->mCronTimer.stop();

	if( this->mState == WSPAUSED )
	{
		qDebug()<<"already paused";
		return ;
	}
	this->mState = WSPAUSED ;

	WalkSiteThread * t = 0 ;
	for( int i = 0 ; i < 5 ; i ++)
	{			
		t = WalkSiteThread::mAllThreadList.at(i);		
		t->isPaused = true ;
		//t->wait();
		//delete t ; t = 0 ;
	}
}
void	WalkSiteWndEx::onRestart()
{
	this->onPause();
	this->onStart();
	
}

void	WalkSiteWndEx::onShowRadarScanner(bool show) 
{
}
void	WalkSiteWndEx::onOpenDirectory() 
{
	QDesktopServices::openUrl(this->ui.wsw_cb_save_to->currentText());
}
void	WalkSiteWndEx::onSaveState() 
{
}
void	WalkSiteWndEx::onLoadState() 
{
}

void	WalkSiteWndEx::onCrawledPage()
{
}
void	WalkSiteWndEx::onCrawledData()
{
}
void	WalkSiteWndEx::onSpeedChangeed(int speed )
{

}

void WalkSiteWndEx::putLog(QString log)
{
	//int length = this->ui.wsw_te_walk_log->toPlainText().length() ;
	//if( length > 3000 )
	//{
	//	this->ui.wsw_te_walk_log->clear();
	//}

	//this->ui.wsw_te_walk_log->insertPlainText(log + QChar('\n'));
	//this->ui.wsw_te_walk_log->verticalScrollBar()->setValue(this->ui.wsw_te_walk_log->verticalScrollBar()->maximum());

}

void WalkSiteWndEx::setStackWidgetCurrentIndex(bool checked)
{
	this->ui.stackedWidget->setCurrentIndex(checked ? 1 : 0 );
	if( checked == false )
	{
		this->ui.wswe_setting_menu_button->setIcon(QIcon("icons/crystalsvg/32x32/actions/previous.png"));
	}
	else
	{
		this->ui.wswe_setting_menu_button->setIcon(QIcon("icons/crystalsvg/32x32/actions/next.png"));
	}
}


