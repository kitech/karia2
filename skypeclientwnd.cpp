#include <QtCore>
#include "skypeclientwnd.h"


/////////Skype control const begin
#define SKYPE_SUPPORTED_PROTOCOL	5

//------------------------------------------------------------------------------
// User Status -----------------------------------------------------------------
//enum TSkypeUserStatus{ usUnknown, usOnline, usOffline, usAway, usNA, usDND,usInvisible, usLoggedOut, usSkypeMe } ;
const char * USERSTATUS_STRING[] = {"UNKNOWN","ONLINE","OFFLINE","AWAY","NA","DND","INVISIBLE","LOGGEDOUT","SKYPEME"};

const TSkypeUserStatus READABLE_USERSTATUS[] = { usUnknown, usOnline, usOffline, usAway, usNA, usDND, usInvisible, usLoggedOut, usSkypeMe} ;

const TSkypeUserStatus WRITEABLE_USERSTATUS [] = { usOnline, usOffline, usAway, usNA, usDND, usInvisible, usSkypeMe } ;

//------------------------------------------------------------------------------
// Connection Status -----------------------------------------------------------
//enum TSkypeConnStatus  { csOffline, csConnecting, csPausing, csOnline, csLoggedOut };
const  char *  CONNSTATUS_STRING[] = { "OFFLINE","CONNECTING","PAUSING","ONLINE","LOGGEDOUT" } ;
const TSkypeConnStatus READABLE_CONNSTATUS[] = { csOffline, csConnecting, csPausing, csOnline , csLoggedOut } ;
const TSkypeConnStatus WRITEABLE_CONNSTATUS[] = { csOffline  } ;

//------------------------------------------------------------------------------
// Searchtypes -----------------------------------------------------------------
//enum TSkypeSearchType { stUsers, stFriends, stCalls, stActiveCalls,
 //                        stMissedCalls, stMessages, stMissedMessages, stChats,
//                         stActiveChats, stMissedChats, stRecentChats,
//						 stBookmarkedChats, stChatMsgs, stMissedChatMsgs } ;

const char * SEARCHTYPE_STRING[] = { "USERS","FRIENDS","CALLS","ACTIVECALLS","MISSEDCALLS",
                         "MESSAGES","MISSEDMESSAGES","CHATS","ACTIVECHATS",
                         "MISSEDCHATS","RECENTCHATS","BOOKMARKEDCHATS",
						 "CHATMESSAGES","MISSEDCHATMESSAGES" } ;

const TSkypeSearchType SEARCHABLE_TYPE [] = { stUsers, stFriends, stCalls, stActiveCalls,
                         stMissedCalls, stMissedMessages, stChats,
                         stActiveChats, stMissedChats, stRecentChats,
                         stBookmarkedChats, stChatMsgs,
						 stMissedChatMsgs } ; // stMessages, stActiveChats,
                                             // stMissedChats, stRecentChats,
                                             // stBookmarkedChats don"t work
                                             // at the moment (Skype 1.2 Beta)

//------------------------------------------------------------------------------
// User Properties -------------------------------------------------------------

//enum TSkypeUserProperty { upHandle, upFullname, upBirthday, upSex, upLanguage,
//                           upCountry, upProvince, upCity, upPhoneHome,
//                           upPhoneOffice, upPhoneMobile, upHomepage, upAbout,
//                           upHasCallEquipment, upBuddyStatus, upIsAuthorized,
//                           upIsBlocked, upDisplayname, upOnlineStatus,
//						   upLastOnlineTimeStamp } ;

const char * USERPROPERTY_STRING[] = { "HANDLE","FULLNAME","BIRTHDAY","SEX","LANGUAGE",
                           "COUNTRY","PROVINCE","CITY","PHONE_HOME",
                           "PHONE_OFFICE","PHONE_MOBILE","HOMEPAGE","ABOUT",
                           "HASCALLEQUIPMENT","BUDDYSTATUS","ISAUTHORIZED",
                           "ISBLOCKED","DISPLAYNAME","ONLINESTATUS",
						   "LASTONLINETIMESTAMP" } ;

const TSkypeUserProperty READABLE_USERPROPERTY [] =  // Protocol 4
							{ upHandle, upFullname, upBirthday, upSex, upLanguage,
                            upCountry, upProvince, upCity, upPhoneHome,
                            upPhoneOffice, upPhoneMobile, upHomepage, upAbout,
                            upHasCallEquipment, upBuddyStatus, upIsAuthorized,
                            upIsBlocked, upDisplayname, upOnlineStatus,
							upLastOnlineTimeStamp } ;

