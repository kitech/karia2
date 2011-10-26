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

#include <QRect>
#include <QSize>
#include <QProcess>
#include <QDockWidget>

#include <QToolTip>
//////
#include "libng/html-parse.h"

#include "walksitewnd.h"
#include "radarscanner.h"


WalkSiteWnd::WalkSiteWnd(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);

	//this->mMainWnd = static_cast<QMainWindow*> (parent );
	/////////////
	this->ui.mainToolBar->addAction(this->ui.actionStart);
	this->ui.mainToolBar->addAction(this->ui.actionStop);
	this->ui.mainToolBar->addAction(this->ui.actionRestart);
	this->ui.mainToolBar->addSeparator();

	this->ui.mainToolBar->addAction(this->ui.actionSettings);

	mSavePath = ".";
	mSpanHosts = false ;
	mFollowFtp = true ;
	mTimeStamp = false ;
	mRecursiveLevel = -1 ;
	mMaxSize = 0 ;
	mThreadCount = 1 ;

	//
	mCtrlSock = 0 ;
	mDataSock = 0 ;
	
	//mWalkState
	this->mWalkState = -1 ;	//就绪状态

	//初始化统计信息。
	this->mWalkPageCount = 0 ;
	this->mWalkPageTotalSize = 0 ;
	this->mCurrentSpeed = 0.0 ;
	this->mErrorPageCount = 0 ;
	this->mRTList = 0 ;

	//
	this->mAcceptHost = new QStringListModel();
	this->mRejectHost = new QStringListModel();
	this->mAcceptExt = new QStringListModel();
	this->mRejectExt = new QStringListModel();
	this->ui.wsw_lv_accept_host->setModel(this->mAcceptHost);
	this->ui.wsw_lv_reject_host->setModel(this->mRejectHost);
	this->ui.wsw_lv_accept_ext->setModel(this->mAcceptExt);
	this->ui.wsw_lv_reject_ext->setModel(this->mRejectExt);

	//connect signals
	QObject::connect(this->ui.actionSettings,SIGNAL(triggered(bool)),this,SLOT(onShowSetting(bool)));
	QObject::connect(this->ui.wsw_tb_show_save_path,SIGNAL(clicked()),this,SLOT(onSelectSavePath()));
	QObject::connect(this->ui.wsw_pb_start_walk,SIGNAL(clicked()),this,SLOT(onStartRetriver()));

	QObject::connect(this,SIGNAL(nextRetriver()),this,SLOT(onNextRetriver()));
	QObject::connect(this,SIGNAL(walkSiteFinished()),this,SLOT(onWalkSiteFinished()));

	//
	QObject::connect(this->ui.wsw_pb_add_accept_host,SIGNAL(clicked()),this,SLOT(onAddAcceptHost()));
	QObject::connect(this->ui.wsw_pb_delete_accept_host,SIGNAL(clicked()),this,SLOT(onDeleteAcceptHost()));
	QObject::connect(this->ui.wsw_pb_add_reject_host,SIGNAL(clicked()),this,SLOT(onAddRejectHost()));
	QObject::connect(this->ui.wsw_pb_delete_reject_host,SIGNAL(clicked()),this,SLOT(onDeleteRejectHost()));
	QObject::connect(this->ui.wsw_pb_add_reject_ext,SIGNAL(clicked()),this,SLOT(onAddRejectExtension()));
	QObject::connect(this->ui.wsw_pb_delete_reject_ext,SIGNAL(clicked()),this,SLOT(onDeleteRejectExtension()));
	QObject::connect(this->ui.wsw_pb_add_accept_ext,SIGNAL(clicked()),this,SLOT(onAddAcceptExtension()));
	QObject::connect(this->ui.wsw_pb_delete_accept_ext,SIGNAL(clicked()),this,SLOT(onDeleteAcceptExtension()));
	QObject::connect(this->ui.actionStop,SIGNAL(triggered()),this,SLOT(onPauseWalkSite()));
	QObject::connect(this->ui.actionStart,SIGNAL(triggered()),this,SLOT(onStartWalkSite()));
	QObject::connect(this->ui.actionSave_As,SIGNAL(triggered()),this,SLOT(onSaveState()));
	QObject::connect(this->ui.actionLoad,SIGNAL(triggered()),this,SLOT(onRestoreState()));
	QObject::connect(this->ui.actionNew,SIGNAL(triggered()),this,SLOT(onNewWalkSite()));
	QObject::connect(this->ui.actionShow_Radar_Scanner,SIGNAL(triggered(bool)),this,SLOT(onShowRadarScanner(bool)));

	//恢复原有数据。
	this->onRestoreState();
	if( this->mBaseUrl.length() == 0 )
	{
		//原来没有下载任务
		this->onNewWalkSite();
	}
	else
	{
		//this->ui.wsw_pb_start_walk->setEnabled(false);
	}

	//计时器超时更新用户界面上的统计信息
	QObject::connect( & this->mWalkTimer , SIGNAL(timeout()), this, SLOT(onUpdateStatInfo()));
	
	//初始化雷达控件
	this->mRadar = new RadarScanner(this->ui.graphicsView);
	this->mRadar->show();
	this->ui.graphicsView_2->setMinimumSize(this->ui.graphicsView->minimumSize());
	this->mRadar2 = new RadarScanner(this->ui.graphicsView_2);
	this->mRadar2->setDirection(false);
	this->mRadar2->show();

}

