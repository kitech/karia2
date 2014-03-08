// taskui.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-05 14:03:22 +0000
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "ui_karia2.h"

#include "utility.h"
#include "karia2.h"
#include "aboutdialog.h"
#include "dropzone.h"
#include "taskinfodlg.h"

#include "sqlitecategorymodel.h"
#include "sqlitestorage.h"
#include "sqlitetaskmodel.h"
#include "sqlitesegmentmodel.h"
#include "segmentlogmodel.h"
#include "taskitemdelegate.h"
#include "optionmanager.h"

#include "catmandlg.h"
#include "catpropdlg.h"
#include "preferencesdialog.h"

#include "batchjobmandlg.h"
#include "webpagelinkdlg.h"
#include "taskqueue.h"
#include "taskinfodlg.h"

#include "taskballmapwidget.h"
#include "instantspeedhistogramwnd.h"
#include "walksitewndex.h"

// #include "norwegianwoodstyle.h"
#include "torrentpeermodel.h"
#include "taskservermodel.h"
#include "seedfilemodel.h"
#include "seedfilesdialog.h"

//////
#include "libng/html-parse.h"

//labspace
#include "labspace.h"

#if !defined(Q_OS_WIN32)
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#endif

#include "simplelog.h"

//#include "ariaman.h"
//#include "maiaXmlRpcClient.h"

#include "emaria2c.h"
#include "karia2statcalc.h"

#include "metauri.h"
//#include "skype.h"
//#include "skypetunnel.h"
//#include "skypetracer.h"

#include "taskui.h"

extern QHash<QString, QString> gMimeHash;

TaskUi::TaskUi(Karia2 *pwin)
    : AbstractUi(pwin)
{
    this->mTaskMan = this->mpwin->mTaskMan;
}

TaskUi::~TaskUi()
{
}

bool TaskUi::init()
{
    QObject::connect(this->mainUI->action_New_Download, &QAction::triggered, this, &TaskUi::showNewDownloadDialog);
}


/**
 * access private
 */
int TaskUi::getNextValidTaskId()
{
	int taskId = -1;

	SqliteStorage * storage = SqliteStorage::instance(this);

    // storage->open();

	taskId = storage->getNextValidTaskID();

    if (taskId <= 0) {
        qLogx()<<"Invalid new task id:" << taskId;
        exit(-1);
    }

	return taskId;
}

/**
 * overload method
 */
int TaskUi::createTask(TaskOption *option)
{
	//precondition: 
	assert(option != 0 );

	int taskId = -1;
	
	taskId = this->getNextValidTaskId();

	//qLogx()<<this->mTaskQueue << __FUNCTION__ << "in " <<__FILE__;
	if (taskId >= 0) {
        // cacl the downloading model index
        QItemSelection readySelect, readDeselect;
        QModelIndex topCatIdx = this->mCatViewModel->index(0, 0);
        qLogx()<<topCatIdx.data();
        int l2RowCount = this->mCatViewModel->rowCount(topCatIdx);
        qLogx()<<l2RowCount;
        for (int r = 0 ; r < l2RowCount ; r ++) {
            QModelIndex currCatIdx = this->mCatViewModel->index(r, ng::cats::cat_id, topCatIdx);
            qLogx()<<currCatIdx;
            if (currCatIdx.data().toInt() == ng::cats::downloading) {
                // for (int c = 0; c <= ng::cats::dirty; c ++) {
                //     QModelIndex readyIndex = this->mCatViewModel->index(r, c, topCatIdx);
                //     readySelect.select(readyIndex, readyIndex);
                // }
                readySelect.select(this->mCatViewModel->index(r, 0, topCatIdx), 
                                   this->mCatViewModel->index(r, ng::cats::dirty, topCatIdx));
                break;
            }
        }
        this->mCatView->selectionModel()->select(readySelect, QItemSelectionModel::ClearAndSelect);
        
		////将任务信息添加到 task list view 中
		//QModelIndex index;
		//QAbstractItemModel * mdl = 0;	//SqliteTaskModel::instance(ng::cats::downloading, this);
		// TaskQueue::addTaskModel(taskId, option);
        this->mTaskMan->addTaskModel(taskId, option);
		//在这里可以查看当前活动的任务数，并看是否需要启动该任务。
		//而在onTaskDone槽中可以查看当前活动的任务数，看是否需要调度执行其他的正在等待的任务。

		// this->onStartTask(taskId);	//为了简单我们直接启动这个任务
		//		
	}		

	return taskId;
}

int TaskUi::createTask(int taskId, TaskOption *option)
{
    this->mTaskMan->addTaskModel(taskId, option);
    return taskId;
}

void TaskUi::onTaskShowColumnsChanged(QString columns)
{
    //////
    QStringList colList;
    QString taskShowColumns = columns;
    // qLogx()<<__FUNCTION__<<taskShowColumns<<(ng::tasks::aria_gid);
    if (taskShowColumns != "") {
        colList = taskShowColumns.split(',');
        for (int i = ng::tasks::task_id ; i <= ng::tasks::aria_gid; ++i) {
            // qLogx()<<"show col:"<<colList.contains(QString::number(i));
            if (!colList.contains(QString::number(i))) {
                this->mainUI->mui_tv_task_list->setColumnHidden(i, true);
            } else {
                this->mainUI->mui_tv_task_list->setColumnHidden(i, false);
            }
        }
        if (this->mCustomTaskShowColumns != columns) {
            this->mCustomTaskShowColumns = columns;
        }
    }
}

void TaskUi::onAddTaskList(QStringList list)
{
	////建立任务
	QString taskUrl = list.at(0);

	TaskOption * to = 0;	//创建任务属性。

	taskinfodlg *tid = new taskinfodlg(this->mpwin);
	tid->setTaskUrl(taskUrl);
	int er = tid->exec();
	//taskUrl = tid->taskUrl();
	//segcnt = tid->segmentCount();	
	//to = tid->getOption();	//			

	if (er == QDialog::Accepted)
	{				
		//this->createTask(url,segcnt);
		//这里先假设用户使用相同的设置。
		//delete to; to = 0;
		for (int t = 0; t <list.size(); ++ t )
		{
			taskUrl = list.at(t);
			qLogx()<<taskUrl;
			tid->setTaskUrl(taskUrl );
			to = tid->getOption();
			this->createTask(to );
		}
	}
	else
	{
		//delete to; to = 0;
	}

	delete tid;
}

