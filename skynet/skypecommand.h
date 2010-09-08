#ifndef _SKYPECOMMAND_H
#define _SKYPECOMMAND_H

/***************************************************************
 * skypeCommand.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2008-04-25.
 * @Last Change: 2008-04-25.
 * @Revision:    $Id$
 * Description:
 * Usage:
 * TODO:
 * CHANGES:
 ***************************************************************/

#include <QtCore/QString>
#include <QtCore/QStringList>

enum SkypeResponses { SK_OK, SK_ERROR, SK_INFO, SK_READY_TO_READ, 
                      SK_DATA, SK_DATAGRAM, SK_STREAMS, SK_UNKNOWN, SK_NO_COMMAND, 
                      SK_STATUS, SK_ECHO, SK_END_OF_DATA, SK_CLOSE_STREAM, 
                      SK_PARSE_ERROR, SK_PING, SK_CONTACT_LIST,
                      SK_PROTOCOL,SK_CALL,
                      SK_CURRENTUSERHANDLE
};

enum SkypeCommands{
    CK_UNKNOWN, CK_OK, CK_ERROR, CK_PROTOCOL, CK_CURRENTUSERHANDLE, CK_USER,
    CK_GROUP, CK_APPLICATION
};

enum SkypeStatus {
    SS_OFFLINE, SS_INVISIBLE, SS_DND, SS_AWAY, SS_ONLINE
};

class SkypeResponse {
public:
private:
    enum SkypeResponses Type;
    enum SkypeCommands cmd;
    QString Msg;

    // user object

    // message object

    // application object
    int StreamNum, ProtocolNum, ErrorCode;
    QString ContactName, AppName;
    QString ErrorMsg;
    QString ResponseID;
    QByteArray Data;
    QStringList Contacts;

    // call object
    QString CallID;
    QString CallStatusKey;
    QString CallStatusValue;
    QString CallDuration;

    void clear();

public:
    SkypeResponse();
    virtual ~SkypeResponse();
    bool parse(QString msg);
    bool parseMisc(QString msg);
    bool parseApp(QString msg);
    bool parseCall(QString msg);

    QString responseID() {return ResponseID;};
    int protocolNum() {return ProtocolNum;};
    int streamNum() {return StreamNum;};
    QString appName() { return AppName; };
    QString contactName() {return ContactName;};
    QByteArray data() {return Data;};
    QStringList getContacts() { return Contacts; };
    int errorCode() {return ErrorCode;};
    QString errorMsg() {return ErrorMsg;};
    enum SkypeResponses type() {return Type;};
    QString streamID(); // returns contactName:streamNum
    QString _debugState();

    QString callID() { return this->CallID; }
    QString callDuration() { return this->CallDuration; }
    QString callStatusKey() { return this->CallStatusKey; }
    QString callStatusValue() { return this->CallStatusValue; }
};

class SkypeCommand {
private:
    static int ID;
public:
    static QString CONNECT_TO_SKYPE(QString AppName);
    static QString CREATE_AP2AP(QString appName);
    static QString DELETE_AP2AP(QString AppObject);
    static QString CONNECT_AP2AP(QString appName, QString contactName);
    static QString GET_CONTACT_LIST();
    static QString WRITE_AP2AP(QString appName, QString contactName, int streamNum, QByteArray data);
    static QString READ_AP2AP(QString appName, QString contactName, int streamNum);
    static QString SEND_AP2AP(QString appName, QString contactName, int streamNum, QString data);
    static QString RECV_AP2AP(QString appName, QString contactName, int streamNum);
    static QString DISCONNECT_AP2AP(QString appName, QString contactName, int streamNum);
    static QString PING();
    static QString PROTOCOL(int protocolNum);
    static QString CALL(QString contactName);
    static QString GET_CALL_PROP(QString callID, QString propName);
    static QString prependID(QString command, QString myID);
    static QString prependID(QString command);
    static QString nextID();
    static QString getID(QString command);
    static QString streamID(QString contactName, int streamNum);
    static void    removeID(QString &msg);
};





#endif /* _SKYPECOMMAND_H */