const TSkypeUserProperty WRITEABLE_USERPROPERTY[] = { upHandle } ;

//------------------------------------------------------------------------------
// Call Properties -------------------------------------------------------------

//enum TSkypeCallProperty { cpTimestamp, cpPartnerHandle, cpPartnerDispname,
//                           cpConfId, cpJoinConference, cpConfParticipant,
//                           cpConfParticipantsCount, cpType, cpStatus,
//                           cpFailureReason, cpSubject, cpPSTNNumber, cpDuration,
//						   cpSeen, cpDTMF, cpPSTNStatus } ;

const char * CALLPROPERTY_STRING[] = { "TIMESTAMP","PARTNER_HANDLE","PARTNER_DISPNAME",
                           "CONF_ID","JOIN_CONFERENCE","CONF_PARTICIPANT",
                           "CONF_PARTICIPANTS_COUNT","TYPE","STATUS",
                           "FAILUREREASON","SUBJECT","PSTN_NUMBER","DURATION",
						   "SEEN","DTMF","PSTN_STATUS" } ;

const TSkypeCallProperty READABLE_CALLPROPERTY[] = { 
							cpTimestamp, cpPartnerHandle, cpPartnerDispname,
                            cpConfId, cpConfParticipant,
                            cpConfParticipantsCount, cpType, cpStatus,
                            cpFailureReason, cpSubject, cpPSTNNumber,
							cpDuration, cpPSTNStatus } ;


const TSkypeCallProperty WRITEABLE_CALLPROPERTY[] = { cpStatus,cpSeen,cpDTMF,cpJoinConference } ;

//------------------------------------------------------------------------------
// Message Properties ----------------------------------------------------------

//enum TSkypeMessageProperty  { mpTimestamp, mpPartnerHandle, mpPartnerDispname,
//                              mpConfId, mpType, mpStatus, mpSeen,
//							  mpFailureReason, mpBody } ;
//
const char * MESSAGEPROPERTY_STRING[] = { "TIMESTAMP","PARTNER_HANDLE","PARTNER_DISPNAME",
                              "CONF_ID","TYPE","STATUS","SEEN",
							  "FAILUREREASON","BODY" } ;

const TSkypeMessageProperty READABLE_MESSAGEPROPERTY[] = { mpTimestamp } ; // since every MsgID is a ChatMsgID you can"t use
                              // GET MESSAGE in Protocol 4 in a meaningful way
                              // at the moment...

const TSkypeMessageProperty WRITEABLE_MESSAGEPROPERTY [] = { mpTimestamp } ;

//------------------------------------------------------------------------------
// Chat Properties -------------------------------------------------------------

//enum TSkypeChatProperty  { chName, chTimestamp, chAdder, chStatus, chPosters,
//                           chMembers, chTopic, chChatMessages, chActiveMembers,
//						   chFriendlyname } ;

const char * CHATPROPERTY_STRING [] = { "NAME","TIMESTAMP","ADDER","STATUS","POSTERS",
                           "MEMBERS","TOPIC","CHATMESSAGES","ACTIVEMEMBERS",
						   "FRIENDLYNAME" } ;

const TSkypeChatProperty READABLE_CHATPROPERTY[] = { chName, chTimestamp, chAdder, chStatus, chPosters,
                          chMembers, chTopic, chChatMessages, chActiveMembers,
						  chFriendlyname } ; // CHATMESSAGES doesn"t work at the
                                            // moment (Skype 1.2 Beta)

const TSkypeChatProperty WRITEABLE_CHATPROPERTY[] = {chName } ;

//------------------------------------------------------------------------------
// Chat Message Properties -----------------------------------------------------

//enum TSkypeChatMsgProperty { cmChatname, cmTimestamp, cmFromHandle,
//                              cmFromDispname, cmType, cmUsers, cmLeaveReason,
//							  cmBody, cmStatus, cmSeen } ;

const char * CHATMSGPROPERTY_STRING [] = { "CHATNAME","TIMESTAMP","FROM_HANDLE",
                              "FROM_DISPNAME","TYPE","USERS","LEAVEREASON",
							  "BODY","STATUS","SEEN" } ;

const TSkypeChatMsgProperty READABLE_CHATMSGPROPERTY[] = { cmChatname, cmTimestamp, cmFromHandle,
		cmFromDispname, cmType, cmUsers, cmLeaveReason, cmBody, cmStatus } ;

const TSkypeChatMsgProperty WRITEABLE_CHATMSGPROPERTY [] = { cmSeen } ;


//------------------------------------------------------------------------------
// Skype Privileges ------------------------------------------------------------
const  char * PRIVILEGE_STRING[] = {"SKYPEOUT"};