QString TaskUi::decodeThunderUrl(QString enUrl)
{
    // like thunder://QUFodHRwOi8vZG93bi41MnouY29tLy9kb3duL3JhbWRpc2sgdjEuOC4yMDAgZm9yIHdpbnhwIMbGveKw5i5yYXJaWg==

    QTextCodec *u8codec = QTextCodec::codecForName("GBK");
	Q_ASSERT(u8codec != NULL);

    QByteArray bUrl = QByteArray::fromBase64(enUrl.right(enUrl.length() - 10).toLatin1());
    QString deUrl = (u8codec == NULL) ? bUrl : u8codec->toUnicode(bUrl);
    deUrl = deUrl.mid(2, deUrl.length() - 4);
    return deUrl;
}

QString TaskUi::decodeQQdlUrl(QString enUrl)
{
    // like: qqdl://aHR0cDovL3d3dy5sZXZpbC5jbg== ,  “[url]http://www.levil.cn[/url]”
    QTextCodec *u8codec = QTextCodec::codecForName("GBK");
	Q_ASSERT(u8codec != NULL);

    int pos = enUrl.indexOf("&");
    QString enArgs;
    if (pos != -1) {
        enArgs = enUrl.right(enUrl.length() - pos);
        enUrl = enUrl.left(pos);
    }
    QByteArray bUrl = QByteArray::fromBase64(enUrl.right(enUrl.length() - 7).toLatin1());
	
    QString deUrl = (u8codec == NULL) ? bUrl : u8codec->toUnicode(bUrl);
    deUrl = deUrl.mid(7, deUrl.length() - 15);
    return deUrl;
    
    return QString();
}

QString TaskUi::decodeFlashgetUrl(QString enUrl)
{
    // like flashget://W0ZMQVNIR0VUXWZ0cDovL2R5Z29kMTpkeWdvZDFAZDAzMy5keWdvZC5vcmc6MTA1OS8lRTUlQTQlQUElRTUlQjklQjMlRTYlQjQlOEIlRTYlODglOTglRTQlQkElODklNUIxMDI0JUU1JTg4JTg2JUU4JUJFJUE4JUU3JThFJTg3LiVFNCVCOCVBRCVFOCU4QiVCMSVFNSU4RiU4QyVFNSVBRCU5NyU1RC8lNUIlRTclOTQlQjUlRTUlQkQlQjElRTUlQTQlQTklRTUlQTAlODJ3d3cuZHkyMDE4Lm5ldCU1RCVFNSVBNCVBQSVFNSVCOSVCMyVFNiVCNCU4QiVFNiU4OCU5OCVFNCVCQSU4OSVFNyVBQyVBQzA0JUU5JTlCJTg2JTVCJUU0JUI4JUFEJUU4JThCJUIxJUU1JThGJThDJUU1JUFEJTk3JTVELnJtdmJbRkxBU0hHRVRd&18859&1270484198847

    QTextCodec *u8codec = QTextCodec::codecForName("GBK");
	Q_ASSERT(u8codec != NULL);

    int pos = enUrl.indexOf("&");
    QString enArgs;
    if (pos != -1) {
        enArgs = enUrl.right(enUrl.length() - pos);
        enUrl = enUrl.left(pos);
    }
    QByteArray bUrl = QByteArray::fromBase64(enUrl.right(enUrl.length() - 11).toLatin1());
	
    QString deUrl = (u8codec == NULL) ? bUrl : u8codec->toUnicode(bUrl);
    deUrl = deUrl.mid(10, deUrl.length() - 20);

    if (deUrl.toLower().startsWith("flashget://")) {
        deUrl = decodeFlashgetUrl(deUrl);
    }

    if (deUrl.toLower().startsWith("flashgetx://")) {
        qLogx()<<"Unsupported protocol type."<<deUrl;
    }
    return deUrl;

}

QString TaskUi::decodeEncodeUrl(QString enUrl)
{
    
    if (enUrl.toLower().startsWith("thunder://")) {
        return decodeThunderUrl(enUrl);
    }
    if (enUrl.toLower().startsWith("flashget://")) {
        return decodeFlashgetUrl(enUrl);
    }
    if (enUrl.toLower().startsWith("qqdl://")) {
        return decodeQQdlUrl(enUrl);
    }
    return enUrl;
}

/////////////
/**
 *
 */
void TaskUi::showNewDownloadDialog()
{
	QString url;
	int segcnt = 7;
	TaskOption * to = 0;	//创建任务属性。

	taskinfodlg *tid = new taskinfodlg(this->mpwin);
	int er = tid->exec();
	url = tid->taskUrl();
	segcnt = tid->segmentCount();	

	to = tid->getOption();	//
	delete tid;

    QString deUrl = decodeEncodeUrl(url);
	qLogx()<<"Decode url: "<<segcnt<<url<<deUrl;
    if (url != deUrl) {
        to->setUrl(deUrl);
        url = deUrl;
    }

	if (er == QDialog::Accepted)	{
        int taskId = this->createTask(to);
		qLogx()<<segcnt<<url<<taskId;
        
        //this->initXmlRpc();

        // payload type
        QMap<QString, QVariant> payload;
        payload["taskId"] = QString("%1").arg(taskId);
        payload["url"] = url;

        QVariantList args;
        QList<QVariant> uris;
        uris << QString(url);
        args.insert(0, uris);

        QMap<QString, QVariant> options ;//= this->taskOptionToAria2RpcOption(to);
        // options["split"] = QString("2");
        args.insert(1, options);

//        this->mAriaRpc->call(QString("aria2.addUri"), args, QVariant(payload),
//                  this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
//                  this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

//        if (!this->mAriaUpdater.isActive()) {
//            this->mAriaUpdater.setInterval(3000);
//            QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
//            this->mAriaUpdater.start();
//        }
        this->mEAria2Man->addUri(taskId, url, to);
	} else {
		delete to; to = 0;
	}
	qLogx()<<segcnt<<url;
}



