/***************************************************************
 * skype.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @Author:      flyfish (liuguangzhao@users.sf.net)
 * @License:     GPL v2.0 or later
 * @Created:     2008-04-30.
 * @Revision:    $Id$
 * Description:
 * Usage:
 * TODO:
 * CHANGES:
 ***************************************************************/
#include <QtCore>
#include <QtGui/QApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>


#include "skype.h"
#include "skypecommand.h"

Skype::Skype(QString AppName): appPrefix(AppName) {
    this->mConnected = false;
    TimeOut = 10000;
    pingTimer = new QTimer(this);
    contactsUpToDate = false;

    QObject::connect(this, SIGNAL(connected(QString)), this, SLOT(onConnected(QString)));
}

Skype::~Skype()
{
}

QStringList Skype::getContacts() {
    if ( this->mConnected ) {
        if ( contactsUpToDate ) return contacts;
        doCommand( SkypeCommand::GET_CONTACT_LIST() );
        return contacts;
    }
    return contacts;
}


void Skype::timeOut() {
    qDebug() << "Timeout while waiting for event #"<<waitForResponseID;
    localEventLoop.exit(2);
}

void Skype::ping() { 
    // sk.sendMsgToSkype( SkypeCommand::PING() );
    this->doCommand(SkypeCommand::PING());
}


void Skype::readIncomingData(QString contactName, int streamNum) {
    sk.sendMsgToSkype( SkypeCommand::READ_AP2AP(appName, contactName, streamNum) );
}

void Skype::processMessage(const QString &message) {
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    // QString u8msg = codec->toUnicode(message.toAscii());
    qDebug() <<__FILE__<<__LINE__<< "SKYPE: <=" << message; // << u8msg;
    emit this->commandResponse(message);

    SkypeResponse cmd;

    if ( ! cmd.parse(message) ) {
        emit skypeError( -1, "Error parsing Skype output" );
        if ( waitingForResponse ) localEventLoop.exit(1);
        return;
    }

    if ( cmd.type() == SK_CONTACT_LIST ) {
        contacts = cmd.getContacts();
        contactsUpToDate = true;
    }

    if ( waitingForResponse && cmd.responseID() == waitForResponseID ) {
        qDebug() << "Received event "<<cmd.responseID() <<" we've been waiting for.";
        qDebug() << "Response received:"<<message;
        localEventLoop.exit(0);
        return;
    }

    if ( cmd.type() == SK_UNKNOWN ) { 
        qDebug()<<__FILE__<<__LINE__<<"UNKNOWN cmd type:"<<cmd.data();
        return;
    }

    if ( cmd.type() == SK_ERROR ) { 
        emit skypeError( cmd.errorCode(), cmd.errorMsg() );
        return;
    }
    // if ( cmd.appName() != appName ) return;

    if (cmd.type() == SK_PROTOCOL) {
        this->protocolNumber = cmd.protocolNum();
        return;
    }

    if (cmd.type() == SK_CURRENTUSERHANDLE) {
        this->appName = "skynet_" + this->appPrefix;// + "_" + cmd.contactName();
        qDebug()<<"unique appName:"<<this->appName;

        emit connected(cmd.contactName());

        bool ok = doCommand( SkypeCommand::CREATE_AP2AP(appName) );
        Q_ASSERT(ok);
        return;
    }

    if ( cmd.type() == SK_READY_TO_READ ) {
        readIncomingData( cmd.contactName(), cmd.streamNum() );
        return;
    }

    if ( cmd.type() == SK_DATA ) {
        if ( streams.contains( cmd.contactName() ) ) {
            streams[cmd.contactName()].append( cmd.data() );
        } else { // should not happen (a SK_STREAMS message should always arrive before)
            qDebug() << "ASSERT: Data arriving before stream Created (" << cmd.contactName() <<":"<< cmd.streamNum() << cmd.data() << ")"; 
            streams[cmd.contactName()] = cmd.data();
            // activeStream[cmd.contactName()] = cmd.streamNum();
            this->activeStreams.insert(cmd.contactName(), cmd.streamNum());
        }
        emit dataInStream( cmd.contactName() );
        return;
    }

    if (cmd.type() == SK_DATAGRAM) {
        // qDebug()<<"Got udp package";
        if (this->dataGrams.contains(cmd.streamNum())) {
            this->dataGrams[cmd.streamNum()] = cmd.data();
        } else {
            // activeStream[cmd.contactName()] = cmd.streamNum();
            this->activeStreams.insert(cmd.contactName(), cmd.streamNum());
            this->dataGrams[cmd.streamNum()] = cmd.data();
        }
        emit this->packageArrived(cmd.contactName(), cmd.streamNum(), cmd.data());
        return;
    }

    if ( cmd.type() == SK_STREAMS ) {
        QByteArray data;
        if (! streams.contains( cmd.contactName() ) ) {
            streams.insert( cmd.contactName(), data );
        }
        // activeStream[ cmd.contactName() ] = cmd.streamNum();
        this->activeStreams.insert(cmd.contactName(), cmd.streamNum());
        emit newStreamCreated(cmd.contactName(), cmd.streamNum());
        return;
    }

    if (cmd.type() == SK_CLOSE_STREAM) {
        // qDebug()<<__FILE__<<__LINE__<<"Maybe stream disconnected:"<<cmd.streamNum();
        emit this->streamClosed();
    }
}
// shoud not be here, maybe in upper level
void Skype::processAP2APMessage(const QString &message)
{
    
}

