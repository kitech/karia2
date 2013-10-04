// karia2statcalc.cpp ---
// 
// Author: liuguangzhao
// Copyright (C) 2007-2012 liuguangzhao@users.sf.net
// URL: 
// Created: 2011-10-28 22:49:13 -0700
// Version: $Id$
// 

#include "simplelog.h"
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

QAtomicInt Karia2StatCalc::poolCounter(1);
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
    Aria2StatCollector::TransferStat *pstat;

    aria2::RequestGroupList rgs, wrgs, frgs;
    aria2::RequestGroupList::iterator it;
    std::shared_ptr<aria2::PieceStorage> ps;
    std::shared_ptr<aria2::SegmentMan> sm;
    std::vector<std::shared_ptr<aria2::PeerStat> > pss;
    std::shared_ptr<aria2::PeerStat> psts;
    std::shared_ptr<aria2::DownloadContext> dctx;
    aria2::DownloadResultList drs;

    const unsigned char *bfptr;
    int bflen;

    rgs = e->getRequestGroupMan()->getRequestGroups();
    wrgs = e->getRequestGroupMan()->getReservedGroups();
    drs = e->getRequestGroupMan()->getDownloadResults();

    // qDebug()<<"active:"<<rgs.size()<<" wating:"<<wrgs.size()<<" done:"<<drs.size();

    // slow stat signal
    if (!e->getRequestGroupMan()->downloadFinished()) {
        if(cp_.differenceInMillis(aria2::global::wallclock())+A2_DELTA_MILLIS < 1000) {
          return;
        }
        cp_ = aria2::global::wallclock();
    }

    int stkey = 0;
    Aria2StatCollector *sclt = new Aria2StatCollector();    
    sclt->tid = m_tid;

    // global
    Q_ASSERT(sizeof(aria2::TransferStat) == sizeof(Aria2StatCollector::TransferStat));
    stat = e->getRequestGroupMan()->calculateStat();
    pstat = sclt->globalStat = (Aria2StatCollector::TransferStat*)calloc(1, sizeof(aria2::TransferStat));
    memcpy(pstat, &stat, sizeof(aria2::TransferStat));

    if (rgs.size() > 0) {
        for (it = rgs.begin(); it != rgs.end(); ++it) {
            std::shared_ptr<aria2::RequestGroup> rg2 = *it;
    
            this->setBaseStat(e, rg2, sclt);
            this->setServersStat(e, rg2, sclt);

            stat = rg2->calculateStat();
            pstat = (Aria2StatCollector::TransferStat*)calloc(1, sizeof(aria2::TransferStat));
            memcpy(pstat, &stat, sizeof(aria2::TransferStat));

            sclt->tasksStat.insert(rg2->getGID(), pstat); // gid => stat

            // stkey
            this->poolCounter.testAndSetOrdered(INT_MAX, 1);
            stkey = this->poolCounter.fetchAndAddRelaxed(1);
            if (this->statPool.contains(stkey)) {
                qLogx()<<"whooooo, impossible.";
            } else {
                this->statPool.insert(stkey, sclt);
            }
            
            //                                     down_speed, up_speed, num_conns, eta);
            qLogx()<<"stat intval:"
                   << stat.sessionDownloadLength << rg2->getGID()
                   <<rg2->getTotalLength() << rg2->getCompletedLength()
                   << rg2->getNumConnection()
                   <<"pss:"<<pss.size()
                // <<"pss:"<<(sm)<<pss.size()
                ;
            
            emit this->progressStat(stkey);
        }
    }

    // 为什么要这么处理呢
    if (drs.size() > 0) {
        this->setDownloadResultStat(e, drs, sclt);

        this->poolCounter.testAndSetOrdered(INT_MAX, 1);
        stkey = this->poolCounter.fetchAndAddRelaxed(1);
        if (this->statPool.contains(stkey)) {
            qLogx()<<"whooooo, impossible.";
        } else {
            this->statPool.insert(stkey, sclt);
        }
        qLogx()<<"maybe finished, hoho." << sclt->errorCode;
        emit this->progressStat(stkey);
    }

    ///////DEBUG
    this->cssc->calculateStat(e);
}

Aria2StatCollector *Karia2StatCalc::getNextStat(int stkey)
{
    if (this->statPool.contains(stkey)) {
        return this->statPool.take(stkey);
    }

    return NULL;
}