// TODO, test with a invalid torrent file
void TaskUi::showNewBittorrentFileDialog()
{
    QString url;
    
    url = QFileDialog::getOpenFileName(this->mpwin, tr("Open a .torrent file..."), QString(),
                                       tr("Torrent File (*.torrent)"), NULL, 0);

    if (url == QString::null) {
        // cancel
    } else if (url.isEmpty()) {
        // select 0 file?
    } else {
        TaskOption *to = new TaskOption(); // TODO get option from GlobalOption
        to->setDefaultValue();
        to->mCatId = ng::cats::downloading;
        to->setUrl("file://" + url); //转换成本地文件协议

        int taskId = this->createTask(to);
		qLogx()<<__FUNCTION__<<url<<taskId;
        
        //this->initXmlRpc();

        QMap<QString, QVariant> payload;
        payload["taskId"] = taskId;
        payload["url"] = "file://" + url;
        payload["cmd"] = QString("torrent_get_files");
        // payload["taskOption"] = to->toBase64Data();

        QFile torrentFile(url);
        torrentFile.open(QIODevice::ReadOnly);
        QByteArray torrentConntent = torrentFile.readAll();
        torrentFile.close();

        QVariantList args;
        QList<QVariant> uris;

        args.insert(0, torrentConntent);
        args.insert(1, uris);

        QMap<QString, QVariant> options;
        // options["split"] = QString("1");
        options["bt-max-peers"] = QString("1");
        options["all-proxy"] = QString("127.0.0.1:65532"); // use a no usable proxy, let aria2 stop quick
        options["select-file"] = QString("1");
        options["dir"] = QDir::tempPath();
        options["file-allocation"] = "none";

        args.insert(2, options);

//        this->mAriaRpc->call(QString("aria2.addTorrent"), args, QVariant(payload),
//                  this, SLOT(onAriaParseTorrentFileResponse(QVariant &, QVariant &)),
//                  this, SLOT(onAriaParseTorrentFileFault(int, QString, QVariant &)));

        // this->mAriaRpc->call(QString("aria2.addTorrent"), args, QVariant(payload),
        //           this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
        //           this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

        // if (!this->mAriaUpdater.isActive()) {
        //     this->mAriaUpdater.setInterval(3000);
        //     QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
        //     this->mAriaUpdater.start();
        // }
        
        // if (!this->mAriaTorrentUpdater.isActive()) {
        //     this->mAriaTorrentUpdater.setInterval(4000);
        //     QObject::connect(&this->mAriaTorrentUpdater, SIGNAL(timeout()), this, SLOT(onAriaTorrentUpdaterTimeout()));
        //     this->mAriaTorrentUpdater.start();
        // }
    }
}

void TaskUi::showNewMetalinkFileDialog()
{
    QString url;
    
    url = QFileDialog::getOpenFileName(this->mpwin, tr("Open a .metalink file..."), QString(),
                                       tr("Metalink File (*.metalink)"), NULL, 0);

    if (url == QString::null) {
        // cancel
    } else if (url.isEmpty()) {
        // select 0 file?
    } else {
        TaskOption *to = new TaskOption(); // TODO get option from GlobalOption
        to->setDefaultValue();
        to->setUrl("file://" + url); //转换成本地文件协议

        int taskId = this->createTask(to);
		qLogx()<<url<<taskId;
        
        //this->initXmlRpc();

        QMap<QString, QVariant> payload;
        payload["taskId"] = taskId;
        payload["url"] = "file://" + url;

        QFile metalinkFile(url);
        metalinkFile.open(QIODevice::ReadOnly);
        QByteArray metalinkConntent = metalinkFile.readAll();
        metalinkFile.close();

        QVariantList args;

        args.insert(0, metalinkConntent);

        QMap<QString, QVariant> options;
        options["split"] = QString("5");
        args.insert(1, options);

//        this->mAriaRpc->call(QString("aria2.addMetalink"), args, QVariant(payload),
//                  this, SLOT(onAriaAddUriResponse(QVariant &, QVariant &)),
//                  this, SLOT(onAriaAddUriFault(int, QString, QVariant &)));

//        if (!this->mAriaUpdater.isActive()) {
//            this->mAriaUpdater.setInterval(3000);
//            QObject::connect(&this->mAriaUpdater, SIGNAL(timeout()), this, SLOT(onAriaUpdaterTimeout()));
//            this->mAriaUpdater.start();
//        }
        
//        if (!this->mAriaTorrentUpdater.isActive()) {
//            this->mAriaTorrentUpdater.setInterval(4000);
//            QObject::connect(&this->mAriaTorrentUpdater, SIGNAL(timeout()), this, SLOT(onAriaTorrentUpdaterTimeout()));
//            this->mAriaTorrentUpdater.start();
//        }
    }
}

void TaskUi::showBatchDownloadDialog()
{
	QString url;
	int segcnt = 7;
	TaskOption * to = 0;	//创建任务属性。
	QStringList sl;
	
	BatchJobManDlg *bjd = new BatchJobManDlg(this->mpwin);
	int er = bjd->exec();
	sl = bjd->getUrlList();

	delete bjd;

	if (er == QDialog::Accepted) {		
		qLogx()<<segcnt<<url;
		taskinfodlg *tid = new taskinfodlg(this->mpwin);
		for (int i = 0; i < sl.count(); ++i) {
			tid->setTaskUrl(sl.at(i));
			to = tid->getOption();
			this->createTask(to );
		}
		delete tid;	
	}
	else
	{
		delete to; to = 0;
	}	
}

