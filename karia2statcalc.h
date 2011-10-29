// karia2statcalc.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-10-28 22:48:56 -0700
// Version: $Id$
// 
#ifndef _KARIA2STATCALC_H_
#define _KARIA2STATCALC_H_


#include "StatCalc.h"

#include <string>
#include <functional>
#include <QtCore>

#include "TimerA2.h"
#include "ConsoleStatCalc.h"

class ConsoleStatCalc;

class Karia2StatCalc:public QObject, public aria2::StatCalc
{
    Q_OBJECT;
private:
    time_t summaryInterval_;
    aria2::SharedHandle<aria2::ConsoleStatCalc> cssc;
    int m_tid;
public:
    Karia2StatCalc(int tid, time_t summaryInterval);

    virtual ~Karia2StatCalc() {}

    virtual void calculateStat(const aria2::DownloadEngine* e);

signals:
    void progressState(int tid, quint32 gid, quint64 total_length,
                   quint64 curr_length, quint32 down_speed, quint32 up_speed,
                   quint32 num_conns, quint32 eta);
};

typedef aria2::SharedHandle<Karia2StatCalc> Karia2StatCalcHandle;

#endif /* _KARIA2STATCALC_H_ */