void Karia2StatCalc::calculateStatDemoTest(const aria2::DownloadEngine* e)
{
    aria2::TransferStat stat;

    aria2::RequestGroupList rgs, wrgs, frgs;
    aria2::RequestGroupList::iterator it;
    std::shared_ptr<aria2::PieceStorage> ps;
    std::shared_ptr<aria2::SegmentMan> sm;
    std::vector<std::shared_ptr<aria2::PeerStat> > pss;
    std::shared_ptr<aria2::PeerStat> psts;
    std::shared_ptr<aria2::DownloadContext> dctx;
    aria2::DownloadResultList dres;

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
            std::shared_ptr<aria2::RequestGroup> rg2 = *it;
            stat = e->getRequestGroupMan()->calculateStat();
            stat = rg2->calculateStat();

            Aria2StatCollector *sclt = new Aria2StatCollector();
            sclt->tid = this->m_tid;
            // sclt->globalDownloadSpeed = stat.getDownloadSpeed();
        //     sclt->globalUploadSpeed = stat.getUploadSpeed();
            sclt->globalUploadSpeed = stat.uploadSpeed;
            sclt->globalDownloadSpeed = stat.downloadSpeed;
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
            qLogx()<<"stat intval:"<<sclt->tid<<sclt->numActive<<sclt->numWaiting
                   <<sclt->numStopped << sclt->downloadSpeed << sclt->uploadSpeed
                   << stat.sessionDownloadLength << rg2->getGID()
                   <<rg2->getTotalLength() << rg2->getCompletedLength()
                   << rg2->getNumConnection()
                ;
            emit this->progressStat(0);
        }
    }

    ///////DEBUG
    this->cssc->calculateStat(e);
}

int Karia2StatCalc::setDownloadResultStat(const aria2::DownloadEngine* e, aria2::DownloadResultList &drs, Aria2StatCollector *stats)
{
    Aria2StatCollector *sclt = stats;
    aria2::DownloadResultList::iterator  it;
    aria2::DownloadResult dres;

    for (it = drs.begin(); it != drs.end(); it++) {
        auto adres = it;
        std::shared_ptr<aria2::DownloadResult> dres2 = *it;

        sclt->gid = dres2->metadataInfo->getGID();
        sclt->totalLength = dres2->totalLength;
        sclt->completedLength = dres2->completedLength;
        sclt->uploadLength = dres2->uploadLength;
        sclt->numPieces = dres2->numPieces;
        sclt->pieceLength = dres2->pieceLength;
        sclt->bitfield = dres2->bitfield;
        sclt->infoHash = dres2->infoHash;
        sclt->errorCode = dres2->result;
    }

    return 0;
}