void TaskUi::showProcessWebPageInputDiglog()	//处理WEB页面，取其中链接并下载
{
	QStringList urlList , srcList ,resultList;
	QString url;
	QUrl u;
	QString htmlcode;
	int er = QDialog::Rejected;
	int ulcount;
	int linkCount = 0;
	char ** linkList = 0;

	WebPageUrlInputDlg * wpid = 0;
	WebPageLinkDlg * wpld  = 0;

	wpid = new WebPageUrlInputDlg(this->mpwin);
	er = wpid->exec();
	if (er == QDialog::Accepted )
	{
		url = wpid->getLinkList();
		urlList = url.split("\"+\"");
		ulcount = urlList.size();
		for (int i = 0; i < ulcount; ++i)
		{
			if (i == 0 && urlList.at(0).startsWith("\""))
			{
				urlList[0] = urlList.at(0).right(urlList.at(0).length()-1);
			}
			if (i == ulcount-1 && urlList.at(i).endsWith("\""))
			{
				urlList[i] = urlList.at(i).left(urlList.at(i).length()-1);
			}
		}
		qLogx()<<urlList;

		if (urlList.size() > 0 )
		{
			for (int i = 0; i < ulcount; ++i)
			{
				u = urlList.at(i);
				if (u.scheme().toUpper().compare("HTTP") == 0
					|| u.scheme().toUpper().compare("HTTPS") == 0 )
				{
					//是否需要我们自己从HTTP服务器读取文件内容呢。
					//windows 不需要.UNIX的各资源管理器还不清楚是否支持打开HTTP的功能。

				}
				else if (u.scheme().toUpper().length() == 1 )	// C D E and so on	for win , / for unix
				{
					QFile f(urlList.at(0));
					f.open(QIODevice::ReadOnly);
					htmlcode = QString(f.readAll().data());
					f.close();
				}
				else
				{
					//not supported page file access way
				}
				//test
				linkCount = 0;
				linkList = html_parse_get_all_link(htmlcode.toLatin1().data() , &linkCount );
				qLogx()<<linkCount;
				for (int j = 0; j < linkCount; j ++ )
				{
					srcList.append(QString(linkList[j]));
				}
				html_parse_free_link_mt(linkList,linkCount);
			}	//end for (int i = 0; i < ulcount; ++i)
		}

		//////
		if (srcList.size() > 0 )
		{
			wpld = new WebPageLinkDlg(this->mpwin);
			wpld->setSourceUrlList(srcList);
			if (wpld->exec() == QDialog::Accepted)
			{
				resultList = wpld->getResultUrlList();
				qLogx()<<resultList;

			}
			delete wpld; wpld = 0;
		}
		else // no url found in user input webpage file
		{
			QMessageBox::warning(this->mpwin, tr("Process Weg Page :"), tr("No URL(s) Found In the Weg Page File/Given URL"));
		}
		
		///建立新任务。
		if (resultList.size() > 0 )
		{
			////建立任务
			QString taskUrl = resultList.at(0);

			TaskOption * to = 0;	//创建任务属性。

			taskinfodlg *tid = new taskinfodlg(this->mpwin);
			tid->setTaskUrl(taskUrl);
			int er = tid->exec();
			//taskUrl = tid->taskUrl();
			//segcnt = tid->segmentCount();	
			//to = tid->getOption();	//			

			if (er == QDialog::Accepted)
			{				
				//this->createTask(url,segcnt);
				//这里先假设用户使用相同的设置。
				//delete to; to = 0;
				for (int t = 0; t < resultList.size(); ++ t )
				{
					taskUrl = resultList.at(t);
					qLogx()<<taskUrl;
					tid->setTaskUrl(taskUrl );
					to = tid->getOption();
					this->createTask(to );
				}
			}
			else
			{
				//delete to; to = 0;
			}

			delete tid;
		}	//end if (resultList.size() > 0 )
		else
		{
			QMessageBox::warning(this->mpwin, tr("Process Weg Page :"), tr("No URL(s) Selected"));
		}	//end if (resultList.size() > 0 ) else 
	}

	if (wpid != 0 ) delete wpid;
	if (wpld != 0 ) delete wpld;
}

// void TaskUi::onShowDefaultDownloadProperty()
// {
// 	taskinfodlg *tid = new taskinfodlg(0,0);		
	
// 	//tid->setRename(fname);			
// 	int er = tid->exec();	
	
// 	delete tid;	
// }

QPair<QString, QString> TaskUi::getFileTypeByFileName(QString fileName)
{
	QPair<QString,QString> ftype("unknown", "unknown.png");

    // if (fileName.isEmpty()) {
    //     return ftype;
    // }

#if defined(Q_OS_WIN)
	QString path = "icons/crystalsvg/128x128/mimetypes/";
#elif defined(Q_OS_MAC)
    QString path = "icons/crystalsvg/128x128/mimetypes/";
#else // *nix
    QString path = QString("/usr/share/icons/%1/128x128/mimetypes/").arg(QIcon::themeName());
    if (!QDir().exists(path)) {
        path = QString("/usr/share/icons/gnome/32x32/mimetypes/gnome-mime-");
    }
#endif
	QStringList fileParts = fileName.toLower().split(".");
    QString fileExtName = fileParts.at(fileParts.count() - 1);
    
    ftype.first = fileExtName;
    if (gMimeHash.contains(fileExtName)) {
        ftype.second = path + gMimeHash.value(fileExtName) + ".png";
    } else {
        ftype.second = path + ftype.second;
    }

    // ("/usr/share/icons", "/usr/local/share/icons", "/usr/share/icons", ":/home/gzleo/.kde4/share/icons", ":/icons")
    // qLogx()<<QIcon::themeSearchPaths();
    
    return ftype;
}

void TaskUi::onShowTaskProperty()
{
	qLogx()<<__FUNCTION__;
	// QItemSelectionModel * sim;
	TaskQueue *tq = NULL;
	QModelIndexList mil;

	//在运行队列中查找。
	mil  = this->mTaskListView->selectionModel()->selectedIndexes();
	if (this->mTaskListView->model() != 0 &&
		mil.size() == this->mTaskListView->model()->columnCount()) {
		//only selected one ,可以处理，其他多选的不予处理。
		int taskId = mil.at(0).data().toInt();
        if (taskId <= 0) {
        }
		// tq = this->mTaskMan->findTaskById(taskId);
		if (tq == 0) {
			// tq = this->mTaskMan->findTaskById(taskId);
		}
	}

	//根据不同的位置显示不同的任务属性。
    if (tq != 0) {
        // QString url = tq->mTaskUrl;
        // int segcnt = tq->mTotalSegmentCount;
        QString url;
        int segcnt = 5;
        QString fname;//= tq->mTaskOption->mSaveName;
		
        taskinfodlg *tid = new taskinfodlg(0, this->mpwin);
        tid->setTaskUrl(url);
        tid->setSegmentCount(segcnt);
        tid->setRename(fname);
		
        int er = tid->exec();
        Q_UNUSED(er);
		
        delete tid;
    }
}
void TaskUi::onShowTaskProperty(int pTaskId)
{
    Q_UNUSED(pTaskId);
}