//------------------------------------------------------------------------------
// Skype Profiles --------------------------------------------------------------
const char * PROFILE_STRING[] = 
		{"PSTN_BALANCE","PSTN_BALANCE_CURRENCY"} ;

//------------------------------------------------------------------------------
// Event Types -----------------------------------------------------------------

/*  变成Qt的信号 。
  TSkypeErrorNotify = procedure(Code : integer; Description : string) of object;

  TSkypeUserStatusNotify = procedure(UserStatus : TSkypeUserStatus) of object;

  TSkypeConnStatusNotify = procedure(ConnStatus : TSkypeConnStatus) of object;

  TSkypeCurrentUserHandleNotify = procedure(UserHandle : string) of object;

  TSkypeSearchNotify = procedure(SearchType : TSkypeSearchType;
                                 aList : TStringList) of object;

  TSkypeUserNotify = procedure(Username : string;
                               UserProperty : TSkypeUserProperty;
                               data : string) of object;

  TSkypeNameNotify = procedure(Name : string) of object;

  TSkypeProtocolNotify = procedure(ProtocolNumber : integer) of object;

  TSkypeCallNotify = procedure(CallId : Integer;
                               CallProperty : TSkypeCallProperty;
                               data : string) of object;

  TSkypeMessageNotify = procedure(MessageID : Integer;
                                  MessageProperty : TSkypeMessageProperty;
                                  data : string) of object;

  TSkypeChatNotify = procedure(Chatname : string;
                               ChatProperty : TSkypeChatProperty;
                               data : string) of object;

  TSkypeChatMsgNotify = procedure(ChatMsgID : Integer;
                                  ChatMsgProperty : TSkypeChatMsgProperty;
                                  data : string) of object;

  TSkypeAudioNotify = procedure(Device : string) of object;

  TSkypeMuteNotify = procedure(Muted : boolean) of object;

  TSkypePrivilegeNotify = procedure(priv : TSkypePrivilege;
                                    granted : boolean) of object;

  TSkypeProfileNotify = procedure(priv : TSkypeProfile;
                                  data : string) of object;

  TSkypeVersionNotify = procedure(Version : string) of object;

  TNotSupportedErrorNotify = procedure(errormsg : string) of object;

  TSkypeCopyData = procedure(rawdata : string) of object;
*/

/////////Skype control const end 

/////////////////////////////////
#ifdef WIN32
/////////////////

#include <windows.h>
#include <process.h>
#include <rpcdce.h>
#define SKYPECONTROLAPI_ATTACH_SUCCESS  0
#define SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION   1
#define SKYPECONTROLAPI_ATTACH_REFUSED   2
#define SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE   3
#define SKYPECONTROLAPI_ATTACH_API_AVAILABLE  0x8001

HWND hInit_MainWindowHandle;
HINSTANCE hInit_ProcessHandle;
char acInit_WindowClassName[128];
HANDLE hGlobal_ThreadShutdownEvent;
bool volatile fGlobal_ThreadRunning=true;
UINT uiGlobal_MsgID_SkypeControlAPIAttach;
UINT uiGlobal_MsgID_SkypeControlAPIDiscover;
HWND hGlobal_SkypeAPIWindowHandle=NULL;
#if defined(_DEBUG)
	bool volatile fGlobal_DumpWindowsMessages=true;
#else
	bool volatile fGlobal_DumpWindowsMessages=false;
#endif
DWORD ulGlobal_PromptConsoleMode=0;

HANDLE volatile hGlobal_PromptConsoleHandle=NULL;

////////////////////////
#endif
/////////////////////////////////


/////////////
SkypeClientWnd::SkypeClientWnd(QWidget *parent)
    : QWidget(parent)
{
	ui.setupUi(this);
	mReconnectTimer.start(3000);

	//////////////////////////////////
	/////////////////////////////////
#ifdef WIN32
	/////////////////
	//////////////////////////////////
	///skype
	this->registerSkypeWindowMessage();
	::hInit_MainWindowHandle = this->winId();

	if( SendMessage( HWND_BROADCAST, uiGlobal_MsgID_SkypeControlAPIDiscover, (WPARAM)hInit_MainWindowHandle, 0)!=0 )
	{
		qDebug()<<"Sendmesagee OK";
		//SendMessage( HWND_BROADCAST, uiGlobal_MsgID_SkypeControlAPIDiscover, (WPARAM)hInit_MainWindowHandle, 0);
	}	
	else
	{
		//qDebug()<<"sendmessage error";
	}
	qDebug()<<"Lerror: "<<::GetLastError();

	//signal
	//QObject::connect(this->ui.pushButton,SIGNAL(clicked()),this,SLOT(createSkypeApp2AppApplication()));
	//////////////////////////////////
#endif
	///////////////////////////////////
}