WalkSiteWnd::~WalkSiteWnd()
{
	if( this->mWalkTimer.isActive() )
	{
		this->mWalkTimer.stop();
	}
	delete this->mAcceptHost ;
	delete this->mRejectHost;
	delete this->mAcceptExt;
	delete this->mRejectExt ;
}

void WalkSiteWnd::onShowRadarScanner(bool show)
{
	if( show) 
	{
		this->mRadar->show();
		this->mRadar2->show();
	}
	else
	{
		this->mRadar->setHidden(true) ;
		this->mRadar2->setHidden(true);
	}
}

void WalkSiteWnd::onSelectSavePath()
{
	QString path ;

	path = QFileDialog::getExistingDirectory(this).trimmed();

	if( path.isEmpty() || path.isNull() || path.length() == 0 )
	{
	}
	else
	{
		this->ui.wsw_cb_save_to->setEditText(path);
	}
}

void WalkSiteWnd::onShowSetting(bool show)
{
	this->ui.wsw_setting_frame->setHidden(!show);
}

//新的遍历
void WalkSiteWnd::onNewWalkSite() 
{
	//清理原来的任务信息，更改菜单按钮状态，设置新遍历的初始值。
	this->onPauseWalkSite();
	this->ui.actionStart->setEnabled(false);
	this->ui.actionStop->setEnabled(false);
	this->ui.wsw_pb_start_walk->setEnabled(true);
	this->ui.wsw_cb_follow_ftp->setChecked(true);
	this->ui.wsw_cb_span_hosts->setChecked(false);
	this->ui.wsw_cb_time_stamp->setChecked(false);
	this->ui.wsw_le_base_url->setText("");
	this->ui.wsw_cb_save_to->setEditText(".");
	this->ui.wsw_sb_recursive_level->setValue(-1);
	this->ui.wsw_sb_max_size->setValue(0);
	this->ui.wsw_sb_thread_count->setValue(5);


	//
	this->mReadyList.clear();
	this->mReadyList.append(this->mBaseUrl);
	//this->mRunningList.clear();
	this->mDoneList.clear();	

	//删除socket吧？

}

void WalkSiteWnd::initRTList()
{
	QString log ;

	QTcpSocket * tmpSock ;
	//create socket object here
	rtqueue * rq = 0 ;
	this->mRTList = (rtqueue**)malloc(sizeof(rtqueue*)*this->mThreadCount);
	for( int i = 0 ; i < this->mThreadCount ; i ++ )
	{
		this->mRTList[i] = new rtqueue();
		
		tmpSock = new QTcpSocket();
		rq = this->mRTList[i];
		rq->mrSock = tmpSock ;
		rq->mrHFile = 0 ;
		rq->mrState = 0 ;
		rq->mrUrl = QString("");
		rq->mrHtmlCode = QString("");
		rq->mrHttpHeader = QString("");
		this->mCtrlSock = tmpSock ;
		this->mCtrlSock->setObjectName(QString("%1").arg(i));
			QObject::connect(this->mCtrlSock,SIGNAL(connected()),this,SLOT(onCtrlSockConnected()));
			QObject::connect(this->mCtrlSock,SIGNAL(disconnected()),this,SLOT(onCtrlSockDisconnected()));
			QObject::connect(this->mCtrlSock,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onCtrlSockError( QAbstractSocket::SocketError)));
			QObject::connect(this->mCtrlSock,SIGNAL(hostFound()),this,SLOT(onCtrlSockHostFound()));
			QObject::connect(this->mCtrlSock,SIGNAL(stateChanged ( QAbstractSocket::SocketState  )),this,SLOT(onCtrlSockStateChanged(QAbstractSocket::SocketState)));
			QObject::connect(this->mCtrlSock,SIGNAL(aboutToClose()),this,SLOT(onCtrlSockAboutToClose()));
			QObject::connect(this->mCtrlSock,SIGNAL(bytesWritten(qint64)),this,SLOT(onCtrlSockBytesWritten(qint64 )));
			QObject::connect(this->mCtrlSock,SIGNAL(readyRead()),this,SLOT(onCtrlSockReadyRead()));
			this->mCtrlSock = 0 ;
	}
	log = "Init walk site element done.";
	this->putLog( log );	//日志。
}