void TaskUi::onShowTaskPropertyDigest(const QModelIndex & index )
{
	//qLogx()<<__FUNCTION__ << index.data();
	// int taskId;
	this->mSwapPoint = QCursor::pos();
	this->mSwapModelIndex = index;
	QTimer::singleShot(1000, this, SLOT(onShowTaskPropertyDigest()));
}

void TaskUi::onShowTaskPropertyDigest( )
{
	//qLogx()<<__FUNCTION__;
	QString tips = tr("<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\" white-space: pre-wrap; font-family:宋体; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;\"><table  width=\"100%\"  height=\"100%\" border=\"1\">  <tr>    <td width=\"97\">&nbsp;<img name=\"\" src=\"%1\" width=\"80\" height=\"80\" alt=\"\"></td>    <td  height=\"100%\" ><b>%2</b><br>-------------------------------<br>File Size: %3<br>File Type: .%4<br>Completed: %5<br>-------------------------------<br>Save Postion: %6<br>URL: %7<br>Refferer: %8<br>Comment: %9<br>-------------------------------<br>Create Time: %10<br>------------------------------- </td>  </tr></table></body></html>");

	QPoint np = this->mainUI->mui_tv_task_list->viewport()->mapFromGlobal(QCursor::pos());
	QModelIndex nidx = this->mainUI->mui_tv_task_list->indexAt(np);
	QModelIndex tidx;
	// TaskQueue * tq = 0;
	// int catId = -1;
	
	if (this->mainUI->mui_tv_task_list->viewport()->underMouse()&&
		this->mSwapModelIndex.isValid() && nidx.isValid() && nidx == this->mSwapModelIndex ) {
		int row = nidx.row();
		QString id , url , ftype, name , path , gotlength , totallength , refferer, comment ,createtime  , fimg;
		tidx = nidx.model()->index(row,0);
		id = tidx.data().toString();
		//tq = TaskQueue::findTaskById(id.toInt());
		//if (tq != 0   )
		//{
		//	//path = tq->mTaskOption->mSavePath;
		//}
		//else
		//{
		//	path = ".";
		//}
		tidx = nidx.model()->index(row,ng::tasks::user_cat_id);
		path = SqliteStorage::instance(this)->getSavePathByCatId(tidx.data().toInt());

		tidx = nidx.model()->index(row, ng::tasks::org_url);
		url = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::file_name);
		name = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::abtained_length);
		gotlength = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::file_size);
		totallength = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::referer);
		refferer = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::comment);
		comment = tidx.data().toString();
		tidx = nidx.model()->index(row, ng::tasks::create_time);
		createtime = tidx.data().toString();

		//ftype = "unknown";
		//fimg = "cubelogo.png";
		QPair<QString , QString> guessedFileType = this->getFileTypeByFileName(name);
		ftype = guessedFileType.first;
		fimg = guessedFileType.second;

		//qLogx()<<"show digest"<<this->mSwapModelIndex.data();
		//QToolTip * tt = new QToolTip();
		tips = tips.arg(fimg);
		tips = tips.arg(name);
		tips = tips.arg(totallength);
		tips = tips.arg(ftype);		//get file type by file name , or get file type by file content , later 
		tips = tips.arg(gotlength);
		tips = tips.arg(path);
		tips = tips.arg(url);
		tips = tips.arg(refferer);
		tips = tips.arg(comment);
		tips = tips.arg(createtime);
		QToolTip::showText(QCursor::pos(),tips );
		//delete tt;
	}
	this->mSwapPoint = QPoint(0,0);
	this->mSwapModelIndex = QModelIndex();	
}


void TaskUi::onSegmentListSelectChange(const QItemSelection & selected, const QItemSelection & deselected )
{
	qLogx()<<__FUNCTION__;
    Q_UNUSED(deselected);

	int taskId;
	int segId;
	QString segName;

	QModelIndexList mil = selected.indexes();

	taskId = mil.at(1).data().toInt();
	segId = mil.at(0).data().toInt();
	segName = mil.at(2).data().toString();
	qLogx()<<taskId<<segName<<segId;

	//seach log model by taskid and seg id 

	QAbstractItemModel * mdl = SegmentLogModel::instance(taskId , segId, this);

	this->mSegLogListView->setModel(0);
	if (mdl != 0 ) {
		this->mSegLogListView->setModel(mdl);
		this->mSegLogListView->resizeColumnToContents(2);
		this->mSegLogListView->scrollToBottom();
	} else {
		qLogx()<<__FUNCTION__<<" model mSegLogListView = 0 ";
	}
}