SkypeClientWnd::~SkypeClientWnd()
{

}

//////////////////////////////
/////////////////////////////////
#ifdef WIN32
/////////////////
//////////////////////////////
bool SkypeClientWnd::registerSkypeWindowMessage()
{
	int bRet = false ;

	::hInit_MainWindowHandle = this->winId();
	static char acInputRow[1024];
	bool fProcessed;



	uiGlobal_MsgID_SkypeControlAPIAttach=RegisterWindowMessageA("SkypeControlAPIAttach");
	uiGlobal_MsgID_SkypeControlAPIDiscover=RegisterWindowMessageA("SkypeControlAPIDiscover");
	if( uiGlobal_MsgID_SkypeControlAPIAttach!=0 && uiGlobal_MsgID_SkypeControlAPIDiscover!=0 )
	{
		qDebug()<<"attach: "<<::uiGlobal_MsgID_SkypeControlAPIAttach<<"discover: "<<::uiGlobal_MsgID_SkypeControlAPIDiscover;
		bRet =  true ;
	}
	else
	{
		qDebug()<<::GetLastError();
		bRet = false ;
	}
	if( SendMessage( HWND_BROADCAST, uiGlobal_MsgID_SkypeControlAPIDiscover, (WPARAM)hInit_MainWindowHandle, 0)!=0 )
	{
		qDebug()<<"Sendmesagee OK";
		
	}	
	else
	{
		qDebug()<<"sendmessage error";
	}
	qDebug()<<"Lerror: "<<::GetLastError();
	return bRet ;

	return true ;
}

bool SkypeClientWnd::reconnectIfDisconnected() 
{

	return true ;
}

bool SkypeClientWnd::winEvent ( MSG * message, long * result )
{

	HWND hWindow ;
	UINT uiMessage;
	WPARAM uiParam;
	LPARAM ulParam;

	///
	LRESULT lReturnCode;
	bool fIssueDefProc;

	////
	hWindow = message->hwnd;
	uiMessage = message->message;
	uiParam = message->wParam;
	ulParam = message->lParam ;

	lReturnCode=0;
	fIssueDefProc=false;
	switch(uiMessage)
		{
		case WM_DESTROY:
			hInit_MainWindowHandle=NULL;
			PostQuitMessage(0);
			break;
		case WM_COPYDATA:
			if( hGlobal_SkypeAPIWindowHandle==(HWND)uiParam )
			{
				PCOPYDATASTRUCT poCopyData=(PCOPYDATASTRUCT)ulParam;
				printf( "Message from Skype(%u): %.*s\n", poCopyData->dwData, poCopyData->cbData, poCopyData->lpData);
				lReturnCode=1;	
				*result =1 ;// Return Code must be different from zero, otherwise
								// Skype will consider the connection broken.
				this->dispatchSkypeMessage(QString((char*)(poCopyData->lpData)));
			}
			break;
		default:
			if( uiMessage==uiGlobal_MsgID_SkypeControlAPIAttach )
			{
				switch(ulParam)
					{
					case SKYPECONTROLAPI_ATTACH_SUCCESS:
						printf("!!! Connected; to terminate issue #disconnect\n");
						hGlobal_SkypeAPIWindowHandle=(HWND)uiParam;
						break;
					case SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION:
						printf("!!! Pending authorization\n");
						break;
					case SKYPECONTROLAPI_ATTACH_REFUSED:
						printf("!!! Connection refused\n");
						break;
					case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:
						printf("!!! Skype API not available\n");
						break;
					case SKYPECONTROLAPI_ATTACH_API_AVAILABLE:
						printf("!!! Try connect now (API available); issue #connect\n");
						break;
					}
				lReturnCode=1;
				*result = 1 ;
				break;
			}
			//fIssueDefProc=true;	
			
			break;
		}
	if( fIssueDefProc )	//we dont need define our defproc
		lReturnCode=DefWindowProc( hWindow, uiMessage, uiParam, ulParam);
	if( !fGlobal_DumpWindowsMessages )
	{
		printf( "WindowProc: hWindow=0x%08X, MainWindow=0x%08X, Message=%5u, WParam=0x%08X, LParam=0x%08X; Return=%ld%s\n",
				hWindow, hInit_MainWindowHandle, uiMessage, uiParam, ulParam, lReturnCode, fIssueDefProc? " (default)":"");
	}

	//*result = 1 ;

	return(lReturnCode);
//
	
	return false  ;
}