void WalkSiteWnd::destoryRTList() 
{
	QString log ;

	if( this->mRTList != 0 )
	{		
		//create socket object here
		rtqueue * rq = 0 ;
		//this->mRTList = (rtqueue**)malloc(sizeof(rtqueue*)*this->mThreadCount);
		for( int i = 0 ; i < this->mThreadCount ; i ++ )
		{
			rq = this->mRTList[i] ;		
			
			delete rq->mrSock ; rq->mrSock = 0 ;			
			if( rq->mrHFile != 0 ) delete rq->mrHFile ; rq->mrHFile = 0 ;
			delete this->mRTList[i] ; this->mRTList[i] = 0 ;
		}
		delete this->mRTList ; this->mRTList = 0 ;
	}
	log = "Init walk site element done.";
	this->putLog( log );	//日志。
}

void WalkSiteWnd::onStartRetriver()
{
	qDebug()<<__FUNCTION__;
	QUrl uu ;
	QString log ;

	//初始化一些变量，像 sock等。
	this->mBaseUrl = this->ui.wsw_le_base_url->text().trimmed();
	if( this->mBaseUrl.isNull() || this->mBaseUrl.isEmpty() )
	{
		qDebug()<<this->mBaseUrl ;
		return ;
	}
	//是否是支持的协议类型。
	uu = QUrl(this->mBaseUrl);
	if( uu.scheme().toUpper().compare("HTTP")==0 ||uu.scheme().toUpper().compare("HTTPS")==0
		||uu.scheme().toUpper().compare("FTP") == 0 )
	{
		log = QString("Base URL: %1").arg(this->mBaseUrl);
		this->putLog(log);
	}
	else
	{
		qDebug()<<"Not supported Scheme";
		log = QString("Base URL: %1\nNot supported Scheme\n Stopped.").arg(this->mBaseUrl);
		this->putLog(log);
		return ;
	}

	this->mSavePath = this->ui.wsw_cb_save_to->currentText().trimmed();
	if( this->mSavePath.isNull() || this->mBaseUrl.isEmpty() )
	{
		qDebug()<<this->mSavePath  ;
		return ;
	}
	//更新界面
	{
		this->ui.wsw_pb_start_walk->setEnabled(false);
		this->ui.actionStop->setEnabled(true);		
	}

	//开始计时
	this->mStartTime = QDateTime::currentDateTime();
	this->mWalkTimer.start(1000);	//每一秒钟更新一次。

	//取得下载控制参数。在该任务没有完成以前，这些参数不再重新载入。在暂停任务并重新启动时，这些参数是需要重新读入的。
	this->mFollowFtp = this->ui.wsw_cb_follow_ftp->isChecked();
	this->mSpanHosts = this->ui.wsw_cb_span_hosts->isChecked();
	this->mTimeStamp = this->ui.wsw_cb_time_stamp->isChecked();
	this->mRecursiveLevel = this->ui.wsw_sb_recursive_level->text().toInt();
	this->mMaxSize = this->ui.wsw_sb_max_size->value();
	this->mThreadCount = this->ui.wsw_sb_thread_count->value() ;

	//
	this->mReadyList.clear();
	this->mReadyList.append(this->mBaseUrl);
	//this->mRunningList.clear();
	this->mDoneList.clear();	
	
	this->initRTList();

	log = "Init walk site element done.";
	this->putLog( log );	//日志。

	//
	//emit nextRetriver();
	this->onStartWalkSite();

}

void WalkSiteWnd::onWalkSiteFinished()
{
	qDebug()<<__FUNCTION__;
	qDebug()<<this->mReadyList.size() ;
	//qDebug()<<this->mRunningList.size() ;
	qDebug()<<this->mDoneList.size() ;
	this->mReadyList.clear();
	//this->mRunningList.clear();
	this->mDoneList.clear();

	this->destoryRTList();

}

void WalkSiteWnd::onNextRetriver()
{
	qDebug()<<__FUNCTION__;

	if( this->mReadyList.size() == 0 )
	{
		emit walkSiteFinished();
		return ;
	}

	if( this->mWalkState == this->WS_PAUSED )
	{
		qDebug()<< " Paused by User " ;
		return ;
	}

	QString host;
	int port ;
	QString query ;

	
	//connect to host
	QTcpSocket * tmpSock = 0 ;
	struct rtqueue * rq = 0 ;

	//限制速度


	this->mRTMutex.lock();
	qDebug()<<"locked mutext";
	for( int i = 0 ; i < this->mThreadCount ; i ++ )
	{
		rq = this->mRTList[i];
		if( rq->mrState == 0 )
		{
			rq->mrState =1 ;
			break ;
		}
		rq = 0 ;
	}
	qDebug()<<"unlocking mutext";
	this->mRTMutex.unlock();

		if( rq != 0 )
		{
			
			tmpSock = rq->mrSock ;
			//tmpSock->setObjectName("run");
			tmpSock->disconnectFromHost();
			
			//
			if( this->mReadyList.count() <= 0 )
			{
				qDebug()<<"no more ready list exist , i think it completed " ;
				return ;
			}
			QString siteUrl = this->mReadyList.takeAt(0);
			
			QString oldUrl = rq->mrUrl ;
			
			if( oldUrl.length()>0) 
			{				
				this->mDoneList.append(oldUrl);
			}
			this->mWalkPageCount ++ ;
			
			//this->mRunningList.clear();
			//this->mRunningList.append(siteUrl);

			QUrl uu(siteUrl);

			host = uu.host();
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
				qDebug()<<"not supported scheme";
				rq->mrState = 0 ;
				return ;
			}
					
			rq->mrUrl = siteUrl ;
			tmpSock->connectToHost(host,port);			
		}
		else
		{
			qDebug()<<" no  left segment for download , waiting one that stopped";
		}
	
}

