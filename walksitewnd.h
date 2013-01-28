#ifndef WALKSITEWND_H
#define WALKSITEWND_H

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

#include "ui_walksitewnd.h"

class RadarScanner ;

/**
 * 该类准备用异步socket，练习一下异步socket的使用方法。而不再使用原来的同步阻塞socket
 *
 * 网站遍历模块。比较独立的，作为主程序的一个工具来使用。
 * 
 * 可能使用多个Socket 实例的异步调用方法，并不等于多线程阻塞socket的效果。
 * 
 */
class WalkSiteWnd : public QMainWindow
{
    Q_OBJECT

public:
    WalkSiteWnd(QWidget *parent = 0);
    ~WalkSiteWnd();

private:
    Ui::WalkSiteWndClass ui;

	QMainWindow * mMainWnd ;

	//setting vars
	QString mBaseUrl;
	QString mSavePath;
	bool mSpanHosts;
	bool mFollowFtp;
	bool mTimeStamp;
	int mRecursiveLevel ;
	bool mMaxSize ;
	int mThreadCount ;

	//

	QStringListModel * mAcceptHost;
	QStringListModel * mRejectHost;
	QStringListModel * mAcceptExt;
	QStringListModel * mRejectExt ;

	//
	QTcpSocket * mCtrlSock;
	QTcpSocket * mDataSock ;
	QStringList mReadyList;
	QStringList mDoneList;
	QStringList mFaildList ;
	//QStringList mRunningList ;

	//multi thread need the structure for store
	//QHash<QTcpSocket * , QFile * > mFileHash ;
	class rtqueue {
	public:
		QTcpSocket * mrSock;
		QFile * mrHFile ;
		int mrState ;	//0 ready , 1 running
		bool mrReadHeaderFinished ;
		QString mrUrl ;		
		QString mrHtmlCode ;
		QString mrHttpHeader ;
		
	};
	//QHash<int , QHash<QString,QString> > mRT

	rtqueue ** mRTList ;

	QMutex mRTMutex ;

	/**
	 * 当前的状态，1 正下载，2 已经完成，3 还是暂停，4 还是就绪 (恢复上次的未完成的任务的状态) , 0 是否需要等待执行状态。
	 */
	enum { WS_WAITING , WS_RUNNING , WS_DONE , WS_PAUSED , WS_READY } ;
	int mWalkState ;

	QDateTime mStartTime ;	//任务启动时间，用于计算下载速率。
	QDateTime mEndTime ;
	QDateTime mPauseTime ;

	/**
	 * 几个需要在下载过程中统计并显示的值。
	 */
	double mCurrentSpeed ;
	QString mCurrentSpeedString ;

	int mParsedHtmlPageCount ;	//一共解析了多少个HTML页面文件。
	//int mReadyQueueSize ;
	int mRunningSocketCount ;	//正在运行的socket个数。
	int mWalkPageCount ;	//已经下载了的页面个数
	int mWalkPageTotalSize ;	//已经下载了的页面大小。
	int mErrorPageCount ;		//下载过程中遇到错误而没有能正常下载下来的页面个数。

	QTimer mWalkTimer ;	//定时更新用户界面显示统计信息的计时器。

	RadarScanner * mRadar ;
	RadarScanner * mRadar2 ;

	//
	//bool mReadHeaderFinished ;
	//QString mHttpHeader ;
	//QFile * mHFile;
	//QString mHtmlCode ;
	void parseWebPage(QString & currUrl ,QString & htmlcode);
	void putLog(QString log);

	//check if it is allowed url
	bool isAllowedUrl(QString url);

	//计算速率
	double GetTaskRate( QDateTime  btime , QDateTime  etime,long gotLength  , char * buff  , double adjustTime )  ;
	double TimeDiff( QDateTime & btime , QDateTime & etime ) ;

private slots:
	void initRTList();
	void destoryRTList() ;
	/**
	 * 保存当前的状态，以更下次继续下载。
	 */
	void onSaveState();
	void onRestoreState() ;

	/**
	 * 暂时停止当前的下载
	 */
	void onPauseWalkSite();
	void onStartWalkSite() ;

	void onSelectSavePath();
	void onShowSetting(bool show);

	//新的遍历
	void onNewWalkSite() ;

	//
	void onStartRetriver();
	void onNextRetriver();
	void onWalkSiteFinished();
	
	//socket 相关操作。
	void onCtrlSockConnected() ;
	void onCtrlSockError( QAbstractSocket::SocketError socketError );
	void onCtrlSockHostFound();
	void onCtrlSockStateChanged( QAbstractSocket::SocketState socketState );
	void onCtrlSockDisconnected();

	void onCtrlSockBytesWritten(qint64 bytes);
	void onCtrlSockAboutToClose();
	void onCtrlSockReadyRead();

	//配置相关操作。
	void onAddAcceptHost();
	void onDeleteAcceptHost();
	void onAddRejectHost();
	void onDeleteRejectHost();
	void onAddAcceptExtension();
	void onDeleteAcceptExtension();
	void onAddRejectExtension();
	void onDeleteRejectExtension();

	/**
	 * 在计时器超时时更新用户界面上的统计信息。
	 */
	void onUpdateStatInfo() ;

	//
	void onShowRadarScanner(bool show);


signals:
	void nextRetriver();
	void walkSiteFinished();

	

};

#endif // WALKSITEWND_H