bool SkypeClientWnd::createSkypeApp2AppApplication()
{
	this->sendSkypeMessage("CREATE APPLICATION appskype");

	return true ;
}

bool SkypeClientWnd::skypeAppCreate(QString appname)
{
	this->sendSkypeMessage("CREATE APPLICATION " + appname );
	return true ;
}

	//可能对每一个可连接的用户创建一个app对象。所有appname = 对方skypeid , 也可能不行，所有还是要有两个参数的。
	//一个app应该是可能连接许多用户的。
bool SkypeClientWnd::skypeAppConnect(QString appname,QString skypeid )
{
	this->sendSkypeMessage("ALTER APPLICATION " + appname + " CONNECT " + skypeid );

	return true ;
}
bool SkypeClientWnd::skypeAppWrite(QString appname , QString skypeid,QString connid, QString text)
{
	this->sendSkypeMessage("ALTER APPLICATION " + appname + " WRITE " + skypeid + ":" + connid + " " + text  );
	return true ;
}
	//这应该有个参数来存储读取到的数据，或者把读取到的数据作为返回也是可以的。
bool SkypeClientWnd::skypeAppRead(QString appname ,QString skypeid , QString connid ) 
{
	QString data ;

	this->sendSkypeMessage("ALTER APPLICATION " + appname + " READ " + skypeid + ":" + connid    );
	return true ;
}
bool SkypeClientWnd::skypeAppSendDatagram(QString appname , QString skypeid , QString connid , QString text )
{
	this->sendSkypeMessage("ALTER APPLICATION " + appname + " DATAGRAM " + skypeid + ":" + connid + " " + text  );
	return true ;
}
bool SkypeClientWnd::skypeAppDisconnect(QString appname ,QString skypeid , QString connid)
{
	this->sendSkypeMessage("ALTER APPLICATION " + appname + " DISCONNECT " + skypeid + ":" + connid    );
	return true ;
}
bool SkypeClientWnd::skypeAppDelete(QString appname)
{
	this->sendSkypeMessage("DELETE APPLICATION " + appname   );
	return true ;
}


bool SkypeClientWnd::sendSkypeMessage(QString msg)
{
		char * cmsg = 0 ;

		cmsg = (char*)malloc(msg.length()+1);
		memset(cmsg,0,msg.length()+1);
		strcpy(cmsg,msg.toAscii().data());

				COPYDATASTRUCT oCopyData;
				bool ret = FALSE ;
				// send command to skype
				oCopyData.dwData=0;
				oCopyData.lpData=cmsg;
				oCopyData.cbData=strlen(cmsg)+1;
				if( oCopyData.cbData!=1 )
					{
					if( (ret = SendMessage( hGlobal_SkypeAPIWindowHandle, WM_COPYDATA, (WPARAM)hInit_MainWindowHandle, (LPARAM)&oCopyData))==FALSE )
						{
						hGlobal_SkypeAPIWindowHandle=NULL;
						printf("!!! Disconnected\n");
						}
						else
						{
							printf("send data : ret = %d  %s \n", ret , cmsg );
						}
					}

			free(cmsg) ; cmsg = 0 ;

	return true ;

}



/////////////////////////////////
//////////////////////////////
// Try to connect to Skype
void SkypeClientWnd::AttachToSkype( int Protocol   )  
{
	if( SendMessage( HWND_BROADCAST, uiGlobal_MsgID_SkypeControlAPIDiscover, (WPARAM)hInit_MainWindowHandle, 0)!=0 )
	{
		qDebug()<<"AttachToSkype OK";
		
	}	
	else
	{
		qDebug()<<"AttachToSkype error";
	}	
}


bool SkypeClientWnd::SendData(QString aString ) 
{
		char * cmsg = 0 ;

		cmsg = (char*)malloc(aString.length()+1);
		memset(cmsg,0,aString.length()+1);
		strcpy(cmsg,aString.toAscii().data());

				COPYDATASTRUCT oCopyData;
				bool ret = FALSE ;
				// send command to skype
				oCopyData.dwData=0;
				oCopyData.lpData=cmsg;
				oCopyData.cbData=strlen(cmsg)+1;
				if( oCopyData.cbData!=1 )
					{
					if( (ret = SendMessage( hGlobal_SkypeAPIWindowHandle, WM_COPYDATA, (WPARAM)hInit_MainWindowHandle, (LPARAM)&oCopyData))==FALSE )
						{
						hGlobal_SkypeAPIWindowHandle=NULL;
						printf("!!! Disconnected\n");
						}
						else
						{
							printf("send data : ret = %d  %s \n", ret , cmsg );
						}
					}

			free(cmsg) ; cmsg = 0 ;
	return false ;
}

