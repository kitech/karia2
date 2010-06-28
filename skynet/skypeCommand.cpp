/***************************************************************
 * skypeCommand.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2008-04-25.
 * @Last Change: 2008-04-25.
 * @Revision:    0.0
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/
#include "skypeCommand.h"

#include <QtCore/QString>
#include <QtCore/QRegExp>
#include <QtCore/QStringList>
#include <QtCore/QtDebug>

int skypeCommand::ID = 0;

QString skypeCommand::PING() { 
  return "PING";
}

QString skypeCommand::CONNECT_TO_SKYPE(QString AppName) { 
  return "NAME "+ AppName;
}

QString skypeCommand::CREATE(QString appName) { 
  return "CREATE APPLICATION "+appName;
}

QString skypeCommand::DELETE(QString appName) { 
  return "DELETE APPLICATION " +appName;
}

QString skypeCommand::PROTOCOL(int protocolNum) { 
  return "PROTOCOL "+QString::number(protocolNum);
}

QString skypeCommand::CONNECT(QString appName, QString contactName) { 
  return "ALTER APPLICATION "+appName+" CONNECT "+ contactName;
}

QString skypeCommand::DISCONNECT(QString appName, QString contactName, int streamNum) { 
  return "ALTER APPLICATION "+appName+" DISCNNECT " + skypeCommand::streamID(contactName,streamNum);
}

QString skypeCommand::GET_CONTACT_LIST() { 
  return "SEARCH FRIENDS";
}

QString skypeCommand::WRITE(QString appName, QString contactName, int streamNum, QByteArray data) { 
  return "ALTER APPLICATION "+ appName +" WRITE "+skypeCommand::streamID(contactName,streamNum)+" "+data;
}

QString skypeCommand::READ(QString appName, QString contactName, int streamNum) { 
  return "ALTER APPLICATION "+ appName + " READ " + skypeCommand::streamID(contactName,streamNum);
}

QString skypeCommand::prependID(QString command, QString myID) { 
  return "#"+myID+" "+command;
}

QString skypeCommand::prependID(QString command) { 
  return prependID(command, QString::number(ID++));
}

QString skypeCommand::getID(QString command) { 
  QRegExp exp;
  QStringList list;
  exp.setPattern("^#([^ ]*).*$");
  if ( exp.exactMatch(command) ) {
      list = exp.capturedTexts();
      return list[1];
  } else return "";
}


QString skypeCommand::streamID(QString contactName, int streamNum) {
  return contactName+":"+QString::number(streamNum);
}

void skypeResponse::clear() { 
  Type = SK_UNKNOWN;
  StreamNum =0; ProtocolNum=0;ErrorCode=0;
  ContactName="";AppName="";ErrorMsg="";ResponseID="";Data.clear();
  Msg="";
}

QString skypeResponse::streamID() { 
  return skypeCommand::streamID(ContactName, StreamNum);
}

QString skypeResponse::_debugState() {
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

void skypeCommand::removeID(QString &msg) { 
  msg.remove( QRegExp("^#[^ ]* ") );
} 


bool skypeResponse::parse(QString msg) {
  QRegExp exp;
  QStringList list;
  QString CommandName;
  bool tmp=false, echo=false;
  clear();
  Msg=msg;
  ResponseID = skypeCommand::getID(Msg);
  skypeCommand::removeID(msg);

  if ( msg == "" ) { Type=SK_NO_COMMAND; return true;}

  if ( msg.indexOf("PONG") == 0 ) { 
    Type = SK_PING;
    return true;
  }

  if ( msg.indexOf("USERS ") == 0 ) {
    msg.remove( QRegExp("^USERS ") );
    Contacts = msg.split( ", ", QString::SkipEmptyParts );
    Type = SK_CONTACT_LIST;
    return true;
  }

  if ( msg.indexOf("PROTOCOL") == 0 ) { // PROTOCOL
    Type = SK_INFO;
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
  
  echo = true; // Except for APPLICATION everything should be of type SK_ECHO
  if ( msg.indexOf("ALTER") == 0 ) {
       exp.setPattern("ALTER APPLICATION ([^ ]*) ([^ ]*) *([^ ]*) *(.*)");
  } else if ( msg.indexOf("APPLICATION") == 0 ) {
    exp.setPattern("APPLICATION ([^ ]*) ([^ ]*) *([^ ]*) *(.*)");
    echo=false; // We need to determine Type
  } else if ( msg.indexOf("DELETE") == 0 ) { 
    exp.setPattern("DELETE ([^ ]*) ([^ ]*) *([^ ]*) *(.*)");    
  } else if ( msg.indexOf("CREATE") == 0 ) { 
    exp.setPattern("CREATE APPLICATION ([^ ]*) *([^ ]*) *(.*)"); 
  } else {
    Type=SK_UNKNOWN;
    return true;
  }

  if ( ! exp.exactMatch(msg) ) { 
    Type=SK_PARSE_ERROR;
    return false;
  }

  list = exp.capturedTexts();
  if (list.size() > 1) AppName = list[1];
  if (list.size() > 2) CommandName = list[2];
  if (list.size() > 3) ContactName = list[3];
  if (list.size() > 4) Data.append(list[4]);
  StreamNum=0;
  exp.setPattern("([^:]*):([0-9]*).*");
  if ( exp.exactMatch(ContactName) ) {
    list = exp.capturedTexts();
    ContactName = list[1];
    StreamNum = list[2].toInt(&tmp);
  }
 
  if ( echo && CommandName != "READ" ) Type = SK_ECHO;
  else  if ( CommandName == "CONNECTING" || CommandName == "SENDING" ) Type = SK_STATUS;
  else if ( CommandName == "STREAMS" ) Type = SK_STREAMS;
  else if ( CommandName == "READ" ) Type = SK_DATA;
  else if ( CommandName == "RECEIVED" ) Type = SK_READY_TO_READ;
  else Type = SK_UNKNOWN;

  if (Type == SK_READY_TO_READ && ContactName == "") Type = SK_END_OF_DATA;
  else if (Type == SK_STREAMS && ContactName == "") Type = SK_CLOSE_STREAM;
  return true;
}

skypeResponse::skypeResponse() { 
  clear();
}



