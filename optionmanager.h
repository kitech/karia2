// optionmanager.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-05-05 14:01:50 +0800
// Version: $Id$
// 

#include <QtCore>
#include <QtGui>

#ifndef _OPTIONMANAGER_H_
#define _OPTIONMANAGER_H_

#include "sqlitestorage.h"

// template
// QString getMaxSpeed() 
// {
//     if (this->mUserOptions.contains("MaxSpeed")) {
//         return this->mUserOptions.value("MaxSpeed").value;
//     } else if (this->mDefaultOptions.contains("MaxSpeed")) {
//         return this->mDefaultOptions.value("MaxSpeed");
//     } else {
//         return this->loadKey("MaxSpeed", "12345");
//     }
//     return QString();
// }


// a #, mean replace the macro and use " cap's it

#define DECL_OPTION(x,y)                                \
    private: QString x;                                 \
public: QString get##x() {                              \
    QString _key = #x;                                        \
    if (this->mUserOptions.contains(_key)) {                  \
        qDebug()<<"Get "<<_key<<" from user option cache";    \
        return this->mUserOptions.value(_key).value;          \
    } else if (this->mDefaultOptions.contains(_key)) {         \
        qDebug()<<"Get "<<_key<<" from default option cache";  \
        return this->mDefaultOptions.value(_key).value;         \
    } else {                                                    \
        QString _value = this->loadKey(_key, y);                \
        OptionElem _oe(_key, _value);                           \
        this->mUserOptions[_key] = _oe;                         \
        qDebug()<<"Get "<<_key<<" from database";               \
        return _value;                          \
    }                                                           \
    return QString();                                           \
    }                                                           \
public: bool save##x(QString _value) {                          \
    QString _key = #x;                                          \
    OptionElem _oe(_key, _value);                               \
    this->mUserOptions[_key] = _oe;                             \
    this->saveKey(_key, _value, y);                             \
    return true;                                                \
    }


class OptionManager : public QObject
{
    Q_OBJECT;
public:
    ~OptionManager();
	static OptionManager *instance() ;
    QString getMaxSpeed(QString key);

private:
    QString loadKey(QString key, QString dvalue);
    bool saveKey(QString key, QString dvalue, QString type = "auto");
    DECL_OPTION(MaxSegment, "56");
    DECL_OPTION(MinSegmentSize, "1234");
    DECL_OPTION(AutoSaveTaskInterval, "156");
    DECL_OPTION(WriteDataSize, "12345");
    
    // 
    DECL_OPTION(DefaultRefer, "http://www.qtchina.net");
    DECL_OPTION(TaskStartSchedule, "imidiate");
    DECL_OPTION(MaxSegmentEveryTask, "567");

    // 
    DECL_OPTION(ConnectTimeout, "98");
    DECL_OPTION(ReadDataTimeout, "97");
    DECL_OPTION(RetryDelayTimeout, "16");
    DECL_OPTION(MaxSimulateJobs, "7");
    
    // 
    DECL_OPTION(MonitorIe, "false");
    DECL_OPTION(MonitorOpera, "false");
    DECL_OPTION(MonitorFirefox, "false");

    // 
    DECL_OPTION(NoProxy, "true");
    DECL_OPTION(CustomProxy, "false");
    DECL_OPTION(HttpProxy, tr("Direct"));
    DECL_OPTION(FtpProxy, tr("Direct"));
    DECL_OPTION(BittorrentProxy, tr("Direct"));
    DECL_OPTION(MetalinkProxy, tr("Direct"));

    // other
    DECL_OPTION(TaskShowColumns, "");
    DECL_OPTION(RememberSpeedLimit, "false");
    DECL_OPTION(SpeedLimitType, "unlimited"); // other's are manual, auto
    DECL_OPTION(SpeedLimitSpeed, "1");  // bytes/sec
    DECL_OPTION(ShutdownWhenDone, "false");

private:
    class OptionElem {
    public:
        OptionElem(){}
        OptionElem(QString k, QString v, QString t = "auto") {
            this->key = k; this->value = v; this->type = t;
        }
        QString key;
        QString value;
        QString type;
        bool dirty;
    };
	OptionManager(QObject *parent = 0);
	//
	static OptionManager *mInstance;
    SqliteStorage *storage;

    QHash<QString, OptionElem> mUserOptions;
    QHash<QString, OptionElem> mDefaultOptions;
};

#endif /* _OPTIONMANAGER_H_ */
