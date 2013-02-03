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
#include "RequestGroup.h"
#include "RequestGroupMan.h"

class ConsoleStatCalc;
class Aria2StatCollector;

class Karia2StatCalc : public QObject, public aria2::StatCalc
{
    Q_OBJECT;
private:
    aria2::Timer cp_;
    time_t summaryInterval_;
    aria2::SharedHandle<aria2::ConsoleStatCalc> cssc;
    int m_tid;
public:
    Karia2StatCalc(int tid, time_t summaryInterval);

    virtual ~Karia2StatCalc() {}

    virtual void calculateStat(const aria2::DownloadEngine* e);
    virtual void calculateStatDemoTest(const aria2::DownloadEngine* e);

    // 指针的所有权给上层了，本类不再保存该指针的所有权
    // 使用这个临时的pool，即使信号出问题，也能清理掉这些内存
    Aria2StatCollector *getNextStat(int stkey);
    QMap<uint64_t, Aria2StatCollector*> statPool;
    static QAtomicInt poolCounter;

signals:
//    void progressState(int tid, quint32 gid, quint64 total_length,
//                   quint64 curr_length, quint32 down_speed, quint32 up_speed,
//                   quint32 num_conns, quint32 eta);
    // TODO dont emit pointer via qt signal
    void progressStat(int stkey);

private:
    int setDownloadResultStat(const aria2::DownloadEngine* e, aria2::DownloadResultList &drs, Aria2StatCollector *stats);
    int setBaseStat(const aria2::DownloadEngine* e, aria2::SharedHandle<aria2::RequestGroup> &rg, Aria2StatCollector *stats);
    int setFilesStat(const aria2::DownloadEngine* e, aria2::SharedHandle<aria2::RequestGroup> &rg, Aria2StatCollector *stats);
    int setServersStat(const aria2::DownloadEngine* e, aria2::SharedHandle<aria2::RequestGroup> &rg, Aria2StatCollector *stats);
    int setBittorrentStat(const aria2::DownloadEngine* e, aria2::SharedHandle<aria2::RequestGroup> &rg, Aria2StatCollector *stats);
};

typedef aria2::SharedHandle<Karia2StatCalc> Karia2StatCalcHandle;

class Aria2StatCollector
{
public:
    Aria2StatCollector() {reset();}
    ~Aria2StatCollector() {}

    void reset() {
        tid = 0;
        gid = 0;
        status.clear();

        totalLength = 0;
        completedLength = 0;
        uploadLength = 0;
        bitfield.clear();
        downloadSpeed = 0;
        uploadSpeed = 0;
        infoHash.clear();
        numSeeders = 0;
        pieceLength = 0;
        numPieces = 0;
        connections = 0;
        errorCode = 0;
        dir.clear();
    }

    ////////
    int tid;
    int64_t  gid;
    std::string status;

    uint64_t totalLength;
    uint64_t completedLength;
    uint64_t uploadLength;
    std::string bitfield;
    uint32_t downloadSpeed;
    uint32_t uploadSpeed;
    std::string infoHash;
    int numSeeders;

    uint32_t pieceLength;
    uint32_t numPieces;
    uint32_t connections;
    int errorCode;

    std::vector<int64_t> followedBy;
    int64_t belongsTo;
    std::string dir;

    uint32_t eta;

    /////global
    int numActive;
    int numWaiting;
    int numStopped;
    int globalDownloadSpeed;
    int globalUploadSpeed;

    // refer to aria2::TransferStat
    class TransferStat {
    public:
        int downloadSpeed;
        int uploadSpeed;
        int64_t sessionDownloadLength;
        int64_t sessionUploadLength;
        int64_t allTimeUploadLength;
    };
    TransferStat *globalStat;
    TransferStat *sessionStat;
    QMap<uint64_t, TransferStat *> tasksStat;

    class ServerStatCollector {
    public:
        ServerStatCollector() { reset(); }
        void reset() {

        }

        int index;

        class ServerList {
        public:
            std::string uri;
            std::string currentUri;
            int downloadSpeed;
        };
        std::vector<ServerList> servers;
    };

    //
    class PeerStatCollector {
    public:
        PeerStatCollector() {reset();}
        void reset() {
            cuid = 0;
            host_name.clear();
            protocol.clear();
            status = 0;
            start_time = 0;
        }

        long long int cuid;
        std::string host_name;
        std::string protocol;
        int status;
        time_t start_time;
    };
    std::vector<PeerStatCollector> peer_stats;

    // bt
    class BitTorrentStatCollector {
    public:
        BitTorrentStatCollector() { reset(); }
        ~BitTorrentStatCollector() {}
        void reset() {
            announceList.clear();
            comment.clear();
            creationDate = 0;
            mode = 0;
            info.clear();
            name.clear();
        }

        std::vector<std::string> announceList;
        std::string comment;
        time_t creationDate;
        int mode;
        std::string info;
        std::string name;
    };
    BitTorrentStatCollector bt_stats;

    class BitTorrentPeerStatCollector {
    public:
        BitTorrentPeerStatCollector() { reset(); }
        ~BitTorrentPeerStatCollector() {}
        void reset() {
            peerId.clear();
            ip.clear();
            port = 0;
            bitfield.clear();
            amChoking = false;
            peerChoking = false;
            downloadSpeed = 0;
            uploadSpeed = 0;
            seeder = false;
        }

        std::string peerId;
        std::string ip;
        int port;
        std::string bitfield;
        bool amChoking;
        bool peerChoking;
        int downloadSpeed;
        int uploadSpeed;
        bool seeder;
    };
    std::vector<BitTorrentPeerStatCollector> btpeer_stats;

    //files
    class FilesStatCollector {
    public:
        FilesStatCollector() { reset(); }
        ~FilesStatCollector() {}

        void reset() {

        }

        int index;
        std::string path;
        std::string length;
        uint64_t completedLength;
        bool selected;

        class UrisCollector {
        public:
            UrisCollector() { reset(); }
            ~UrisCollector() {}
            void reset() {
                status.clear();
                uri.clear();
            }

            std::string status;
            std::string uri;
        };
        std::vector<UrisCollector> uris;
    };

    std::vector<FilesStatCollector> file_stats;
};

#endif /* _KARIA2STATCALC_H_ */