#else

// Try to connect to Skype
void SkypeClientWnd::AttachToSkype( int Protocol   )  
{

}

bool SkypeClientWnd::SendData(QString aString ) 
{
	char * cmsg = 0 ;

	return false ;
}
//////////////////////////////
////////////////////////
#endif

// Disconnect from Skype
void SkypeClientWnd::DetachFromSkype()
{
  //SkypeAPIWindowHandle := 0;
  //FAttached := false;	???????????
}
void SkypeClientWnd::dispatchSkypeMessage(QString msgdata)
{
	QString Key , subdata ;

	Key = msgdata.split(" ").at(0);

	if (Key.length() >0 )
	{
		subdata = msgdata.right(msgdata.length()-Key.length()-1);
		if (Key == "ERROR")
		   EvaluateERROR(subdata) ;  else
		if (Key == "USERSTATUS")
		   EvaluateUSERSTATUS(subdata) ;  else
		if (Key == "USER")
		   EvaluateUSER(subdata) ;  else
		if (Key == "NAME")
		   EvaluateNAME(subdata) ;  else
		if (Key == "PROTOCOL")
		   EvaluatePROTOCOL(subdata) ;  else
		if (Key == "PONG")
		   EvaluatePONG(subdata) ;  else
		if (Key == "CALL")
		   EvaluateCALL(subdata) ;  else
		if (Key == "MESSAGE")
		   EvaluateMESSAGE(subdata) ;  else
		if (Key == "CONNSTATUS")
		   EvaluateCONNSTATUS(subdata) ;  else
		if (Key == "CURRENTUSERHANDLE")
		   EvaluateCURRENTUSERHANDLE(subdata) ;  else
		if (Key == "USERS")
		   EvaluateSEARCHRESULTS(stUsers,subdata) ;  else
		if (Key == "FRIENDS")
		   EvaluateSEARCHRESULTS(stFriends,subdata) ;  else
		if (Key == "CALLS")
		   EvaluateSEARCHRESULTS(stCalls,subdata) ;  else
		if (Key == "MISSEDCALLS")
		   EvaluateSEARCHRESULTS(stMissedCalls,subdata) ;  else
		if (Key == "MESSAGES")
		   EvaluateSEARCHRESULTS(stMessages,subdata) ;  else
		if (Key == "MISSEDMESSAGES")
		   EvaluateSEARCHRESULTS(stMissedMessages,subdata) ;  else
		if (Key == "CHATS")
		   EvaluateSEARCHRESULTS(stChats,subdata) ;  else
		if (Key == "CHATMESSAGES")
		   EvaluateSEARCHRESULTS(stChatMsgs,subdata) ;  else
		if (Key == "CALLHISTORYCHANGED")
		   EvaluateCALLHISTORYCHANGED(subdata) ;  else
		if (Key == "IMHISTORYCHANGED")
		   EvaluateIMHISTORYCHANGED(subdata) ;  else
		if (Key == "AUDIO_IN")
		   EvaluateAUDIO_IN(subdata) ;  else
		if (Key == "AUDIO_OUT")
		   EvaluateAUDIO_OUT(subdata) ;  else
		if (Key == "MUTE")
		   EvaluateMUTE(subdata) ;  else
		if (Key == "PRIVILEGE")
		   EvaluatePRIVILEGE(subdata) ;  else
		if (Key == "PROFILE")
		   EvaluatePROFILE(subdata) ;  else
		if (Key == "SKYPEVERSION")
		   EvaluateSKYPEVERSION(subdata) ;  else
		if (Key == "CHAT")
		   EvaluateCHAT(subdata) ;  else
		if (Key == "CHATMESSAGE")
		   EvaluateCHATMESSAGE(subdata);
		else if ( Key == "APPLICATION")
			EvaluateAPPLICATION(subdata);
	}
}

    // each method processes one kind of message