int Karia2StatCalc::setBaseStat(const aria2::DownloadEngine* e, std::shared_ptr<aria2::RequestGroup> &rg,  Aria2StatCollector *stats)
{
    Aria2StatCollector *sclt = stats;
    aria2::TransferStat stat;

    std::deque<std::shared_ptr<aria2::RequestGroup> > rgs;
//    std::shared_ptr<aria2::RequestGroup> rg;
    std::shared_ptr<aria2::PieceStorage> ps;
    std::shared_ptr<aria2::SegmentMan> sm;
    std::vector<std::shared_ptr<aria2::PeerStat> > pss;
    std::shared_ptr<aria2::PeerStat> psts;
    std::shared_ptr<aria2::DownloadContext> dctx;
    std::deque<std::shared_ptr<aria2::RequestGroup> >::iterator it;

    const unsigned char *bfptr;
    int bflen;

    stats->sessionId = e->getSessionId();

    ps = rg->getPieceStorage();
    sm = rg->getSegmentMan();
    if (sm.get())
        pss = sm->getPeerStats();
    dctx = rg->getDownloadContext();

    sclt->state = rg->getState();
    sclt->gid = rg->getGID();
    sclt->totalLength = rg->getTotalLength();
    sclt->completedLength = rg->getCompletedLength();
    sclt->uploadLength = 0; // ???????
    stat = rg->calculateStat();
    sclt->downloadSpeed = stat.downloadSpeed;
    sclt->uploadSpeed = stat.uploadSpeed;
    if(rg->getTotalLength() > 0 && stat.downloadSpeed > 0) {
        sclt->eta = (rg->getTotalLength()-rg->getCompletedLength())/stat.downloadSpeed;
    }
    sclt->connections = rg->getNumConnection();
    if (dctx.get()) {
        sclt->numPieces = dctx->getNumPieces();
        sclt->pieceLength = dctx->getPieceLength();
    }

    qLogx()<<sclt->gid<<":"<<sclt->totalLength<<" "<<sclt->completedLength
           <<" "<<sclt->downloadSpeed<<" "<<sclt->uploadSpeed
           <<" "<<sclt->connections<<" "<<sclt->numPieces <<" "<<sclt->pieceLength
           <<" "<<sclt->eta<<" finish:"<<rg->downloadFinished();

    if (ps.get()) {
        bfptr = ps->getBitfield();
        bflen = ps->getBitfieldLength();

        sclt->bitfield = aria2::util::toHex(bfptr, bflen);
        qLogx()<<"Bitfield:"<<(bflen*8)<<" "<<sclt->bitfield.c_str();
    }

    if (pss.size() > 0) {
        Aria2StatCollector::PeerStatCollector psc;
        for (int i = 0; i < pss.size(); i++) {
            psts = pss.at(i);
            if (psts.get()) {
                qLogx()<<"psts:"<<i<<" id:"<<psts->getCuid()<<" host:"<<psts->getHostname().c_str()<<" proto:"
                       <<psts->getProtocol().c_str();
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

int Karia2StatCalc::setFilesStat(const aria2::DownloadEngine* e, std::shared_ptr<aria2::RequestGroup> &rg, Aria2StatCollector *stats)
{
    Aria2StatCollector *sclt = stats;
    aria2::TransferStat stat;

    std::deque<std::shared_ptr<aria2::RequestGroup> > rgs;
    std::shared_ptr<aria2::PieceStorage> ps;
    std::shared_ptr<aria2::SegmentMan> sm;
    std::vector<std::shared_ptr<aria2::PeerStat> > pss;
    std::shared_ptr<aria2::PeerStat> psts;
    std::shared_ptr<aria2::DownloadContext> dctx;
    std::deque<std::shared_ptr<aria2::RequestGroup> >::iterator it;

    const unsigned char *bfptr;
    int bflen;



    return 0;
}

int Karia2StatCalc::setServersStat(const aria2::DownloadEngine* e, std::shared_ptr<aria2::RequestGroup> &rg, Aria2StatCollector *stats)
{
    Aria2StatCollector *sclt = stats;
    aria2::TransferStat stat;

    std::deque<std::shared_ptr<aria2::RequestGroup> > rgs;
    std::shared_ptr<aria2::PieceStorage> ps;
    std::shared_ptr<aria2::SegmentMan> sm;
    std::vector<std::shared_ptr<aria2::PeerStat> > pss;
    std::shared_ptr<aria2::PeerStat> psts;
    std::shared_ptr<aria2::DownloadContext> dctx;
    std::deque<std::shared_ptr<aria2::RequestGroup> >::iterator it;
    std::shared_ptr<aria2::MetadataInfo> metaInfo;
    std::vector<std::shared_ptr<aria2::FileEntry> > fileEntries;
    std::vector<std::string> uris;

    const unsigned char *bfptr;
    int bflen;

    dctx = rg->getDownloadContext();

    sm = rg->getSegmentMan();
    if (!sm) {
        return 0;
    }

    fileEntries = dctx->getFileEntries();
    if (!fileEntries.empty()) {
        fileEntries.at(0)->getUris(uris);
    }

    pss = sm->getPeerStats();
    for (int i = 0; i < pss.size(); i++) {
        psts = pss.at(i);
        
        Aria2StatCollector::ServerStatCollector::ServerInfo servInfo;
        metaInfo = rg->getMetadataInfo();
        servInfo.uri = metaInfo ? metaInfo->getUri() : "";
        servInfo.uri = servInfo.uri.empty() ? (uris.empty() ? "" : uris.at(0)) : servInfo.uri;
        servInfo.downloadSpeed = psts->calculateDownloadSpeed();
        servInfo.state = psts->getStatus();
        servInfo.hostname = psts->getHostname();
        servInfo.protocol = psts->getProtocol();

        stats->server_stats.servers.push_back(servInfo);

        qLogx()<<"servinfo:" <<(metaInfo ? metaInfo->getGID() : -1)
               <<(metaInfo ? metaInfo->getUri().c_str() : "'mi null'")
               <<"gid="<<rg->getGID()
               <<servInfo.uri.c_str()<<servInfo.downloadSpeed<<servInfo.state
               << servInfo.hostname.c_str() << servInfo.protocol.c_str();

    }

    return 0;
}

int Karia2StatCalc::setBittorrentStat(const aria2::DownloadEngine* e, std::shared_ptr<aria2::RequestGroup> &rg, Aria2StatCollector *stats)
{
    return 0;
}