void WalkSiteWnd::onCtrlSockConnected()
{
	qDebug()<<__FUNCTION__;
	QTcpSocket * tmpSock = 0 ;
	struct rtqueue * rq = 0 ;
	QHttpRequestHeader hrh ;

	tmpSock = static_cast<QTcpSocket*>(this->sender());
	rq = this->mRTList[tmpSock->objectName().toInt()];

	QUrl url(rq->mrUrl);
	

	hrh.setValue("Host",url.host() ) ;		
	hrh.setValue("Range", QString("bytes=%1-").arg(0) ) ;
	hrh.setValue("Accept","*/*");
	hrh.setValue("User-Agent","Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)");
	hrh.setValue("Pragma","no-cache");
	hrh.setValue("Cache-Control","no-cache");
	hrh.setValue("Referer","");
	if( url.encodedQuery().length()>0)
		hrh.setRequest("GET",url.path()+"?"+url.encodedQuery().data());
	else
			hrh.setRequest("GET",url.path());
	hrh.setValue("Connection", "close" ) ;		

	rq->mrReadHeaderFinished = false ;
	rq->mrHttpHeader.clear();
	rq->mrHFile = 0 ;
	rq->mrHtmlCode.clear();
	tmpSock->write(hrh.toString().toAscii());

}

void WalkSiteWnd::onCtrlSockError( QAbstractSocket::SocketError socketError)
{
	QTcpSocket * tmpSock = 0 ;
	struct rtqueue * rq = 0 ;
	char speedBuff[32] = {0} ;

	tmpSock = static_cast<QTcpSocket*>(this->sender());
	rq = this->mRTList[tmpSock->objectName().toInt()];

	qDebug()<<__FUNCTION__<<tmpSock->objectName()<<socketError << tmpSock->errorString();
	if( socketError ==QAbstractSocket::RemoteHostClosedError && rq->mrHFile != 0 )
	{
		this->mWalkPageTotalSize += rq->mrHFile->size();	//计算已经取得的文件的总和。
		this->mCurrentSpeed = this->GetTaskRate(this->mStartTime,QDateTime::currentDateTime(),this->mWalkPageTotalSize,speedBuff ,0.0);
		this->mCurrentSpeedString = QString(speedBuff);
		
		QString fname = rq->mrHFile->fileName().toUpper() ;
		rq->mrHFile->close();
		delete rq->mrHFile ;
		rq->mrHFile = 0 ;
		if( fname.endsWith("/") || fname.endsWith(".HTML") || fname.endsWith(".HTM"))
		{
			this->parseWebPage(rq->mrUrl,rq->mrHtmlCode);
		}
	}
	else
	{
		this->mErrorPageCount ++ ; 
	}
	if( rq != 0 )
	{
		rq->mrState = 0 ;
		rq->mrReadHeaderFinished = 0 ;
		rq->mrHtmlCode.clear();
		rq->mrUrl.clear();
	}

	emit nextRetriver();	//继续下一个
}

void WalkSiteWnd::onCtrlSockHostFound()
{
	qDebug()<<__FUNCTION__;
}

void WalkSiteWnd::onCtrlSockStateChanged( QAbstractSocket::SocketState socketState )
{
	qDebug()<<__FUNCTION__;
}

void WalkSiteWnd::onCtrlSockDisconnected()
{
	qDebug()<<__FUNCTION__;
}