void TaskUi::onTaskListSelectChange(const QItemSelection & selected, const QItemSelection & deselected)
{
    Q_UNUSED(deselected);

	int taskId;
	// int segId;
	QString segName;

	//更新JOB相关菜单enable状态。
	this->onUpdateJobMenuEnableProperty();

	///
	QModelIndexList mil = selected.indexes();

	if (selected.size() == 0 ) {
		//不能简单的退出，而应该做点什么吧， 看看它的表现 。
		//在没有任务选中的时候，是否应该清除线程列表及线程日志中的内容。
		return;	//no op needed 
	}

	taskId = mil.at(ng::tasks::task_id).data().toInt();

	qLogx()<<__FUNCTION__<<taskId<<segName;

	QModelIndex index;
	QAbstractItemModel *mdl = 0;

	// update task ball and peer view
    if (this->mTaskMan->isTorrentTask(taskId)) {
        mdl = this->mTaskMan->torrentPeerModel(taskId);
        this->mainUI->peersView->setModel(0);
        this->mainUI->peersView->setModel(mdl);
        mdl = this->mTaskMan->torrentTrackerModel(taskId);
        this->mainUI->trackersView->setModel(mdl);
        mdl = this->mTaskMan->taskSeedFileModel(taskId);
        this->mSeedFileView->setModel(mdl);
        QWidget *w = this->mSeedFileView->cornerWidget();
        if (w == NULL) {
            w = new QToolButton();
            static_cast<QToolButton*>(w)->setText("");
            this->mSeedFileView->setCornerWidget(w);
        }
        QObject::connect(mdl, SIGNAL(reselectFile(int, bool)), this, SLOT(onReselectFile(int, bool)));
    } else {
        mdl = this->mTaskMan->torrentPeerModel(taskId);
        this->mainUI->peersView->setModel(0);
        this->mainUI->peersView->setModel(mdl);
        mdl = this->mTaskMan->torrentTrackerModel(taskId);
        this->mainUI->trackersView->setModel(mdl);
        mdl = this->mTaskMan->taskSeedFileModel(taskId);
        this->mSeedFileView->setModel(mdl);
    }
    {
        mdl = this->mTaskMan->taskServerModel(taskId);
        this->mSegListView->setModel(mdl);
    }

    qLogx()<<__FUNCTION__<<"Ball Ball"<<taskId<<mdl<<(mdl ? mdl->rowCount() : 0);
    TaskBallMapWidget::instance()->onRunTaskCompleteState(taskId, true);

    {
        // update task summary view
        QString fileName = mil.at(ng::tasks::file_name).data().toString();
        QString fileSize = mil.at(ng::tasks::file_size).data().toString();
        QString speed = mil.at(ng::tasks::average_speed).data().toString();
        QString blocks = mil.at(ng::tasks::block_activity).data().toString();
        QString savePath = mil.at(ng::tasks::save_path).data().toString();
        QString refer = mil.at(ng::tasks::referer).data().toString();
        QPair<QString, QString> mimeIconPosition = this->getFileTypeByFileName(fileName);

        this->mainUI->label_2->setText(fileName);
        this->mainUI->label_2->setToolTip(fileName);
        this->mainUI->label_3->setText(fileSize);
        this->mainUI->label_5->setText(speed);
        this->mainUI->label_7->setText(blocks);
        this->mainUI->label_9->setText(QString("<a href=\"%1\">%1</a>").arg(savePath));
        this->mainUI->label_9->setToolTip(QString(tr("Location: %1")).arg(savePath));

        // calc how much charactors the label can show
        QFontInfo fontInfo = this->mainUI->label_11->fontInfo();
        int laWidth = this->mainUI->mui_tw_segment_graph_log->width();
        int fpSize = fontInfo.pixelSize();
        // int fppSize = fontInfo.pointSize();
        int chcnt = laWidth * 2/ fpSize;
        // qLogx()<<"calc chcnt:"<<laWidth<<fpSize<<chcnt<<fppSize<<refer.length();

        if (refer.length() > chcnt) {
            this->mainUI->label_11->setText(QString("<a href=\"%1\">%2</a>")
                                           .arg(refer, refer.left(chcnt) + "..."));
        } else {
            this->mainUI->label_11->setText(QString("<a href=\"%1\">%1</a>").arg(refer));
        }
        this->mainUI->label_11->setToolTip(QString(tr("Location: %1")).arg(refer));

        if (fileName.isEmpty()) {
            QString dir = qApp->applicationDirPath() + "/icons";
            this->mainUI->label->setPixmap(QPixmap(dir + "/status/unknown.png").scaled(32, 32));
        } else {
            this->mainUI->label->setPixmap(QPixmap(mimeIconPosition.second).scaled(32, 32));
        }
    }
}

void TaskUi::onCatListSelectChange(const QItemSelection &selected, const QItemSelection &deselected)
{
	qLogx()<<__FUNCTION__<<selected;
	
    // has case selected.size() > 1
	if (selected.size() != 1) {
        return;
    }

    QModelIndex currentIndex;
    currentIndex = selected.at(0).indexes().at(0);
    qLogx()<<currentIndex;
    QModelIndex catIDIndex = selected.at(0).indexes().at(ng::cats::cat_id);
    int catID = catIDIndex.model()->data(catIDIndex ).toInt();

    // qLogx()<<"My cat id is: "<<catIDIndex.model()->data(catIDIndex).toString();
    this->mTaskListView->setModel(0);

    if (catIDIndex.model()->data(catIDIndex).toString() == "0") {
        //this->mCatView->clearSelection(); // 不让选择树根也不合适啊。
        QAbstractItemModel *mdl = SqliteTaskModel::instance(catID, this);
        this->mTaskListView->setModel(mdl);
    } else {
        QAbstractItemModel *mdl = SqliteTaskModel::instance(catID, this);
        this->mTaskListView->setModel(mdl);
    }

    QObject::connect(this->mTaskListView->selectionModel(), 
                     SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection &)),
                     this, SLOT(onTaskListSelectChange(const QItemSelection &, const QItemSelection &)));
    
    // qLogx()<<"colums should show:"<<this->mCustomTaskShowColumns;
    if (!this->mCustomTaskShowColumns.isEmpty()) {
        this->onTaskShowColumnsChanged(this->mCustomTaskShowColumns);
    }

    // clean up
    // qLogx()<<deselected;
    // qLogx()<<deselected.count();
    // qDebug<<deselected.at(0); //.indexes();
    if (deselected.count() > 0) {
        QModelIndex deCatIdIndex = deselected.at(0).indexes().at(ng::cats::cat_id);
        if (deCatIdIndex.data().toInt() == ng::cats::downloading
            || deCatIdIndex.data().toInt() == ng::cats::deleted) {
        } else {
            int deCatId = deCatIdIndex.model()->data(deCatIdIndex).toInt();
            SqliteTaskModel::removeInstance(deCatId);
        }
    }
	
}

