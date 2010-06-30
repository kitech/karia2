/***************************************************************
 * skype.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2008-04-30.
 * @Last Change: 2008-04-30.
 * @Revision:    $Id$
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/
#include <QtGui/QApplication>
#include <QtCore/QDebug>
#include <QtCore/QTimer>


#include "skype.h"
#include "skypeCommand.h"

skype::skype(QString AppName): appPrefix(AppName) {
    this->mConnected = false;
    TimeOut = 10000;
    pingTimer = new QTimer(this);
    contactsUpToDate = false;

    QObject::connect(this, SIGNAL(connected(QString)), this, SLOT(onConnected(QString)));
}

QStringList skype::getContacts() {
    if ( this->mConnected ) {
        if ( contactsUpToDate ) return contacts;
        doCommand( skypeCommand::GET_CONTACT_LIST() );
        return contacts;
    }
    return contacts;
}


void skype::timeOut() {
    qDebug() << "Timeout while waiting for event #"<<waitForResponseID;
    localEventLoop.exit(2);
}

void skype::ping() { 
    sk.sendMsgToSkype( skypeCommand::PING() );
}


void skype::readIncomingData(QString contactName, int streamNum) {
    sk.sendMsgToSkype( skypeCommand::READ_AP2AP(appName, contactName, streamNum) );
}

void skype::processMessage(const QString &message) {
    qDebug() <<__FILE__<<__LINE__<< "SKYPE: <=" << message;

    skypeResponse cmd;

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
        this->appName = "skype_app_" + this->appPrefix;// + "_" + cmd.contactName();
        qDebug()<<"unique appName:"<<this->appName;

        emit connected(cmd.contactName());

        bool ok = doCommand( skypeCommand::CREATE_AP2AP(appName) );
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
            activeStream[cmd.contactName()] = cmd.streamNum();
        }
        emit dataInStream( cmd.contactName() );
        return;
    }

    if ( cmd.type() == SK_STREAMS ) {
        QByteArray data;
        if (! streams.contains( cmd.contactName() ) ) {
            streams.insert( cmd.contactName(), data );
        }
        activeStream[ cmd.contactName() ] = cmd.streamNum();
        emit newStreamCreated( cmd.contactName() );
        return;
    }
}



int skype::waitForResponse( QString cID ) {
    waitingForResponse = true;
    waitForResponseID = cID;
    //QTimer *timer = new QTimer(this);
    QTimer::singleShot(TimeOut, this, SLOT(timeOut()));
    int result = localEventLoop.exec();
    waitingForResponse = false;
    return result;
}

bool skype::doCommand(QString cmd, bool blocking) {
    QString cID = skypeCommand::prependID( cmd );
    QString ID = skypeCommand::getID( cID );
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


bool skype::connectToSkype() { 
    if ( this->mConnected ) return true;
    if ( ! sk.attachToSkype() ) return false;
    QObject::connect(&sk, SIGNAL(newMsgFromSkype(const QString)), this, SLOT(processMessage(const QString)));
    // if ( ! doCommand( skypeCommand::CONNECT_TO_SKYPE(appName) ) ) return false;
    if ( ! doCommand( skypeCommand::CONNECT_TO_SKYPE(this->appPrefix) ) ) return false;
    if ( ! doCommand( skypeCommand::PROTOCOL(50) ) ) return false;
    // if ( ! doCommand( skypeCommand::CREATE_AP2AP(appName) ) ) return false;
    connect( pingTimer, SIGNAL( timeout() ), this, SLOT( ping() ) );
    pingTimer->start(20000);
    this->mConnected = true;
    return true;
}

bool skype::disconnectFromSkype() {

// #ifdef Q_WS_WIN
// #undef DELETE
// #endif

    if ( ! this->mConnected) return true;
    // if ( ! doCommand( skypeCommand::DELETE_AP2AP(appName) ) ) return false;
    disconnect( &sk, 0, this, 0 );
    pingTimer->stop();
    disconnect(pingTimer, 0, 0, 0 );
    this->mConnected = false;
    return true;
}


void skype::newStream(QString contact) { 
    doCommand( skypeCommand::CONNECT_AP2AP( appName, contact ) );
}

bool skype::writeToStream(QByteArray data, QString contactName ) {
    if ( ! activeStream.contains( contactName ) )  return false; // We are not connected to contactName

    doCommand( skypeCommand::WRITE_AP2AP( appName, contactName, activeStream[contactName],data ), false );
    return true;
}

QByteArray skype::readFromStream(QString contactName) {
    QByteArray ret;
    ret.clear();
    if ( streams.contains( contactName ) ) { 
        //   qDebug() << "DEBUG: streams["<<contactName<<"]="<< streams[contactName];
        ret.append( streams[contactName] );
        streams[contactName].clear();
    } 
    return ret;
}

void skype::onConnected(QString skypeName)
{
    Q_UNUSED(skypeName);
    // bool ok = doCommand( skypeCommand::CREATE_AP2AP(appName) );
    // Q_ASSERT(ok);
}

// #include "skype.moc"
