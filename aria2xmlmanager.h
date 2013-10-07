// aria2xmlmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-27 21:55:36 +0000
// Version: $Id$
// 
#ifndef _ARIA2XMLRPCMANAGER_H_
#define _ARIA2XMLRPCMANAGER_H_

#include <memory>
#include <QtNetwork>

#include "aria2rpcmanager.h"

class Karia2StatCalc;
class MaiaXmlRpcClient;


class Aria2XmlManager : public Aria2RpcManager
{
    Q_OBJECT;
public:
    Aria2XmlManager();
    virtual ~Aria2XmlManager();

    virtual void run();

public slots:
    virtual int addTask(int task_id, const QString &url, TaskOption *to);
    // virtual int removeTask(int task_id)  = 0;
    // virtual int startTask(int task_id)  = 0;
    // virtual int stopTask(int task_id)  = 0;
    virtual int pauseTask(int task_id);
    /////
    virtual bool onAllStatArrived(int stkey);
    virtual bool setSpeedLimit(int downloadSpeed, int uploadSpeed) {return true;};

/**
 * 实现下载状态信息的暂存
 * 实现下载状态状态的合并
 * 实现下载状态拆分发出
 * 实现三角通信的一个节点，另两个是GUI和aria2实例
 */
public:
    bool checkAndDispatchStat(Aria2StatCollector *sclt);
    bool checkAndDispatchServerStat(Aria2StatCollector *sclt);
    bool confirmBackendFinished(int tid, void *);

public slots:
    void onAriaGetFeatureResponse(QVariant &response, QNetworkReply *, QVariant &payload);
    void onAriaGetFeatureFault(int code, QString reason, QNetworkReply *, QVariant &payload);
    void onAriaAddUriResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaAddUriFault(int, QString, QNetworkReply *reply, QVariant &payload);
    void onAriaGetUriResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaGetUriFault(int code, QString reason, QNetworkReply *, QVariant &payload);
    void onAriaGetStatusResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaGetStatusFault(int code, QString reason, QNetworkReply *, QVariant &payload);

    void onAriaUpdaterTimeout();
    void onAriaGlobalUpdaterTimeout();

    void onAriaGetActiveResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaGetActiveFault(int code, QString reason, QNetworkReply *, QVariant &payload);

    void onAriaGetServersResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaGetServersFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);
    void onAriaRemoveResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaRemoveFault(int code, QString reason, QNetworkReply *, QVariant &payload);
    void onAriaRemoveTorrentParseFileTaskResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaRemoveTorrentParseFileTaskFault(int code, QString reason, QNetworkReply *, QVariant &payload);
    void onAriaRemoveGetTorrentFilesConfirmResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaRemoveGetTorrentFilesConfirmFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);
    void onTorrentRemoveConfirmTimeout();

    void onAriaGetTorrentPeersResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaGetTorrentPeersFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);
    void onAriaParseTorrentFileResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaParseTorrentFileFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);
    void onAriaGetTorrentFilesResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaGetTorrentFilesFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);
    void onAriaTorrentUpdaterTimeout();

    void onAriaGetVersionResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaGetVersionFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);
    void onAriaGetSessionInfoResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaGetSessionInfoFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);

    void onAriaMultiCallVersionSessionResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaMultiCallVersionSessionFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);

    void onAriaChangeGlobalOptionResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaChangeGlobalOptionFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);

    void onAriaGetGlobalOptionResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaGetGlobalOptionFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);

    void onAriaTorrentReselectFileMachineResponse(QVariant &response, QNetworkReply *reply, QVariant &payload);
    void onAriaTorrentReselectFileMachineFault(int code, QString reason, QNetworkReply *reply, QVariant &payload);

private:
    MaiaXmlRpcClient *mAriaRpc;
    std::unique_ptr<Karia2StatCalc> statCalc_;
};

#endif /* _ARIA2XMLRPCMANAGER_H_ */