void SkypeClientWnd::EvaluateERROR( QString  subdata )
{

}
void SkypeClientWnd::EvaluateUSERSTATUS( QString subdata )
{

}
void SkypeClientWnd::EvaluateUSER(QString subdata )
{

}
void SkypeClientWnd::EvaluateNAME(QString subdata )
{

}
void SkypeClientWnd::EvaluatePROTOCOL(QString subdata )
{

}
void SkypeClientWnd::EvaluatePONG(QString subdata )
{

}
void SkypeClientWnd::EvaluateCALL(QString subdata )
{

}
void SkypeClientWnd::EvaluateMESSAGE(QString subdata )
{

}
void SkypeClientWnd::EvaluateCONNSTATUS(QString subdata )
{

}
void SkypeClientWnd::EvaluateCURRENTUSERHANDLE(QString subdata )
{

}
void SkypeClientWnd::EvaluateSEARCHRESULTS( TSkypeSearchType atype ,QString  subdata )
{

}
void SkypeClientWnd::EvaluateCALLHISTORYCHANGED(QString subdata )
{

}
void SkypeClientWnd::EvaluateIMHISTORYCHANGED(QString subdata )
{

}
void SkypeClientWnd::EvaluateAUDIO_IN(QString subdata )
{

}
void SkypeClientWnd::EvaluateAUDIO_OUT(QString subdata )
{

}
void SkypeClientWnd::EvaluateMUTE(QString subdata )
{

}
void SkypeClientWnd::EvaluatePRIVILEGE(QString subdata )
{

}
void SkypeClientWnd::EvaluatePROFILE(QString subdata )
{

}
void SkypeClientWnd::EvaluateSKYPEVERSION(QString subdata )
{

}
void SkypeClientWnd::EvaluateCHAT(QString subdata )
{

}
void SkypeClientWnd::EvaluateCHATMESSAGE(QString subdata )
{

}

void SkypeClientWnd::EvaluateAPPLICATION(QString subdata)
{

}


void SkypeClientWnd::Search( TSkypeSearchType aType , QString Target   )
{

	//if aType in SEARCHABLE_TYPE[FUsedProtocol]
    //then 
		SendData("SEARCH " + QString(SEARCHTYPE_STRING[aType]) + " " + Target) ;
    //else NotSupportedError('Searchtype "' +
    //                       SEARCHTYPE_STR[aType] +
     //                      '" not supported (Protocol '+
      //                     IntToStr(FUsedProtocol)+')', true );	
}

// these methods request one property at a time...
void SkypeClientWnd::GetProperty(QString Username     , TSkypeUserProperty UserProperty  )
{
	SendData("GET USER " + Username + " " + QString(USERPROPERTY_STRING[UserProperty]) ) ;
}

void SkypeClientWnd::GetProperty( int CallID , TSkypeCallProperty CallProperty   )
{
	SendData("GET CALL " + QString(CallID) + " " + QString(CALLPROPERTY_STRING[CallProperty]) );
}   

void SkypeClientWnd::GetProperty(int MessageID    , TSkypeMessageProperty  MessageProperty   )
{
	SendData("GET MESSAGE " + QString(MessageID) +  " "  + QString(MESSAGEPROPERTY_STRING[MessageProperty]) ) ;
} 

void SkypeClientWnd::GetProperty(QString Chatname  , TSkypeChatProperty   ChatProperty   )
{
	SendData("GET CHAT " + Chatname + " " + QString(CHATPROPERTY_STRING[ChatProperty])) ;
}

void SkypeClientWnd::GetProperty(int ChatMsgID ,TSkypeChatMsgProperty ChatMsgProperty  )
{
	SendData("GET CHATMESSAGE " + QString(ChatMsgID) + " " + QString(CHATMSGPROPERTY_STRING[ChatMsgProperty]));
} 

void SkypeClientWnd::GetUserStatus()
{
	SendData("GET USERSTATUS");
}
void SkypeClientWnd::GetConnStatus()
{
	SendData("GET CONNSTATUS");
}
void SkypeClientWnd::GetAudioInInfo()
{
	SendData("GET AUDIO_IN");
}
void SkypeClientWnd::GetAudioOutInfo()
{
	SendData("GET AUDIO_OUT");
}
void SkypeClientWnd::GetCurrentUserHandle()
{
	SendData("GET CURRENTUSERHANDLE");
}
void SkypeClientWnd::GetMuteInfo() 
{
	SendData("GET MUTE");
}
void SkypeClientWnd::GetPrivilegeInfo(TSkypePrivilege priv   )
{
	SendData("GET PRIVILEGE " + QString(PRIVILEGE_STRING[priv]));
}
void SkypeClientWnd::GetProfileInfo(TSkypeProfile prop   )
{
	SendData("GET PROFILE " + QString(PROFILE_STRING[prop]) );
}
void SkypeClientWnd::GetSkypeVersion() 
{
	SendData("GET SKYPEVERSION");
}

void SkypeClientWnd::SetProperty(int CallID ,  TSkypeCallProperty   CallProperty  , QString  Value   )
{
	SendData("SET CALL " + QString(CallID) + " " +
                  QString(CALLPROPERTY_STRING[CallProperty] ) + " " + Value );
}

void SkypeClientWnd::SetProperty(int MessageID ,TSkypeMessageProperty MessageProperty )
{
	SendData("SET MESSAGE " + QString(MessageID) + " " +
                  QString(MESSAGEPROPERTY_STRING[MessageProperty]));
} 

