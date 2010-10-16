/***************************************************************
 * SkypeCommand.cpp
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
#include "skypecommand.h"

#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QtDebug>

int SkypeCommand::ID = 0;

// TODO group by skype api object

QString SkypeCommand::PING() { 
    return "PING";
}

QString SkypeCommand::PUBLISH_SA_NAME(QString appName) { 
    return "NAME " + appName;
}

QString SkypeCommand::CREATE_AP2AP(QString appName) { 
    return "CREATE APPLICATION " + appName;
}

QString SkypeCommand::DELETE_AP2AP(QString appName) { 
    return "DELETE APPLICATION " +appName;
}

QString SkypeCommand::PROTOCOL(int protocolNum) { 
    return "PROTOCOL "+QString::number(protocolNum);
}

QString SkypeCommand::CONNECT_AP2AP(QString appName, QString contactName) { 
    return "ALTER APPLICATION "+appName+" CONNECT "+ contactName;
}

QString SkypeCommand::DISCONNECT_AP2AP(QString appName, QString contactName, int streamNum) { 
    return "ALTER APPLICATION "+appName+" DISCNNECT " + SkypeCommand::streamID(contactName,streamNum);
}

QString SkypeCommand::GET_CONTACT_LIST() { 
    return "SEARCH FRIENDS";
}

QString SkypeCommand::WRITE_AP2AP(QString appName, QString contactName, int streamNum, QByteArray data) { 
    return "ALTER APPLICATION "+ appName +" WRITE "+SkypeCommand::streamID(contactName,streamNum)+" "+data;
}

QString SkypeCommand::READ_AP2AP(QString appName, QString contactName, int streamNum) { 
    return "ALTER APPLICATION "+ appName + " READ " + SkypeCommand::streamID(contactName,streamNum);
}

QString SkypeCommand::SEND_AP2AP(QString appName, QString contactName, int streamNum, QString data)
{
    return QString("ALTER APPLICATION %1 DATAGRAM %2:%3 %4")
        .arg(appName).arg(contactName).arg(streamNum).arg(data);
}

QString SkypeCommand::RECV_AP2AP(QString appName, QString contactName, int streamNum)
{
    return QString();
}

QString SkypeCommand::CALL(QString contactName)
{
    return QString("CALL %1").arg(contactName);
}

QString SkypeCommand::GET_CALL_PROP(QString callID, QString propName)
{
    return QString("GET CALL %1 %2").arg(callID).arg(propName);
}

QString SkypeCommand::SET_CALL_PROP(QString callID, QString propName, QString propValue)
{
    return QString("SET CALL %1 %2 %3").arg(callID).arg(propName).arg(propValue);
}

QString SkypeCommand::ALTER_CALL_STATUS(QString callID, QString propName)
{
    return QString("ALTER CALL %1 %2").arg(callID).arg(propName);
}

QString SkypeCommand::ALTER_CALL_SET_INPUT_PORT(QString callID, QString port)
{
    return QString("ALTER CALL %1 SET_INPUT PORT=\"%2\"").arg(callID).arg(port);
}

QString SkypeCommand::ALTER_CALL_SET_OUTPUT_PORT(QString callID, QString port)
{
    return QString("ALTER CALL %1 SET_OUTPUT PORT=\"%2\"").arg(callID).arg(port);
}


QString SkypeCommand::prependID(QString command, QString myID) { 
    return "#"+myID+" "+command;
}

QString SkypeCommand::prependID(QString command) { 
    return prependID(command, QString::number(ID += 2));
}

QString SkypeCommand::nextID()
{
    return QString::number(ID += 2);
}

QString SkypeCommand::getID(QString command) { 
    QRegExp exp;
    QStringList list;
    exp.setPattern("^#([^ ]*).*$");
    if ( exp.exactMatch(command) ) {
        list = exp.capturedTexts();
        return list[1];
    } else return "";
}


QString SkypeCommand::streamID(QString contactName, int streamNum) {
    return contactName+":"+QString::number(streamNum);
}


///////////////
//
///////////////
void SkypeResponse::clear() { 
    Type = SK_UNKNOWN;
    StreamNum =0; ProtocolNum=0;ErrorCode=0;
    ContactName="";AppName="";ErrorMsg="";ResponseID="";Data.clear();
    Msg="";
}

QString SkypeResponse::streamID() { 
    return SkypeCommand::streamID(ContactName, StreamNum);
}

QString SkypeResponse::_debugState() {
    switch (Type) { 
    case SK_OK: return "OK";
    case SK_ERROR: return "ERROR "+QString::number(ErrorCode)+": "+ErrorMsg;
    case SK_INFO: return "INFO: "+Data;
    case SK_READY_TO_READ:return "DATA READY: "+streamID();
    case SK_DATA:return "DATA ("+streamID()+"): "+Data;
    case SK_STREAMS: return "NEW STREAM: "+streamID();
    case SK_UNKNOWN: return "UNKNOWN: " + Msg;
    case SK_NO_COMMAND: return "NO COMMAND";
    case SK_ECHO: return "ECHO: "+Msg;
    case SK_STATUS: return "STATUS: "+Data;
    case SK_END_OF_DATA: return "END OF DATA";
    case SK_CLOSE_STREAM: return "CLOSE STREAM";
    case SK_PARSE_ERROR: return "PARSE ERROR: "+Msg;
    default: return "";
    }
}

void SkypeCommand::removeID(QString &msg) { 
    msg.remove( QRegExp("^#[^ ]* ") );
} 


bool SkypeResponse::parse(QString msg) {
    QRegExp exp;
    QStringList list;
    QString CommandName;
    bool tmp=false, echo=false;
    clear();
    Msg=msg;
    ResponseID = SkypeCommand::getID(Msg);
    SkypeCommand::removeID(msg);

    if ( msg == "" ) { Type=SK_NO_COMMAND; return true;}

    if ( msg.indexOf("PONG") == 0 ) { 
        Type = SK_PING;
        return true;
    }

    // TODO less match should put at end
    if (msg.indexOf("CONNSTATUS") == 0) {
        Type = SK_CONNSTATUS;
        this->StatusText = msg.split(" ").at(1);
        if (msg.indexOf("ONLINE") != -1) {
            this->StatusType = SS_ONLINE;
        } else if (msg.indexOf("OFFLINE") != -1) {
            this->StatusType = SS_OFFLINE;
        } else if (msg.indexOf("AWAY") != -1) {
            this->StatusType = SS_AWAY;
        } else if (msg.indexOf("INVISIBLE") != -1) {
            this->StatusType = SS_INVISIBLE;
        } else {
            this->StatusType = SS_UNKNOWN;
        }
        return true;
    }

    if (msg.indexOf("USER ") == 0) {
        this->Type = SK_USER;
        return true;
    }

    if (msg.indexOf("GROUP ") == 0) {
        this->Type = SK_GROUP;
        return true;
    }

    if (msg.indexOf("USERSTATUS") == 0) {
        this->Type = SK_USERSTATUS;
        return true;
    }

    if ( msg.indexOf("USERS ") == 0 ) {
        msg.remove( QRegExp("^USERS ") );
        Contacts = msg.split( ", ", QString::SkipEmptyParts );
        Type = SK_CONTACT_LIST;
        return true;
    }

    if ( msg.indexOf("PROTOCOL") == 0 ) { // PROTOCOL
        // Type = SK_INFO;
        Type = SK_PROTOCOL;
        list = msg.split(" ");
        this->ProtocolNum = list.at(1).toInt();
        // qDebug()<<"ProtocolNum:"<<this->ProtocolNum;
        return true;
    }

    if ( msg.indexOf("OK") == 0 ) { // OK
        Type = SK_OK;
        return true;
    }

    if ( msg.indexOf("NAME") == 0 ) { // NAME
        Type = SK_ECHO;
        return true;
    }

    if ( msg.indexOf("ERROR") == 0 ) { // error
        Type = SK_ERROR;
        exp.setPattern("^ERROR ([0-9]*) *([^:]*)[:]* *(.*)$");
        if ( exp.exactMatch(msg) ) {
            list = exp.capturedTexts();
            ErrorCode = list[1].toInt(&tmp);
            CommandName = list[2];
            ErrorMsg = list[3];
            return true;
        } else {
            ErrorCode = -1;
            ErrorMsg = "Unknown Skype error";
            return true;
        }
    } 

    if (msg.startsWith("CURRENTUSERHANDLE")) {
        this->Type = SK_CURRENTUSERHANDLE;
        list = msg.split(" ");
        this->ContactName = list.at(1);
        // qDebug()<<"Got my name"<<this->ContactName;
        return true;
    }

    echo = true; // Except for APPLICATION everything should be of type SK_ECHO
    if ( msg.indexOf("ALTER APPLICATION") == 0 ) {
        exp.setPattern("ALTER APPLICATION ([^ ]*) ([^ ]*) *([^ ]*) *(.*)");
    } else if ( msg.indexOf("APPLICATION") == 0 ) {
        // exp.setPattern("APPLICATION ([^ ]*) ([^ ]*) *([^ ]*) *(.*)");
        exp.setPattern("APPLICATION ([^ ]*) ([^ ]*) *([^ ]*) *(.*)");
        echo=false; // We need to determine Type
    } else if ( msg.indexOf("DELETE") == 0 ) { 
        exp.setPattern("DELETE ([^ ]*) ([^ ]*) *([^ ]*) *(.*)");    
    } else if ( msg.indexOf("CREATE") == 0 ) { 
        exp.setPattern("CREATE APPLICATION ([^ ]*) *([^ ]*) *(.*)"); 
    } else if (msg.indexOf("CALL") == 0
               || msg.indexOf("ALTER CALL") == 0) {
        return this->parseCall(msg);
    } else {
        Type = SK_UNKNOWN;
        return true;
    }

    if ( ! exp.exactMatch(msg) ) { 
        Type = SK_PARSE_ERROR;
        return false;
    }

    list = exp.capturedTexts();
    if (list.size() > 1) AppName = list[1];
    if (list.size() > 2) CommandName = list[2];
    if (list.size() > 3) ContactName = list[3];
    if (list.size() > 4) {
        // why have a space at begin?
        if (Data.isEmpty() && list[4].length() > 0 && list[4].at(0) == QChar(' ')) {
            Data.append(list[4].trimmed());
        } else {
            Data.append(list[4]);
        }
    }
    StreamNum = 0;
    exp.setPattern("([^:]*):([0-9]*).*");
    if ( exp.exactMatch(ContactName) ) {
        list = exp.capturedTexts();
        ContactName = list[1];
        StreamNum = list[2].toInt(&tmp);
    }

    if ( echo && CommandName != "READ" ) Type = SK_ECHO;
    else  if ( CommandName == "CONNECTING" || CommandName == "SENDING" ) Type = SK_STATUS;
    else if ( CommandName == "STREAMS" ) {
        Type = SK_STREAMS;
    }
    else if ( CommandName == "READ" ) Type = SK_DATA;
    else if ( CommandName == "RECEIVED" ) Type = SK_READY_TO_READ;
    else if (CommandName == "DATAGRAM") {
        Type = SK_DATAGRAM;
    }
    else Type = SK_UNKNOWN;

    // qDebug()<<__FILE__<<__LINE__<<"hhhhhhhhhhhhh"<<this->Type<<CommandName;    

    if (Type == SK_READY_TO_READ && ContactName == "") Type = SK_END_OF_DATA;
    else if (Type == SK_STREAMS && ContactName == "") Type = SK_CLOSE_STREAM;
    return true;
}
bool SkypeResponse::parseMisc(QString msg)
{
    
}

bool SkypeResponse::parseApp(QString msg)
{
    
}

bool SkypeResponse::parseCall(QString msg)
{
    QRegExp exp;
    QStringList list;
    QString CommandName;
    bool tmp=false, echo=false;
    clear();
    Msg=msg;
    ResponseID = SkypeCommand::getID(Msg);
    SkypeCommand::removeID(msg);

    // TODO , can not paser ALTER CALL <id> ANSWER response
    echo = true; // Except for APPLICATION everything should be of type SK_ECHO
    if (msg.indexOf("CALL") == 0) {
        exp.setPattern("CALL ([^ ]*) ([^ ]*) ([^ ]*)");
    } else if(msg.indexOf("ALTER") == 0) {
        exp.setPattern("ALTER CALL ([^ ]*) ([^ ]*)");
    } else {
        Type = SK_UNKNOWN;
        return true;
    }

    if ( ! exp.exactMatch(msg) ) { 
        Type = SK_PARSE_ERROR;
        return false;
    }

    list = exp.capturedTexts();
    if (list.size() > 1) this->CallID = list[1];
    if (list.size() > 2) this->CallStatusKey = list[2];
    if (list.size() > 3) this->CallStatusValue = list[3];

    this->Type = SK_CALL;
    
    return true;
}

SkypeResponse::SkypeResponse() { 
    clear();
}
SkypeResponse::~SkypeResponse()
{
}



