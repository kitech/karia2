#ifndef WALKSITEWNDEX_H
#define WALKSITEWNDEX_H

#include <QStandardItemModel>
#include <QAbstractItemModel>
#include <QStringListModel>
#include <QtNetwork>
#include <QHttpHeader>
#include <QHostAddress>
#include <QHostInfo>
#include <QStringList>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QMainWindow>
#include <QFile>
#include <QTimer>
#include <QDateTime>

#include <QWidget>
#include <QWidgetAction>
#include <QThread>
class RadarScanner ;

#include "ui_walksitewndex.h"

class WalkSiteThread : public QThread
{
	Q_OBJECT
public:
	WalkSiteThread(QObject * parent = 0 ) ;
	virtual ~WalkSiteThread()  ;

	virtual void run();
	void crawlUrl() ;
	void safeDestroyLater();

public :
	//是否要停止了。
	bool isPaused ;		// = false ;
	QString mCurrUrl ;	//	当前正在处理的URL

	//所有线程列表。
	static QVector<WalkSiteThread *> mAllThreadList ;

	//the Char 是URL的状态 , R ready , E rror , D: done , C crawling
	static QHash<QString , char>  mUrlHashList;
	// char is A : accept  R : reject , 如果这里面没有的默认就是允许的了。
	static QHash<QString , char> mExtHashList ;
	//char is A : accept , R : reject 
	static QHash<QString , char > mHostHashList ; 
	static QMutex mUrlAccessMutex ;
	static QMutex mExtAccessMutex ;
	static QMutex mHostAccessMutex ;

	// 200 nn 300 nn 400 nn 
	static QHash< int , quint64> mRemoteResponseCount ;

	//
	static bool mFollowFtp ;	// = true ;
	static bool mSpanHost ;		// = true ;
	static bool mCmpTimeStamp ;	// = true ;
	static int	mMaxRecursiveDepth ;	// = 5 
	static int	mMaxCrawlSize ;		// = 1 G = 1024 * 1024 * 1024
	static int	mMaxConnection ;	// = 20	相当于20个线程。
	static int mMaxCrawlSpeed ;	//	最大限制速度。
	static QString	mStartUrl ;	//	= url set 

	//已经下载了的数据总大小。
	static quint64 mCrawledSize ;
	//下载了的文件数
	static quint64 mCrawledPageCount ;
	//下载文件保存根目录。
	static QString mSavePath ;	

	//当前速度
	static int mCurrSpeed ;
private:
	QTcpSocket * mCtrlSock ;
	QTcpSocket * mDataSock ;
private:
	QString getNextUrl();
	void parseWebPage( QString & currUrl , QString & htmlcode) ;
	bool isAllowedUrl(QString url);
	bool isNeedParsedUrl(QString url);	//是否是需要解析的文档模式。
	bool isAcceptedHost(QString url);	//是否接受的主机名字。
	bool isAcceptedExt(QString url);	//是否需要下载的扩展名。

signals:
	void crawledPage();
	void crawledData() ;
	void followFtp();
	void spanHost() ;
	void speedChanged(int) ;
	void foundLink(QString );
	void waldFinished();

};

/////////////////////////
class WalkSiteWndEx : public QWidget
{
    Q_OBJECT

public:
    WalkSiteWndEx(QWidget *parent = 0);
    virtual ~WalkSiteWndEx();
	enum { WSREADY = 0 , WSRUNNING , WSPAUSED , WSERROR };

public slots:
	void putLog(QString log);

private:
	QMenu * mMainMenu ;	//这个遍历对话框的主菜单。
	QMenu * mSettingMenu ;	//设置菜单。

	///
	WalkSiteThread * mOneThread ;
	QTimer	mCronTimer ;	//用于显示用户界面上的统计信息。
private:
	Ui::WalkSiteWndExClass ui;
	
	int		mState ;	//当前状态

private slots:
	void	onMainCustomContextMenuRequested ( const QPoint & pos ) ;
	void	onSettingButtonClicked();
	void	onShowRadarScanner(bool show) ;
	void	onOpenDirectory() ;
	void	onSaveState() ;
	void	onLoadState() ;
	void	onSelectSavePath();	//选择保存根目录
	void	onShowFloat(bool floating ); //

	/////////////
	void	onStart () ;
	void	onPause() ;
	void	onRestart() ;
	void	onTimeout();
	////
	void	onCrawledPage();
	void	onCrawledData();
	//speed in nB/s
	void	onSpeedChangeed(int speed );
	void onFoundLink(QString link);

	void setStackWidgetCurrentIndex(bool checked);

};

#endif // WALKSITEWNDEX_H
