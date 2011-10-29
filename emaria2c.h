// emaria2c.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-10-23 06:54:26 -0700
// Version: $Id$
// 

#ifndef _EMARIA2C_H_
#define _EMARIA2C_H_

#include "common.h"

#include <vector>

#include "SharedHandle.h"
#include "DownloadResult.h"

#include <QtCore>

class TaskOption;
class EAria2Worker;
class EAria2Man;

class EAria2Man : public QObject
{
    Q_OBJECT;
protected:
    static EAria2Man *m_instance;
    static QMutex m_inst_mutex;
    EAria2Man(QObject *parent = 0);
public:
    static EAria2Man *instance();
    virtual ~EAria2Man();

public slots:
    // int addTask(QString url);
    int addUri(int task_id, const QString &url, TaskOption *to);

private:
    int _option_processing(aria2::Option& op, std::vector<std::string>& uris,
                           int argc, char* argv[]);
protected:
    QHash<int, EAria2Worker*> m_tasks;
    int m_argc;
    char m_argv[32][256];
};

class EAria2Worker : public QThread
{
    Q_OBJECT;
public:
    EAria2Worker(QObject *parent = 0);
    virtual ~EAria2Worker();

    void run();

protected:
    friend class EAria2Man;

    int m_wid;
    std::vector<aria2::SharedHandle<aria2::RequestGroup> > requestGroups_;

    aria2::SharedHandle<aria2::Option> option_;
    // aria2::SharedHandle<aria2::StatCalc> statCalc_;
    // aria2::SharedHandle<aria2::OutputFile> summaryOut_;
};

#endif /* _EMARIA2C_H_ */