void WalkSiteWnd::onCtrlSockBytesWritten(qint64 bytes)
{
	qDebug()<<__FUNCTION__;
}
void WalkSiteWnd::onCtrlSockAboutToClose()
{
	qDebug()<<__FUNCTION__;
}
void WalkSiteWnd::onCtrlSockReadyRead()
{
	qDebug()<<__FUNCTION__;
	QTcpSocket * tmpSock = 0 ;
	struct rtqueue * rq = 0 ;

	QString strBuff;
	char chBuff[8096] = {0};
	int readyBytes = 0 ;
	int oldReadBuffSize = 0 ;


	//
	tmpSock = static_cast<QTcpSocket*>(this->sender());
	rq = this->mRTList[tmpSock->objectName().toInt()];

	readyBytes = tmpSock->bytesAvailable();
	qDebug()<<readyBytes<<" can Read now"<<tmpSock->objectName().toInt();

	if(! rq->mrReadHeaderFinished )
	{
		//set read buffer size to 1 , then we can control the speed
		oldReadBuffSize = tmpSock->readBufferSize();
		tmpSock->setReadBufferSize(1);
		while(  (tmpSock->readLine(chBuff , sizeof(chBuff) - 1  ) > 0 ) )
		{
			//readLine is impl by getchar,so no problem here
			rq->mrHttpHeader += QString(chBuff);
			//qDebug()<<rb;
			if( rq->mrHttpHeader.endsWith(QString("\r\n\r\n")))
			{
				qDebug()<<"read head end "<<rq->mrHttpHeader ;
				rq->mrReadHeaderFinished = true ;
				break;
			}

		}
		tmpSock->setReadBufferSize(oldReadBuffSize);	// reset the buff size to default value 
	}

	if( rq->mrReadHeaderFinished ) ///////read file block
	{
		if( rq->mrHFile == 0 )
		{
			//取得文件要存储的路径，如果路径不存在则创建之，并创建该文件句柄
			QString curl = rq->mrUrl ;//	假设我们只有一个线程在下载文件。
			QString path , file ;
			QUrl uu(curl);
			QFileInfo fi(uu.path());
			path = QString("%1/%2/%3/").arg(this->mSavePath).arg(uu.host()).arg(fi.path());
			QDir dir(this->mSavePath);
			qDebug()<<"mkdir for create file: "<< path ;
			if( dir.exists(path) )
			{

			}
			else
			{				
				dir.mkpath(path);
			}

			//QFileInfo fi(curl);
			if( fi.fileName().isEmpty() || fi.fileName().isNull() )
			{
				file = path + QString("index.html");
			}
			else
			{
				file = path + fi.fileName();
			}
			rq->mrHFile = new QFile(file);
			rq->mrHFile->open(QIODevice::WriteOnly);
			qDebug()<<"Open file for write";
		}

		readyBytes = tmpSock->bytesAvailable();
		int left = readyBytes ;
		int nret = 0 ;
		while( left > 0 )
		{
			memset(chBuff,0,sizeof(chBuff));
			nret = tmpSock->read(chBuff,sizeof(chBuff)-2);
			//qDebug()<<chBuff ;
			left -= nret ;
			strBuff += QString(chBuff);
			rq->mrHFile->write(chBuff,nret);
			rq->mrHtmlCode += QString(chBuff) ;
		}
	}
}

void WalkSiteWnd::parseWebPage( QString & currUrl , QString & htmlcode)
{
	qDebug()<<__FUNCTION__;
	int linkCount = 0;
	char ** linkList = 0 ;

	QUrl uu(currUrl);
	QFileInfo fi(currUrl);
	QString path = uu.path();
	
	//QString absUrl = QString("%1://%2:%3/%4/").arg(uu.scheme()).arg(uu.host()).arg(uu.port(80)).arg(uu.path()) ;

	linkCount = 0;
	linkList = html_parse_get_all_link(htmlcode.toAscii().data() , &linkCount );
	qDebug()<<linkCount ;
	for( int j = 0 ; j < linkCount ; j ++ )
	{
		//srcList.append(QString(linkList[j]));
		//首先需要对取出来的URL进行判断，重写成绝对URL
		QUrl relative(linkList[j]);
		if( this->mDoneList.contains(uu.resolved(relative).toString()))
			continue;
		if( this->mReadyList.contains(uu.resolved(relative).toString()))
			continue; 
		QUrl tmpUrl = uu.resolved(relative) ;
		this->putLog(tmpUrl.toString());	//日志。
		//对URL的递归深度，HOST span限制，及文件时间限制进行处理。
		if( this->isAllowedUrl(tmpUrl.toString()))
			this->mReadyList.append( uu.resolved(relative).toString() );
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
		else
		{
			continue ;
		}
		if( this->mDoneList.contains(tmpUrl.toString()))
			continue;
		if( this->mReadyList.contains(tmpUrl.toString()))
			continue; 
		this->putLog(tmpUrl.toString());	//日志。
		this->mReadyList.append( tmpUrl.toString() );

		emit nextRetriver();	//继续下一个
	}
	html_parse_free_link_mt(linkList,linkCount);
	
}

