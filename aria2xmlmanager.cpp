// aria2xmlmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-27 21:56:32 +0000
// Version: $Id$
// 

#include <QtNetwork>

#include "simplelog.h"

#include "maiaXmlRpcClient.h"
#include "aria2rpcserver.h"
#include "aria2xmlmanager.h"

Aria2XmlManager::Aria2XmlManager()
    : Aria2RpcManager()
{
}

Aria2XmlManager::~Aria2XmlManager()
{
}

int Aria2XmlManager::addTask(int task_id, const QString &url, TaskOption *to)
{
    // the default port is 6800 on best case, change to 6800+ if any exception.
    this->mAriaRpc = new MaiaXmlRpcClient(QUrl(this->mRpcServer->getRpcUri(Aria2RpcServer::AST_XMLRPC)));

    // get version and session
    // use aria2's multicall method
    QVariantMap  getVersion;
    QVariantMap getSession;
    QVariantList gargs;
    QVariantList args;
    QVariantMap options;
    QVariant payload = 34567;
        
    getVersion["methodName"] = QString("aria2.getVersion");
    getVersion["params"] = QVariant(options);
    getSession["methodName"] = QString("aria2.getSessionInfo");
    getSession["params"] = QVariant(options);

    args.insert(0, getVersion);
    args.insert(1, getSession);

    gargs.insert(0, args);

    this->mAriaRpc->call(QString("system.multicall"), gargs, payload,
                         // this, (const char*)&Aria2XmlManager::onAriaGetFeatureResponse,
                         // this, (const char*)&Aria2XmlManager::onAriaGetFeatureFault);
                         this, SLOT(onAriaGetFeatureResponse(QVariant&, QNetworkReply *, QVariant&)),
                         this, SLOT(onAriaGetFeatureFault(int, QString, QNetworkReply *, QVariant &)));

    return 0;
}

int Aria2XmlManager::pauseTask(int task_id)
{
    return 0;
}

bool Aria2XmlManager::onAllStatArrived(int stkey)
{
    return true;
}

void Aria2XmlManager::onAriaGetFeatureResponse(QVariant &response, QNetworkReply * reply, QVariant &payload)
{
    qLogx()<<response<<payload;

/*
    this->mAriaRpc->deleteLater(); // can not delete it on it slot, should use deleteLater;
    this->mAriaRpc = 0;

    QVariantMap features = response.toList().at(0).toMap();

    this->mVersionString = features.value("version").toString();

    this->mIVersion = 0;
    QStringList verParts = this->mVersionString.split(".");
    Q_ASSERT(verParts.count() > 0 && verParts.count() <= 4);
    for (int i = verParts.count() - 1 ; i >= 0 ; --i) {
        this->mIVersion |= (verParts.at(i).toInt() << (i * 8));
    }

    this->mFeatures = 0;    
    QVariantList enabledFeatures = features.value("enabledFeatures").toList();
    if (enabledFeatures.contains(QString("BitTorrent"))) {
        this->mFeatures |= FeatureBitTorrent;
    }

    if (enabledFeatures.contains(QString("GZip"))) {
        this->mFeatures |= FeatureGZip;
    }

    if (enabledFeatures.contains(QString("HTTPS"))) {
        this->mFeatures |= FeatureHTTPS;
    }

    if (enabledFeatures.contains(QString("Message Digest"))) {
        this->mFeatures |= FeatureMessageDigest;
    }

    if (enabledFeatures.contains(QString("Metalink"))) {
        this->mFeatures |= FeatureMetalink;
    }

    if (enabledFeatures.contains(QString("XML-RPC"))) {
        this->mFeatures |= FeatureXMLRPC;
    }

    if (enabledFeatures.contains(QString("Async DNS"))) {
        this->mFeatures |= FeatureAsyncDNS;
    }

    if (enabledFeatures.contains(QString("Firefox3 Cookie"))) {
        this->mFeatures |= FeatureFirefox3Cookie;
    }

    // session parser
    if (response.toList().at(1).type() == QVariant::List) {
        QVariantList sessions = response.toList().at(1).toList();
        this->mSessionId = sessions.at(0).toMap().value("sessionId").toString();
    } else {
        // aria2 == 1.8.0, now getSession method. this is the error info.
        QVariantMap sessions = response.toList().at(1).toMap();
        qDebug()<<sessions;
    }
*/
}

void Aria2XmlManager::onAriaGetFeatureFault(int code, QString reason, QNetworkReply * reply, QVariant &payload)
{
    qLogx()<<code<<reason<<payload;
    this->mAriaRpc->deleteLater(); // can not delete it on it slot, should use deleteLater;
    this->mAriaRpc = 0;
}