void TaskUi::onTaskListMenuPopup(/* const QPoint & pos */) 
{
	//qLogx()<<__FUNCTION__;

	//将菜单项进行使合适的变灰
	QModelIndex idx;
	QString assumeCategory = "Download";	
	if (this->mCatView->selectionModel()->selectedIndexes().size() == 0 ) {
	} else if (this->mCatView->selectionModel()->selectedIndexes().size() > 0 ) {
		idx = this->mCatView->selectionModel()->selectedIndexes().at(0);
		if (idx.data().toString().compare("Download")!=0
			&& idx.data().toString().compare("Downloaded")!=0
			&& idx.data().toString().compare("Deleted")!=0	) {
			
		} else {
			assumeCategory = idx.data().toString();
		}
	}
	if (assumeCategory.compare("Downloaded") == 0 ) {
		this->mainUI->action_Start->setEnabled(false);
		this->mainUI->action_Pause->setEnabled(false);
		this->mainUI->action_Schedule->setEnabled(false);
		this->mainUI->action_Delete_task->setEnabled(false);
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0) {
			//this->mainUI->action_Start->setEnabled(true);
			//this->mainUI->action_Pause->setEnabled(true);
			//this->mainUI->action_Schedule->setEnabled(true);
			//this->mainUI->action_Delete_task->setEnabled(true);
			this->mainUI->action_Properties->setEnabled(true);
			this->mainUI->action_Comment->setEnabled(true);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI->action_Browse_Referer->setEnabled(true);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI->actionMove_Down->setEnabled(true);
			this->mainUI->actionMove_Up->setEnabled(true);
			this->mainUI->action_Move_bottom->setEnabled(true);
			this->mainUI->action_Move_top->setEnabled(true);
			this->mainUI->action_Site_Properties->setEnabled(true);
		} else {
			//this->mainUI->action_Start->setEnabled(false);
			//this->mainUI->action_Pause->setEnabled(false);
			//this->mainUI->action_Schedule->setEnabled(false);
			//this->mainUI->action_Delete_task->setEnabled(false);
			this->mainUI->action_Properties->setEnabled(false);
			this->mainUI->action_Comment->setEnabled(false);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI->action_Browse_Referer->setEnabled(false);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI->actionMove_Down->setEnabled(false);
			this->mainUI->actionMove_Up->setEnabled(false);
			this->mainUI->action_Move_bottom->setEnabled(false);
			this->mainUI->action_Move_top->setEnabled(false);
			this->mainUI->action_Site_Properties->setEnabled(false);
		}
	} else if (assumeCategory.compare("Deleted") == 0 ) {
		this->mainUI->action_Start->setEnabled(false);
		this->mainUI->action_Pause->setEnabled(false);
		this->mainUI->action_Schedule->setEnabled(false);
		this->mainUI->action_Delete_task->setEnabled(false);
		if (this->mTaskListView->selectionModel()->selectedIndexes().size() > 0) {
			//this->mainUI->action_Start->setEnabled(true);
			//this->mainUI->action_Pause->setEnabled(true);
			//this->mainUI->action_Schedule->setEnabled(true);
			//this->mainUI->action_Delete_task->setEnabled(true);
			this->mainUI->action_Properties->setEnabled(true);
			this->mainUI->action_Comment->setEnabled(true);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI->action_Browse_Referer->setEnabled(true);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI->actionMove_Down->setEnabled(true);
			this->mainUI->actionMove_Up->setEnabled(true);
			this->mainUI->action_Move_bottom->setEnabled(true);
			this->mainUI->action_Move_top->setEnabled(true);
			this->mainUI->action_Site_Properties->setEnabled(true);
		} else {
			//this->mainUI->action_Start->setEnabled(false);
			//this->mainUI->action_Pause->setEnabled(false);
			//this->mainUI->action_Schedule->setEnabled(false);
			//this->mainUI->action_Delete_task->setEnabled(false);
			this->mainUI->action_Properties->setEnabled(false);
			this->mainUI->action_Comment->setEnabled(false);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI->action_Browse_Referer->setEnabled(false);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI->actionMove_Down->setEnabled(false);
			this->mainUI->actionMove_Up->setEnabled(false);
			this->mainUI->action_Move_bottom->setEnabled(false);
			this->mainUI->action_Move_top->setEnabled(false);
			this->mainUI->action_Site_Properties->setEnabled(false);
		}
	} else {//if (assumeCategory.compare("Download") == 0 )
		if (this->mTaskListView->selectionModel()->selectedIndexes().size() > 0) {
			this->mainUI->action_Start->setEnabled(true);
			this->mainUI->action_Pause->setEnabled(true);
			this->mainUI->action_Schedule->setEnabled(true);
			this->mainUI->action_Delete_task->setEnabled(true);
			this->mainUI->action_Properties->setEnabled(true);
			this->mainUI->action_Comment->setEnabled(true);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI->action_Browse_Referer->setEnabled(true);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI->actionMove_Down->setEnabled(true);
			this->mainUI->actionMove_Up->setEnabled(true);
			this->mainUI->action_Move_bottom->setEnabled(true);
			this->mainUI->action_Move_top->setEnabled(true);
			this->mainUI->action_Site_Properties->setEnabled(true);
		} else {
			this->mainUI->action_Start->setEnabled(false);
			this->mainUI->action_Pause->setEnabled(false);
			this->mainUI->action_Schedule->setEnabled(false);
			this->mainUI->action_Delete_task->setEnabled(false);
			this->mainUI->action_Properties->setEnabled(false);
			this->mainUI->action_Comment->setEnabled(false);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI->action_Browse_Referer->setEnabled(false);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI->actionMove_Down->setEnabled(false);
			this->mainUI->actionMove_Up->setEnabled(false);
			this->mainUI->action_Move_bottom->setEnabled(false);
			this->mainUI->action_Move_top->setEnabled(false);
			this->mainUI->action_Site_Properties->setEnabled(false);
		}
	}
	
	if (this->mTaskListView == sender() ) {
		this->mTaskPopupMenu->popup(QCursor::pos());
	} else {
		//不是任务视图控件发送来的。
	}
}