void WalkSiteWnd::putLog(QString log)
{
	int length = this->ui.wsw_te_walk_log->toPlainText().length() ;
	if( length > 3000 )
	{
		this->ui.wsw_te_walk_log->clear();
	}

	this->ui.wsw_te_walk_log->insertPlainText(log + QChar('\n'));
	this->ui.wsw_te_walk_log->verticalScrollBar()->setValue(this->ui.wsw_te_walk_log->verticalScrollBar()->maximum());
}

bool WalkSiteWnd::isAllowedUrl(QString url)
{
	//the rules is , follow ftp , span host and recursive level 
	//the rules is the allowed extentions 
	//the rules is the allowed host
	//the rules is the allowed timestamp
	QUrl cu(this->mBaseUrl);
	QUrl uu(url);
	if( ! this->mFollowFtp && uu.scheme().toUpper().compare("FTP") == 0 ) return false ;
	if( ! this->mSpanHosts && uu.host().compare(cu.host()) != 0 ) return false ;
	

	return true ;
}


void WalkSiteWnd::onAddAcceptHost()
{
	QString host;

	host = QInputDialog::getText(this,"","");

	if( host.isEmpty() || host.isNull() || host.length() == 0) 
	{
		return;
	}
	else
	{
		int row = this->mAcceptHost->rowCount();
		this->mAcceptHost->insertRow(row);
		this->mAcceptHost->setData(this->mAcceptHost->index(row,0),host);
	}
}
void WalkSiteWnd::onDeleteAcceptHost()
{
	QItemSelectionModel *ism = this->ui.wsw_lv_accept_host->selectionModel();
	QModelIndexList mil = ism->selectedIndexes();
	if( mil.count() > 0 )
	{
		this->mAcceptHost->removeRow(mil.at(0).row());
	}
}
void WalkSiteWnd::onAddRejectHost()
{
	QString host;

	host = QInputDialog::getText(this,"","");

	if( host.isEmpty() || host.isNull() || host.length() == 0) 
	{
		return;
	}
	else
	{
		int row = this->mRejectHost->rowCount();
		this->mRejectHost->insertRow(row);
		this->mRejectHost->setData(this->mRejectHost->index(row,0),host);
	}
}
void WalkSiteWnd::onDeleteRejectHost()
{
	QItemSelectionModel *ism = this->ui.wsw_lv_reject_host->selectionModel();
	QModelIndexList mil = ism->selectedIndexes();
	if( mil.count() > 0 )
	{
		this->mRejectHost->removeRow(mil.at(0).row());
	}
}
void WalkSiteWnd::onAddAcceptExtension()
{
	QString host;

	host = QInputDialog::getText(this,"","");

	if( host.isEmpty() || host.isNull() || host.length() == 0) 
	{
		return;
	}
	else
	{
		int row = this->mAcceptExt->rowCount();
		this->mAcceptExt->insertRow(row);
		this->mAcceptExt->setData(this->mAcceptExt->index(row,0),host);
	}
}
void WalkSiteWnd::onDeleteAcceptExtension()
{
	QItemSelectionModel *ism = this->ui.wsw_lv_accept_ext->selectionModel();
	QModelIndexList mil = ism->selectedIndexes();
	if( mil.count() > 0 )
	{
		this->mAcceptExt->removeRow(mil.at(0).row());
	}
}
void WalkSiteWnd::onAddRejectExtension()
{
	QString host;

	host = QInputDialog::getText(this,"","");

	if( host.isEmpty() || host.isNull() || host.length() == 0) 
	{
		return;
	}
	else
	{
		int row = this->mRejectExt->rowCount();
		this->mRejectExt->insertRow(row);
		this->mRejectExt->setData(this->mRejectExt->index(row,0),host);
	}
}
void WalkSiteWnd::onDeleteRejectExtension()
{
	QItemSelectionModel *ism = this->ui.wsw_lv_reject_ext->selectionModel();
	QModelIndexList mil = ism->selectedIndexes();
	if( mil.count() > 0 )
	{
		this->mRejectExt->removeRow(mil.at(0).row());
	}
}

/**
 * 保存当前的状态，以更下次继续下载。
 */
