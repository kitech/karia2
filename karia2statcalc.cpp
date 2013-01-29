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
    aria2::TransferStat stat;

    aria2::RequestGroupList rgs, wrgs, frgs;
    // aria2::SharedHandle<aria2::RequestGroup> rg;
    // aria2::RequestGroup* rg; //xxx
    aria2::RequestGroupList::SeqType::iterator it;
    aria2::SharedHandle<aria2::PieceStorage> ps;
    aria2::SharedHandle<aria2::SegmentMan> sm;
    std::vector<aria2::SharedHandle<aria2::PeerStat> > pss;
    aria2::SharedHandle<aria2::PeerStat> psts;
    aria2::SharedHandle<aria2::DownloadContext> dctx;
    aria2::DownloadResultList dres;
    // std::deque<aria2::SharedHandle<aria2::RequestGroup> >::iterator it;


    const unsigned char *bfptr;
    int bflen;

    rgs = e->getRequestGroupMan()->getRequestGroups();
    wrgs = e->getRequestGroupMan()->getReservedGroups();
    dres = e->getRequestGroupMan()->getDownloadResults();

    std::cout<<"active:"<<rgs.size()<<" wating:"<<wrgs.size()<<" done:"<<dres.size()<<std::endl;

    // slow stat signal
    {
        if(cp_.differenceInMillis(aria2::global::wallclock())+A2_DELTA_MILLIS < 1000) {
          return;
        }
        cp_ = aria2::global::wallclock();
    }

    if (rgs.size() > 0) {
        for (it = rgs.begin(); it != rgs.end(); ++it) {
            auto rg = *it;
            stat = e->getRequestGroupMan()->calculateStat();

            Aria2StatCollector *sclt = new Aria2StatCollector();
            sclt->tid = this->m_tid;
            // sclt->globalDownloadSpeed = stat.getDownloadSpeed();
        //     sclt->globalUploadSpeed = stat.getUploadSpeed();
            sclt->numActive = rgs.size();
            sclt->numWaiting = wrgs.size();
            sclt->numStopped = dres.size();

            {
                // status
                // if (rg.isPauseRequested()) {
                //     sclt->status = std::string("paused");
                // } else {
                //     sclt->status = std::string("active");
                // }
            }

        //        this->setBaseStat(e, rg, sclt);
            //            emit this->progressState(this->m_tid, gid, total_length, curr_length,
            //                                     down_speed, up_speed, num_conns, eta);
            emit this->progressState(sclt);
        }
    }

    ///////DEBUG
    this->cssc->calculateStat(e);
}