void SkypeClientWnd::SetProperty( int ChatMsgID ,TSkypeChatMsgProperty ChatMsgProperty ) 
{
	SendData("SET CHATMESSAGE " + QString(ChatMsgID) + " " +
                  QString(CHATMSGPROPERTY_STRING[ChatMsgProperty]));
}

void SkypeClientWnd::SetUserStatus(TSkypeUserStatus Status   )
{
	SendData("SET USERSTATUS " + QString(USERSTATUS_STRING[Status]));
}
void SkypeClientWnd::SetMute(bool Muted )
{
	//if Muted then
	SendData("SET MUTE ON") ;
    //else SendData("SET MUTE OFF");
}

void SkypeClientWnd::Call(QString Target) //Target can be a username or phone number
                                     // or speed dial number...
{
	SendData("CALL " + Target);
}
void SkypeClientWnd::SendMsg(QString Username , QString aText )
{
	SendData("MESSAGE " + Username + " " + aText);
}

    // the following methods can open some dialogs of Skype
void SkypeClientWnd::AddContactDialog() // Opens "Add a contact"
{
	SendData("OPEN ADDAFRIEND");
}
void SkypeClientWnd::IMDialog(QString username , QString aText ) // Opens
                                                           // "Instant Message"
{
	SendData("OPEN IM " + username + " " + aText);
}	
void SkypeClientWnd::OpenChat(QString users ) // Opens a chat window with users
                                     // example for users "heinz, harry, hennes"
{
	SendData("OPEN CHAT " + users);
}
void SkypeClientWnd::FocusSkype() 
{
	SendData("FOCUS");
}
    // a method to test the connection to Skype. If successful a OnPong is
    // triggered
void SkypeClientWnd::PingSkype() 
{
	SendData("PING");
}


//------------------------------------------------------------------------------
// blocking Methods..
/*
procedure TSkypeControl.ResetRequest;
begin
  RequestPending := false;
  RequestID      := "";
  RequestResult  := "";
  RequestError   := false;
end;
*/
    // the following methods gather information from Skype automatically. They
    // return not until the response message(s) from Skype is(are) received.
QStringList SkypeClientWnd::blSearch(TSkypeSearchType aType , QString Target   )
{
	QStringList sl ;


	return sl ;
}

QString SkypeClientWnd::blGetProperty( QString Username ,TSkypeUserProperty UserProperty   ) 
{
	QString rstr ;

	return rstr ;
}                                                                    

QString SkypeClientWnd::blGetProperty(int CallID ,TSkypeCallProperty CallProperty  ) 
{
	QString rstr ;

	return rstr ;
}                                                                   

QString SkypeClientWnd::blGetProperty(int MessageID ,TSkypeMessageProperty MessageProperty  )
{
	QString rstr ;

	return rstr ;
}                                                                     

QString SkypeClientWnd::blGetProperty(QString Chatname ,TSkypeChatProperty ChatProperty  ) 
{
	QString rstr ;

	return rstr ;
}                                                                  

QString SkypeClientWnd::blGetProperty(int ChatMsgID ,TSkypeChatMsgProperty ChatMsgProperty )
{
	QString rstr ;

	return rstr ;
}

TSkypeUserStatus SkypeClientWnd::blGetUserStatus()
{
	TSkypeUserStatus us ;

	return us ;
}
TSkypeConnStatus SkypeClientWnd::blGetConnStatus() 
{
	TSkypeConnStatus cs ;

	return cs ;
}
QString SkypeClientWnd::blGetAudioInInfo ()
{
	QString rstr ;

	return rstr ;
}
QString SkypeClientWnd::blGetAudioOutInfo ()
{
	QString rstr ;

	return rstr ;
}
QString SkypeClientWnd::blGetCurrentUserHandle ()
{
	QString rstr ;

	return rstr ;
}
bool SkypeClientWnd::blGetMuteInfo()
{
	bool bret ;

	return bret ;
}

bool SkypeClientWnd::blGetPrivilegeInfo(TSkypePrivilege priv   )  
{
	bool bret ;

	return bret ;
}

QString SkypeClientWnd::blGetProfileInfo(TSkypeProfile prop ) 
{
	QString rstr ;

	return rstr ;
}
QString SkypeClientWnd::blGetSkypeVersion () 
{
	QString rstr ;

	return rstr ;
}

    // this function does not block during the hole call, it just returns the
    // callid of the starting call...
int SkypeClientWnd::blCall(QString Target )
{
	int iret ;

	return iret ;
}