void WalkSiteWnd::onSaveState()
{
	qDebug()<<__FUNCTION__ ;
	
	QString log ;
	QString file = "data/walksite.dat";
	QFile fi(file);
	
	QDir dir ;
	dir.mkdir("data");

	if( fi.exists() )
	{
		fi.remove();
	}
	fi.open(QIODevice::WriteOnly|QIODevice::Unbuffered);

	QDataStream out(&fi);

	out<<this->mBaseUrl;
	log = QString("save url %1").arg(this->mBaseUrl) ;
	this->putLog(log);
	out<<this->mSavePath;
	out<<this->mSpanHosts;
	out<<this->mFollowFtp ;
	//setting vars
	out<<this->mTimeStamp;
	out<<this->mRecursiveLevel ;
	out<<this->mMaxSize ;
	out<<this->mThreadCount ;

	//stat
	out<<this->mParsedHtmlPageCount ;
	out<<this->mRunningSocketCount;
	out<<this->mWalkPageCount ;
	out<<this->mWalkPageTotalSize ;
	out<<this->mErrorPageCount ;

	//
	out<<this->mReadyList;
	out<<this->mDoneList;
	out<<this->mFaildList ;
}
void WalkSiteWnd::onRestoreState()
{
	qDebug()<<__FUNCTION__ ;

	QString log ;
	QString file = "data/walksite.dat";
	QFile fi(file);
	
	QDir dir ;
	dir.mkdir("data");

	if( fi.exists() )
	{
		//fi.remove();
		fi.open(QIODevice::ReadOnly);
		QDataStream in(&fi);

		in>>this->mBaseUrl;
		log = QString("load url %1").arg(this->mBaseUrl) ;
		this->putLog(log);
		in>>this->mSavePath;
		in>>this->mSpanHosts;
		in>>this->mFollowFtp ;
		//setting vars
		in>>this->mTimeStamp;
		in>>this->mRecursiveLevel ;
		in>>this->mMaxSize ;
		in>>this->mThreadCount ;

		//stat
		in>>this->mParsedHtmlPageCount ;
		in>>this->mRunningSocketCount;
		in>>this->mWalkPageCount ;
		in>>this->mWalkPageTotalSize ;
		in>>this->mErrorPageCount ;

		//
		in>>this->mReadyList;
		in>>this->mDoneList;
		in>>this->mFaildList ;
		
		log = QString("load done.") ;
		this->putLog(log);

		//界面操作
		if( this->mBaseUrl.length() > 0 )	//原有记录有效
		{
			this->ui.wsw_pb_start_walk->setEnabled(false);
			this->ui.wsw_le_base_url->setText(this->mBaseUrl);
			this->ui.wsw_cb_save_to->setEditText(this->mSavePath);
			this->ui.actionStart->setEnabled(true);
			this->ui.actionStop->setEnabled(false);

			this->ui.wsw_cb_follow_ftp->setChecked(this->mFollowFtp);
			this->ui.wsw_cb_span_hosts->setChecked(this->mSpanHosts);
			this->ui.wsw_cb_time_stamp->setChecked(this->mTimeStamp);

			this->ui.wsw_sb_recursive_level->setValue(this->mRecursiveLevel);
			this->ui.wsw_sb_max_size->setValue(this->mMaxSize);
			this->ui.wsw_sb_thread_count->setValue(this->mThreadCount);
			
			//
			this->initRTList();
		}
		else
		{
			this->ui.wsw_pb_start_walk->setEnabled(true);
			this->ui.actionStart->setEnabled(false);
			this->ui.actionStop->setEnabled(false);
			
			this->ui.wsw_cb_follow_ftp->setChecked(true);
			this->ui.wsw_cb_span_hosts->setChecked(false);
			this->ui.wsw_cb_time_stamp->setChecked(false);
			this->ui.wsw_le_base_url->setText("");
			this->ui.wsw_cb_save_to->setEditText(".");
			this->ui.wsw_sb_recursive_level->setValue(-1);
			this->ui.wsw_sb_max_size->setValue(0);
			this->ui.wsw_sb_thread_count->setValue(5);

		}
	}
	
}

/**
 * 暂时停止当前的抓取数据
 */
void WalkSiteWnd::onPauseWalkSite()
{
	qDebug()<<__FUNCTION__ ;

	if( this->mWalkState == this->WS_PAUSED )
	{
		qDebug()<<"already paused";
		return ;
	}
	if( this->mRTList == 0 )
	{
		qDebug()<<" not started ";
		return ;
	}
	//更新控制界面按钮
	{
		this->ui.actionStart->setEnabled(true);
		this->ui.actionStop->setEnabled(false);
	}
	//connect to host
	QTcpSocket * tmpSock = 0 ;
	struct rtqueue * rq = 0 ;

	this->mWalkState = this->WS_PAUSED ;
	for( int i = 0 ; i < this->mThreadCount ; i ++ )
	{
		rq = this->mRTList[i];
		if( rq->mrSock->isOpen() )
		{
			this->putLog("close socket");
			rq->mrSock->close();
			rq->mrSock->disconnectFromHost();
		}
		
		if( rq->mrState == 1 )
		{
			rq->mrState =0 ;	
			qDebug()<<"no done task "<<rq->mrUrl ;

			this->mReadyList.append(rq->mrUrl);	//将未完成的任务添加到就绪表中。
			rq->mrUrl = "";
		}		
	}
	this->mPauseTime = QDateTime::currentDateTime() ;
	this->mRadar->onStopScan();
	this->mRadar2->onStopScan();	
}

/**
 * 开始抓取数据　
 */