int Karia2StatCalc::setBaseStat(const aria2::DownloadEngine* e, aria2::SharedHandle<aria2::RequestGroup> &rg,  Aria2StatCollector *stats)
{
    Aria2StatCollector *sclt = stats;
    aria2::TransferStat stat;

    std::deque<aria2::SharedHandle<aria2::RequestGroup> > rgs;
//    aria2::SharedHandle<aria2::RequestGroup> rg;
    aria2::SharedHandle<aria2::PieceStorage> ps;
    aria2::SharedHandle<aria2::SegmentMan> sm;
    std::vector<aria2::SharedHandle<aria2::PeerStat> > pss;
    aria2::SharedHandle<aria2::PeerStat> psts;
    aria2::SharedHandle<aria2::DownloadContext> dctx;
    std::deque<aria2::SharedHandle<aria2::RequestGroup> >::iterator it;

    const unsigned char *bfptr;
    int bflen;

    ps = rg->getPieceStorage();
    sm = rg->getSegmentMan();
    if (sm.get())
        pss = sm->getPeerStats();
    dctx = rg->getDownloadContext();

    // sclt->gid = rg->getGID();
    // sclt->totalLength = rg->getTotalLength();
    // sclt->completedLength = rg->getCompletedLength();
    // sclt->uploadLength = 0; // ???????
    // stat = rg->calculateStat();
    // sclt->downloadSpeed = stat.getDownloadSpeed();
    // sclt->uploadSpeed = stat.getUploadSpeed();
    // if(rg->getTotalLength() > 0 && stat.getDownloadSpeed() > 0) {
    //     sclt->eta = (rg->getTotalLength()-rg->getCompletedLength())/stat.getDownloadSpeed();
    // }
    // sclt->connections = rg->getNumConnection();
    // if (dctx.get()) {
    //     sclt->numPieces = dctx->getNumPieces();
    //     sclt->pieceLength = dctx->getPieceLength();
    // }

    std::cout<<sclt->gid<<":"<<sclt->totalLength<<" "<<sclt->completedLength
            <<" "<<sclt->downloadSpeed<<" "<<sclt->uploadSpeed
           <<" "<<sclt->connections<<" "<<sclt->numPieces <<" "<<sclt->pieceLength
          <<" "<<sclt->eta<<" finish:"<<rg->downloadFinished()
          <<std::endl;

    if (ps.get()) {
        bfptr = ps->getBitfield();
        bflen = ps->getBitfieldLength();

        sclt->bitfield = aria2::util::toHex(bfptr, bflen);
        std::cout<<"Bitfield:"<<(bflen*8)<<" "<<sclt->bitfield<<std::endl;
    }

    if (pss.size() > 0) {
        Aria2StatCollector::PeerStatCollector psc;
        for (int i = 0; i < pss.size(); i++) {
            psts = pss.at(i);
            if (psts.get()) {
                std::cout<<"psts:"<<i<<" id:"<<psts->getCuid()<<" host:"<<psts->getHostname()<<" proto:"
                        <<psts->getProtocol()<<std::endl;
                psc.reset();
                psc.cuid = psts->getCuid();
                psc.host_name = psts->getHostname();
                psc.protocol = psts->getProtocol();

                sclt->peer_stats.push_back(psc);
            }
        }
    }

    return 0;
}

int Karia2StatCalc::setFilesStat(const aria2::DownloadEngine* e, aria2::SharedHandle<aria2::RequestGroup> &rg, Aria2StatCollector *stats)
{
    Aria2StatCollector *sclt = stats;
    aria2::TransferStat stat;

    std::deque<aria2::SharedHandle<aria2::RequestGroup> > rgs;
//    aria2::SharedHandle<aria2::RequestGroup> rg;
    aria2::SharedHandle<aria2::PieceStorage> ps;
    aria2::SharedHandle<aria2::SegmentMan> sm;
    std::vector<aria2::SharedHandle<aria2::PeerStat> > pss;
    aria2::SharedHandle<aria2::PeerStat> psts;
    aria2::SharedHandle<aria2::DownloadContext> dctx;
    std::deque<aria2::SharedHandle<aria2::RequestGroup> >::iterator it;

    const unsigned char *bfptr;
    int bflen;



    return 0;
}

int Karia2StatCalc::setServersStat(const aria2::DownloadEngine* e, aria2::SharedHandle<aria2::RequestGroup> &rg, Aria2StatCollector *stats)
{
    Aria2StatCollector *sclt = stats;
    aria2::TransferStat stat;

    std::deque<aria2::SharedHandle<aria2::RequestGroup> > rgs;
//    aria2::SharedHandle<aria2::RequestGroup> rg;
    aria2::SharedHandle<aria2::PieceStorage> ps;
    aria2::SharedHandle<aria2::SegmentMan> sm;
    std::vector<aria2::SharedHandle<aria2::PeerStat> > pss;
    aria2::SharedHandle<aria2::PeerStat> psts;
    aria2::SharedHandle<aria2::DownloadContext> dctx;
    std::deque<aria2::SharedHandle<aria2::RequestGroup> >::iterator it;

    const unsigned char *bfptr;
    int bflen;


    return 0;
}

int Karia2StatCalc::setBittorrentStat(const aria2::DownloadEngine* e, aria2::SharedHandle<aria2::RequestGroup> &rg, Aria2StatCollector *stats)
{
    return 0;
}

