#ifndef _SKYPE_H
#define _SKYPE_H

/***************************************************************
 * skype.h
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
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include "skypecommand.h"
#include "skypecommon.h"

// TODO to async mode
class Skype : public QObject { 
    Q_OBJECT;
private:
    skypeComm sk;

    QString skypeName;
    QString appPrefix;
    QString appName;
    int protocolNumber;
    bool mConnected;
    QHash<QString, QByteArray> streams; // 
    QHash<QString, int> activeStream; // <skype_id, stream_id>
    QHash<int, QString> dataGrams;// <stream_id, udp_data>

    QStringList contacts;
    bool contactsUpToDate;

    bool waitingForResponse;
    QString waitForResponseID;
    QEventLoop localEventLoop;
    QTimer *pingTimer;
    int TimeOut;
    SkypeResponse response;

protected:
    void readIncomingData(QString contact, int stream);
    bool doCommand(QString cmd, bool blocking = true);
    int waitForResponse ( QString commandID );

public:
    Skype(QString AppName);
    virtual ~Skype();
    bool connectToSkype();
    bool disconnectFromSkype();
    void newStream(QString contact);
    bool writeToStream(QByteArray data, QString contactName); //deprecated
    bool writeToSock(QString contactName, QByteArray data) { return writeToStream( data, contactName ); };
    QByteArray readFromStream(QString contact);
    bool sendPackage(QString contactName, int streamNum, QString data);
    bool sendPackage(QString contactName, QString data);

    QStringList getContacts();

public slots:
    void onCommandRequest(QString cmd);

signals:
    void connected(QString skypeName);
    void connectionLost();
    void skypeError(int errNo, QString Msg);
    void dataInStream(QString contactName);
    void newStreamCreated(QString contactName);

    void commandRequest(QString cmd);
    void commandResponse(QString cmd);

    void packageSent(QString contactName, QString data);
    void packageArrived(QString contactName, int stream, QString data);


protected slots:
    void onConnected(QString skypeName);
    void processMessage(const QString &message);
    void processAP2APMessage(const QString &message);
    void timeOut();
    void ping();
};



#endif /* _SKYPE_H */
