// karia2statcalc.cpp ---
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-10-28 22:49:13 -0700
// Version: $Id$
// 

#include "karia2statcalc.h"

#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif // HAVE_TERMIOS_H
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif // HAVE_SYS_IOCTL_H
#include <unistd.h>

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <iterator>

#include "DownloadEngine.h"
#include "RequestGroupMan.h"
#include "RequestGroup.h"
#include "FileAllocationMan.h"
#include "FileAllocationEntry.h"
#include "CheckIntegrityMan.h"
#include "CheckIntegrityEntry.h"
#include "util.h"
#include "DownloadContext.h"
#include "wallclock.h"
#include "FileEntry.h"
#include "console.h"
#ifdef ENABLE_BITTORRENT
# include "bittorrent_helper.h"
# include "PeerStorage.h"
# include "BtRegistry.h"
#endif // ENABLE_BITTORRENT

#include "SegmentMan.h"
#include "PieceStorage.h"
#include "PeerStat.h"


Karia2StatCalc::Karia2StatCalc(int tid, time_t summaryInterval)
    : QObject(0),
      m_tid(tid),
      summaryInterval_(summaryInterval)
{
    this->cssc.reset(new aria2::ConsoleStatCalc(summaryInterval, true));
}

void Karia2StatCalc::calculateStat(const aria2::DownloadEngine* e)
{
    uint64_t total_length, curr_length;
    uint32_t down_speed, up_speed;
    uint32_t eta = 0;
    uint32_t num_conns;
    aria2::a2_gid_t gid;
    aria2::TransferStat stat;

    std::deque<aria2::SharedHandle<aria2::RequestGroup> > rgs;
    aria2::SharedHandle<aria2::RequestGroup> rg;
    aria2::SharedHandle<aria2::PieceStorage> ps;
    aria2::SharedHandle<aria2::SegmentMan> sm;
    std::vector<aria2::SharedHandle<aria2::PeerStat> > pss;
    aria2::SharedHandle<aria2::PeerStat> psts;
    std::deque<aria2::SharedHandle<aria2::RequestGroup> >::iterator it;

    rgs = e->getRequestGroupMan()->getRequestGroups();

    if (rgs.size() > 0) {
        for (it = rgs.begin(); it != rgs.end(); ++it) {
            rg = *it;
            ps = rg->getPieceStorage();
            sm = rg->getSegmentMan();
            if (sm.get())
                pss = sm->getPeerStats();

            gid = rg->getGID();
            total_length = rg->getTotalLength();
            curr_length = rg->getCompletedLength();
            // down_speed = rg->downloadFinished();
            stat = rg->calculateStat();
            down_speed = stat.getDownloadSpeed();
            up_speed = stat.getUploadSpeed();
            if(rg->getTotalLength() > 0 && stat.getDownloadSpeed() > 0) {
                eta = (rg->getTotalLength()-rg->getCompletedLength())/stat.getDownloadSpeed();
            }
            num_conns = rg->getNumConnection();

            std::cout<<gid<<":"<<total_length<<" "<<curr_length
                    <<" "<<down_speed<<" "<<up_speed<<" "<<num_conns
                   <<" "<<eta<<" PIE:"<<(ps.get() ? ps->getBitfieldLength() : 0)
                  << (ps.get() ? QByteArray((const char*)ps->getBitfield(), ps->getBitfieldLength()).data() : QByteArray().data())
                  <<std::endl;

            if (pss.size() > 0) {
                for (int i = 0; i < pss.size(); i++) {
                    psts = pss.at(i);
                    if (psts.get()) {
                        std::cout<<"psts:"<<i<<" id:"<<psts->getCuid()<<" host:"<<psts->getHostname()<<" proto:"
                                <<psts->getProtocol()<<std::endl;
                    }
                }
            }

            emit this->progressState(this->m_tid, gid, total_length, curr_length,
                                     down_speed, up_speed, num_conns, eta);
        }
    }

    ///////DEBUG
    this->cssc->calculateStat(e);
}