int Skype::waitForResponse( QString cID ) 
{
    waitingForResponse = true;
    waitForResponseID = cID;
    //QTimer *timer = new QTimer(this);
    QTimer::singleShot(TimeOut, this, SLOT(timeOut()));
    int result = localEventLoop.exec();
    waitingForResponse = false;
    return result;
}

bool Skype::doCommand(QString cmd, bool blocking) 
{
    QString cID = SkypeCommand::prependID( cmd );
    QString ID = SkypeCommand::getID( cID );
    qDebug() <<__FILE__<<__LINE__<< "SKYPE: =>" << cID;
    emit this->commandRequest(cmd);
    sk.sendMsgToSkype( cID );
    if ( blocking ) {
        qDebug() << "Waiting for response to message "<<ID;
        int result = waitForResponse( ID );
        qDebug() << "Result of waiting" << result;
        if ( result == 0 ) {
            if ( response.type() != SK_ERROR ) return true;
        }
        return false;
    } else return true;
}

void Skype::onCommandRequest(QString cmd)
{
    this->doCommand(cmd);
}

bool Skype::connectToSkype() 
{ 
    if ( this->mConnected ) return true;
    if ( ! sk.attachToSkype() ) return false;
    QObject::connect(&sk, SIGNAL(newMsgFromSkype(const QString)), this, SLOT(processMessage(const QString)));
    // if ( ! doCommand( SkypeCommand::CONNECT_TO_SKYPE(appName) ) ) return false;
    if ( ! doCommand( SkypeCommand::CONNECT_TO_SKYPE(this->appPrefix) ) ) return false;
    if ( ! doCommand( SkypeCommand::PROTOCOL(50) ) ) return false;
    // if ( ! doCommand( SkypeCommand::CREATE_AP2AP(appName) ) ) return false;
    connect( pingTimer, SIGNAL( timeout() ), this, SLOT( ping() ) );
    pingTimer->start(20000);
    this->mConnected = true;
    return true;
}

bool Skype::disconnectFromSkype() 
{
    if ( ! this->mConnected) return true;
    // if ( ! doCommand( SkypeCommand::DELETE_AP2AP(appName) ) ) return false;
    disconnect( &sk, 0, this, 0 );
    pingTimer->stop();
    disconnect(pingTimer, 0, 0, 0 );
    this->mConnected = false;
    return true;
}


void Skype::newStream(QString contact) 
{ 
    int ok = doCommand( SkypeCommand::CONNECT_AP2AP( appName, contact ) );
    Q_ASSERT(ok);
}

bool Skype::writeToStream(QByteArray data, QString contactName ) 
{
    // if ( ! activeStream.contains( contactName ) )  return false; // We are not connected to contactName
    if (!this->activeStreams.leftContains(contactName)) return false;

    // doCommand( SkypeCommand::WRITE_AP2AP( appName, contactName, activeStream[contactName],data ), false );
    doCommand( SkypeCommand::WRITE_AP2AP( appName, contactName, this->activeStreams[contactName], data), false);
    return true;
}

QByteArray Skype::readFromStream(QString contactName) {
    QByteArray ret;
    ret.clear();
    if ( streams.contains( contactName ) ) { 
        //   qDebug() << "DEBUG: streams["<<contactName<<"]="<< streams[contactName];
        ret.append( streams[contactName] );
        streams[contactName].clear();
    } 
    return ret;
}

bool Skype::sendPackage(QString contactName, int streamNum, QString data)
{
    QString cmd = SkypeCommand::SEND_AP2AP(this->appName, contactName, streamNum, data);
    if (!this->doCommand(cmd)) {
        Q_ASSERT(1 == 2);
        return false;
    }
    return true;    
}

// for client, should use only one stream
bool Skype::sendPackage(QString contactName, QString data)
{
    // qDebug()<<this->activeStream;
    qDebug()<<this->activeStreams;
    // TODO stream_id should be dynamic detect
    // Q_ASSERT(this->activeStream.count() == 1);
    Q_ASSERT(this->activeStreams.count() == 1);
    // QList<QString> keys = this->activeStream.keys();
    QList<QString> keys = this->activeStreams.leftValues();
    // int streamNum = this->activeStream.value(keys.at(0));
    int streamNum = this->activeStreams.leftToRight(keys.at(0));
    return this->sendPackage(contactName, streamNum, data);
}

void Skype::onConnected(QString skypeName)
{
    Q_UNUSED(skypeName);
    // bool ok = doCommand( SkypeCommand::CREATE_AP2AP(appName) );
    // Q_ASSERT(ok);
}

// #include "skype.moc"