void TaskUi::onUpdateJobMenuEnableProperty() 
{
	qLogx()<<__FUNCTION__;

	//将菜单项进行使合适的变灰
	QModelIndex idx;
	// QString assumeCategory = "Download";
    int catId = ng::cats::downloading;
	
	if (this->mCatView->selectionModel()->selectedIndexes().size() == 0) {

	} else if (this->mCatView->selectionModel()->selectedIndexes().size() > 0) {
		idx = this->mCatView->selectionModel()->selectedIndexes().at(ng::cats::cat_id);
        catId = idx.data().toInt();
		// if (idx.data().toString().compare("Download")!=0
		// 	&& idx.data().toString().compare("Downloaded")!=0
		// 	&& idx.data().toString().compare("Deleted")!=0	) {
			
		// } else {
		// 	assumeCategory = idx.data().toString();
		// }
	}

	if (catId == ng::cats::downloaded) { // (assumeCategory.compare("Downloaded") == 0 ) {
		this->mainUI->action_Start->setEnabled(false);
		this->mainUI->action_Pause->setEnabled(false);
		this->mainUI->action_Schedule->setEnabled(false);
		this->mainUI->action_Delete_task->setEnabled(false);
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0) {
			//this->mainUI->action_Start->setEnabled(true);
			//this->mainUI->action_Pause->setEnabled(true);
			//this->mainUI->action_Schedule->setEnabled(true);
			this->mainUI->action_Delete_task->setEnabled(true);
			this->mainUI->action_Properties->setEnabled(true);
			this->mainUI->action_Comment->setEnabled(true);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI->action_Browse_Referer->setEnabled(true);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI->actionMove_Down->setEnabled(true);
			this->mainUI->actionMove_Up->setEnabled(true);
			this->mainUI->action_Move_bottom->setEnabled(true);
			this->mainUI->action_Move_top->setEnabled(true);
			this->mainUI->action_Site_Properties->setEnabled(true);
		} else {
			//this->mainUI->action_Start->setEnabled(false);
			//this->mainUI->action_Pause->setEnabled(false);
			//this->mainUI->action_Schedule->setEnabled(false);
			//this->mainUI->action_Delete_task->setEnabled(false);
			this->mainUI->action_Properties->setEnabled(false);
			this->mainUI->action_Comment->setEnabled(false);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI->action_Browse_Referer->setEnabled(false);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI->actionMove_Down->setEnabled(false);
			this->mainUI->actionMove_Up->setEnabled(false);
			this->mainUI->action_Move_bottom->setEnabled(false);
			this->mainUI->action_Move_top->setEnabled(false);
			this->mainUI->action_Site_Properties->setEnabled(false);
		}
	} else if (catId == ng::cats::deleted) { // (assumeCategory.compare("Deleted") == 0 )
		this->mainUI->action_Start->setEnabled(false);
		this->mainUI->action_Pause->setEnabled(false);
		this->mainUI->action_Schedule->setEnabled(false);
		this->mainUI->action_Delete_task->setEnabled(false);
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0) {
			//this->mainUI->action_Start->setEnabled(true);
			//this->mainUI->action_Pause->setEnabled(true);
			//this->mainUI->action_Schedule->setEnabled(true);
			//this->mainUI->action_Delete_task->setEnabled(true);
			this->mainUI->action_Properties->setEnabled(true);
			this->mainUI->action_Comment->setEnabled(true);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI->action_Browse_Referer->setEnabled(true);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI->actionMove_Down->setEnabled(true);
			this->mainUI->actionMove_Up->setEnabled(true);
			this->mainUI->action_Move_bottom->setEnabled(true);
			this->mainUI->action_Move_top->setEnabled(true);
			this->mainUI->action_Site_Properties->setEnabled(true);
		} else {
			//this->mainUI->action_Start->setEnabled(false);
			//this->mainUI->action_Pause->setEnabled(false);
			//this->mainUI->action_Schedule->setEnabled(false);
			//this->mainUI->action_Delete_task->setEnabled(false);
			this->mainUI->action_Properties->setEnabled(false);
			this->mainUI->action_Comment->setEnabled(false);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI->action_Browse_Referer->setEnabled(false);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI->actionMove_Down->setEnabled(false);
			this->mainUI->actionMove_Up->setEnabled(false);
			this->mainUI->action_Move_bottom->setEnabled(false);
			this->mainUI->action_Move_top->setEnabled(false);
			this->mainUI->action_Site_Properties->setEnabled(false);
		}
	}
	else //if (assumeCategory.compare("Downloading") == 0 )
	{
		if (this->mTaskListView->selectionModel()->selectedIndexes().size()>0) {
			this->mainUI->action_Start->setEnabled(true);
			this->mainUI->action_Pause->setEnabled(true);
			this->mainUI->action_Schedule->setEnabled(true);
			this->mainUI->action_Delete_task->setEnabled(true);
			this->mainUI->action_Properties->setEnabled(true);
			this->mainUI->action_Comment->setEnabled(true);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(true);
			this->mainUI->action_Browse_Referer->setEnabled(true);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(true);
			this->mainUI->actionMove_Down->setEnabled(true);
			this->mainUI->actionMove_Up->setEnabled(true);
			this->mainUI->action_Move_bottom->setEnabled(true);
			this->mainUI->action_Move_top->setEnabled(true);
			this->mainUI->action_Site_Properties->setEnabled(true);
		} else {
			this->mainUI->action_Start->setEnabled(false);
			this->mainUI->action_Pause->setEnabled(false);
			this->mainUI->action_Schedule->setEnabled(false);
			this->mainUI->action_Delete_task->setEnabled(false);
			this->mainUI->action_Properties->setEnabled(false);
			this->mainUI->action_Comment->setEnabled(false);
			this->mainUI->action_Copy_URL_To_ClipBoard->setEnabled(false);
			this->mainUI->action_Browse_Referer->setEnabled(false);
			this->mainUI->action_Browse_With_Site_Explorer->setEnabled(false);
			this->mainUI->actionMove_Down->setEnabled(false);
			this->mainUI->actionMove_Up->setEnabled(false);
			this->mainUI->action_Move_bottom->setEnabled(false);
			this->mainUI->action_Move_top->setEnabled(false);
			this->mainUI->action_Site_Properties->setEnabled(false);
		}
	}
	
}

void TaskUi::onLogListMenuPopup(const QPoint & pos ) 
{
    Q_UNUSED(pos);
	this->mLogPopupMenu->popup(QCursor::pos());
}
void TaskUi::onSegListMenuPopup(const QPoint & pos) 
{
	// this->mSegmentPopupMenu->popup(QCursor::pos());
    Q_UNUSED(pos);
}
void TaskUi::onCateMenuPopup(const QPoint & pos)
{
    Q_UNUSED(pos);
	this->mCatPopupMenu->popup(QCursor::pos());
}


void TaskUi::onStorageOpened()
{
    this->mCatView = this->mainUI->mui_tv_category;

    ////
    this->mpwin->mTaskListView = this->mainUI->mui_tv_task_list;
    this->mTaskListView = this->mainUI->mui_tv_task_list;
    this->mTaskItemDelegate = new TaskItemDelegate();
    this->mTaskListView->setItemDelegate(this->mTaskItemDelegate);
    this->mTaskTreeViewModel = SqliteTaskModel::instance(ng::cats::downloading, this);
    this->mTaskListView->setModel(this->mTaskTreeViewModel);
    this->mTaskListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->mTaskListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->mTaskListView->setAlternatingRowColors(true);
    // QObject::connect(this->mTaskListView->selectionModel(),
    //                  SIGNAL(selectionChanged (const QItemSelection & , const QItemSelection &   )),
    //                  this, SLOT(onTaskListSelectChange(const QItemSelection & , const QItemSelection &   ) ) );
}