void WalkSiteWnd::onStartWalkSite() 
{
	if( this->mWalkState == this->WS_RUNNING )
	{
		qDebug()<<"already running ";
		return ;
	}
	if( this->mRTList == 0 )
	{
		qDebug()<<" not started ";
		return ;
	}
	//更新控制界面按钮
	{
		this->ui.actionStart->setEnabled(false);
		this->ui.actionStop->setEnabled(true);
	}

	this->mWalkState = this->WS_RUNNING ;
	this->mRadar->onStartScan();
	this->mRadar2->onStartScan();
	emit this->nextRetriver() ;

}




/**
 * 在计时器超时时更新用户界面上的统计信息。
 */
void WalkSiteWnd::onUpdateStatInfo() 
{
	//需要更新的信息有：
	//compeleted page , page size , ave speed , error page
	const char *rate_names[] = {"B", "KB", "MB", "GB" };
	long totalSize = this->mWalkPageTotalSize ;
	int units = 0 ;
	if( this->mWalkPageTotalSize < 1024 )
		units = 0  ;
	else if ( this->mWalkPageTotalSize < 1024 * 1024  )
		units = 1 , totalSize /= 1024 ;
	else if ( this->mWalkPageTotalSize < 1024 * 1024 * 1024 )
		units = 2 , totalSize /= 1024*1024 ;
	else 
		units = 3 , totalSize /= 1024*1024*1024 ;

	this->ui.wsw_le_retrive_page_num ->setText(QString("%1").arg(this->mWalkPageCount)) ;
	//this->ui.wsw_le_retrive_speed->setText(QString("%1").arg(this->mCurrentSpeed));
	this->ui.wsw_le_retrive_speed->setText(QString("%1").arg(this->mCurrentSpeedString));
	this->ui.wsw_le_retrive_total_size ->setText( QString("%1 %2").arg( totalSize ).arg(QString(rate_names[units])));
	this->ui.wsw_le_error_page_num->setText(QString("%1").arg(this->mErrorPageCount));

}


double WalkSiteWnd::GetTaskRate( QDateTime  btime , QDateTime  etime,long gotLength  , char * buff  , double adjustTime ) 
{
	//qDebug()<<__FUNCTION__<<"adjusttime: "<<adjustTime ;
	int i  = 0 ;
	double dtime = 0.0 ;
	long dlength = 0 ;
	long tlength = 0 ;
	double drate = 0.0 ;
	int units = 0 ;
	int pad = 1 ;
	const char *rate_names[] = {"B/s", "KB/s", "MB/s", "GB/s" };
	char    dbuff[32] = {0} ;
	
	dtime = this->TimeDiff(  btime ,  etime ) + adjustTime*1000.0 ;	//the adjustTime is msec
	//qDebug()<<"totalTIme:"<<dtime<<" "<<gotLength ;
	
	dlength = gotLength ;
	
	dtime == 0.0 ? 1 : dtime ;	
	drate = 1000.0 * dlength / dtime ;
	
	if (drate < 1024.0)
		units = 0;
	else if (drate < 1024.0 * 1024.0)
		units = 1, drate /= 1024.0;
	else if (drate < 1024.0 * 1024.0 * 1024.0)
		units = 2, drate /= (1024.0 * 1024.0);
	else
		/* Maybe someone will need this, one day. */
		units = 3, drate /= (1024.0 * 1024.0 * 1024.0);
	
	memset(dbuff,0,sizeof(dbuff));
	//sprintf_s is win function,but it warning out,so ...
#if defined( WIN32 ) && defined( _MSC_VER ) 
	sprintf_s (dbuff,sizeof(dbuff), pad ? "%7.2f %s" : "%.2f %s ", drate, rate_names[units]  );
#else
	snprintf (dbuff,sizeof(dbuff), pad ? "%7.2f %s" : "%.2f %s ", drate, rate_names[units]  );
#endif
	//sprintf(dbuff , "aaa3 %d" , units );
	//pthread_mutex_unlock(&mGcMutex);
	
	if( buff != 0 )
	{
		//strncpy(buff, dbuff,sizeof(dbuff)) ;
		char * p = dbuff ;
		char * q = buff ;
		while( (*q ++ = *p++) != '\0' ) ;
		
	}
	
	return drate ;
	
	return 0.0 ;
}


double WalkSiteWnd::TimeDiff( QDateTime & btime , QDateTime & etime )
{
	if(  etime == btime )	return 0.0 ;

	//fprintf(stderr , "%ld %ld :: %ld %ld \n" , btime->tv_sec , btime->tv_usec  , 
	//			etime->tv_sec , etime->tv_usec );
	int dsec = etime.toTime_t() - btime.toTime_t() ;
	int dmsec = etime.time().msec() - btime.time().msec();

	return dsec * 1000.0 + dmsec ;

	//return ((etime->tv_sec - btime->tv_sec) * 1000.0  + (etime->tv_usec - btime->tv_usec) / 1000.0);	
	return 0.0 ;
}


