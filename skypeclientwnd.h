#ifndef SKYPECLIENTWND_H
#define SKYPECLIENTWND_H

#include <QtCore>
#include <QWidget>
#include <QtGui>
#include <QThread>

#include "ui_skypeclientwnd.h"

////////////////////////////////*
#ifdef WIN32
enum {
	SKYPECONTROLAPI_ATTACH_SUCCESS=0,								// Client is successfully attached and API window handle can be found in wParam parameter
	SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION=1,	// Skype has acknowledged connection request and is waiting for confirmation from the user.
																									// The client is not yet attached and should wait for SKYPECONTROLAPI_ATTACH_SUCCESS message
	SKYPECONTROLAPI_ATTACH_REFUSED=2,								// User has explicitly denied access to client
	SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE=3,					// API is not available at the moment. For example, this happens when no user is currently logged in.
																									// Client should wait for SKYPECONTROLAPI_ATTACH_API_AVAILABLE broadcast before making any further
																									// connection attempts.
	SKYPECONTROLAPI_ATTACH_API_AVAILABLE=0x8001
};
#endif
////////////////////////*/


/////////Skype control const begin
#define SKYPE_SUPPORTED_PROTOCOL	5

//------------------------------------------------------------------------------
// User Status -----------------------------------------------------------------
enum TSkypeUserStatus{ usUnknown, usOnline, usOffline, usAway, usNA, usDND,usInvisible, usLoggedOut, usSkypeMe } ;

//------------------------------------------------------------------------------
// Connection Status -----------------------------------------------------------
enum TSkypeConnStatus  { csOffline, csConnecting, csPausing, csOnline, csLoggedOut };


//------------------------------------------------------------------------------
// Searchtypes -----------------------------------------------------------------
enum TSkypeSearchType { stUsers, stFriends, stCalls, stActiveCalls,
                         stMissedCalls, stMessages, stMissedMessages, stChats,
                         stActiveChats, stMissedChats, stRecentChats,
						 stBookmarkedChats, stChatMsgs, stMissedChatMsgs } ;


//------------------------------------------------------------------------------
// User Properties -------------------------------------------------------------

enum TSkypeUserProperty { upHandle, upFullname, upBirthday, upSex, upLanguage,
                           upCountry, upProvince, upCity, upPhoneHome,
                           upPhoneOffice, upPhoneMobile, upHomepage, upAbout,
                           upHasCallEquipment, upBuddyStatus, upIsAuthorized,
                           upIsBlocked, upDisplayname, upOnlineStatus,
						   upLastOnlineTimeStamp } ;


//------------------------------------------------------------------------------
// Call Properties -------------------------------------------------------------

enum TSkypeCallProperty { cpTimestamp, cpPartnerHandle, cpPartnerDispname,
                           cpConfId, cpJoinConference, cpConfParticipant,
                           cpConfParticipantsCount, cpType, cpStatus,
                           cpFailureReason, cpSubject, cpPSTNNumber, cpDuration,
						   cpSeen, cpDTMF, cpPSTNStatus } ;

//------------------------------------------------------------------------------
// Message Properties ----------------------------------------------------------

enum TSkypeMessageProperty  { mpTimestamp, mpPartnerHandle, mpPartnerDispname,
                              mpConfId, mpType, mpStatus, mpSeen,
							  mpFailureReason, mpBody } ;


//------------------------------------------------------------------------------
// Chat Properties -------------------------------------------------------------

enum TSkypeChatProperty  { chName, chTimestamp, chAdder, chStatus, chPosters,
                           chMembers, chTopic, chChatMessages, chActiveMembers,
						   chFriendlyname } ;

//------------------------------------------------------------------------------
// Chat Message Properties -----------------------------------------------------

enum TSkypeChatMsgProperty { cmChatname, cmTimestamp, cmFromHandle,
                              cmFromDispname, cmType, cmUsers, cmLeaveReason,
							  cmBody, cmStatus, cmSeen } ;



//------------------------------------------------------------------------------
// Skype Privileges ------------------------------------------------------------
enum TSkypePrivilege {spSkypeOut};
//------------------------------------------------------------------------------
// Skype Profiles --------------------------------------------------------------
enum TSkypeProfile { spfPSTNBalance, spfPSTNBalanceCurrency};

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

//////////skype application
enum TSkypeApplication { appConnectable , appConnecting, appStreams , appSending , appReceived } ;


/////////Skype control const end 

//////////////////////block operation thread
class SkypeCmdRunThread : public QThread 
{
	Q_OBJECT
public:
	SkypeCmdRunThread(QObject * parent){}
	~SkypeCmdRunThread(){}
	void run () {} 

};

///////////////////signal event widget
class SkypeClientWnd :  public  QWidget
{
    Q_OBJECT

public:
    SkypeClientWnd(QWidget *parent = 0);
    ~SkypeClientWnd();

private:
    Ui::SkypeClientWndClass ui;
	QTimer mReconnectTimer ;	//防止与skype断线重拨计时器

///////////平台相关skype处理函数
#ifdef WIN32
public slots:
	bool registerSkypeWindowMessage();
	bool reconnectIfDisconnected() ;

	bool createSkypeApp2AppApplication();
	
	bool skypeAppCreate(QString appname);
	//可能对每一个可连接的用户创建一个app对象。所有appname = 对方skypeid , 也可能不行，所有还是要有两个参数的。
	//一个app应该是可能连接许多用户的。
	bool skypeAppConnect(QString appname,QString skypeid );	
	bool skypeAppWrite(QString appname , QString skypeid,QString connid , QString text);
	//这应该有个参数来存储读取到的数据，或者把读取到的数据作为返回也是可以的。
	bool skypeAppRead(QString appname ,QString skypeid , QString connid ) ;
	bool skypeAppSendDatagram(QString appname , QString skypeid , QString connid  , QString text );
	bool skypeAppDisconnect(QString appname ,QString skypeid , QString connid);
	bool skypeAppDelete(QString appname);

	bool sendSkypeMessage(QString msg);
	//bool pingSkypeApi();

protected:
	virtual bool winEvent ( MSG * message, long * result ); 

signals:
	void SkypeWindowMessageRegistered();
	void SkypeWindowMessageRegisterError();
	void SkypeApiConnected();
	void SkypeApiConnectError();
	void SkypeApp2AppCreated();
	void SkypeApp2AppCreateError();
	void SkypeMessageSended();
	void SkypeMessageSendError() ;
	void SkypePingPong();
	void SkypePingError();

#endif
///////////平台相关skype处理函数

private:	///
	void dispatchSkypeMessage(QString msgdata);

    bool mRequestPending; // set to true, if a bl* method is waiting
                              // for a event
    QString mRequestID     ;  // helps the event-handler (evaluate*) to
                              // determine if the actual event is the requested
                              // one
    QVariant mRequestResult  ; // place for the event-handlers to put the
                              // received information
    bool mRequestError  ; // if an error occurs while a request was pending
                              // the request will be aborted. RequestError
                              // provides this information to the waiting bl*
                              // method
    //--------------------------------------------------------------------------
    //-- private representations of properties ---------------------------------

    bool FEventOnRequest  ;
    bool FAttached       ;
    QString FIdentName   ;
    bool FAutoReAttach   ;
    int FUsedProtocol   ;

    //--------------------------------------------------------------------------
    //-- private methods -------------------------------------------------------
   // string parsing function
    QString getFirstDelimited(QString str , QChar dlm ) ;
    // Message hook which is inserted at the beginning of the application's
    // message loop
    //function MessageHook(var Msg: TMessage): Boolean;

    // Sending NAME and PROTOCOL to Skype
    void introduce() ;

    // Helper-function to send WM_COPYDATA Messages
    // returns true if successfull (Data was sent or aString was empty)


    // Helper-function to receive WM_COPYDATA Messages
    // returns the received data as a List of strings
#ifdef WIN32
    QString ReceiveData( MSG * msg , QString subdata ) ;
#endif
    // Dispatch evaluates the first Parameter of a received message and calls
    // the according method to further process the message
    //procedure DispatchMessage(Key, subdata : string);

    // each method processes one kind of message
    void EvaluateERROR( QString  subdata );
    void EvaluateUSERSTATUS( QString subdata );
    void EvaluateUSER(QString subdata );
    void EvaluateNAME(QString subdata );
    void EvaluatePROTOCOL(QString subdata );
    void EvaluatePONG(QString subdata );
    void EvaluateCALL(QString subdata );
    void EvaluateMESSAGE(QString subdata );
    void EvaluateCONNSTATUS(QString subdata );
    void EvaluateCURRENTUSERHANDLE(QString subdata );
    void EvaluateSEARCHRESULTS( TSkypeSearchType atype ,QString  subdata );
    void EvaluateCALLHISTORYCHANGED(QString subdata );
    void EvaluateIMHISTORYCHANGED(QString subdata );
    void EvaluateAUDIO_IN(QString subdata );
    void EvaluateAUDIO_OUT(QString subdata );
    void EvaluateMUTE(QString subdata );
    void EvaluatePRIVILEGE(QString subdata );
    void EvaluatePROFILE(QString subdata );
    void EvaluateSKYPEVERSION(QString subdata );
    void EvaluateCHAT(QString subdata );
    void EvaluateCHATMESSAGE(QString subdata );

	//添加处理app2app的方法
	void EvaluateAPPLICATION(QString subdata);

    // method to reset the requestvariables
    void ResetRequest () ;

    // method to trigger the OnNotSupportedError event
    // killrequest parameter is used to stop possible pending request
    // e.g. in the GetProperty methods
    void NotSupportedError(QString errormsg   ,bool  killrequest = false);

 public slots:

    //--------------------------------------------------------------------------
    //-- constructor / destructor ----------------------------------------------
    //constructor Create(AOwner: TComponent); override;
    //destructor Destroy; override;

    //--------------------------------------------------------------------------
    //-- public methods --------------------------------------------------------

    // Try to connect to Skype
    void AttachToSkype(int Protocol  = SKYPE_SUPPORTED_PROTOCOL ) ;

    // Disconnect from Skype
    void DetachFromSkype();

    // the following methods just send messages to Skype. The results
    // have to be gathered manually with the events generated by
    // the Skype-messages.

    // Search requests a specific type of information about the target.
    // If no target is specified, then all results are returned.
    // aType specifies the information type:
    // stUsers: TARGET - username or a part of it. If the search string contains
    //          "@", then search is performed by e-mail address (note that
    //          e-mail address has to match 100%).
    // stFriends: TARGET not allowed
    // stCalls, stActiveCalls, stMissedCalls, stMessages, stMissedMessages :
    //   TARGET optional
    void Search( TSkypeSearchType aType , QString Target  = "" ) ;

    // these methods request one property at a time...
    void GetProperty(QString Username     , TSkypeUserProperty UserProperty  );   

    void GetProperty( int CallID , TSkypeCallProperty CallProperty   );   

    void GetProperty(int MessageID    , TSkypeMessageProperty  MessageProperty   );  

    void GetProperty(QString Chatname  , TSkypeChatProperty   ChatProperty   );  

    void GetProperty(int ChatMsgID ,TSkypeChatMsgProperty ChatMsgProperty  );  

    void GetUserStatus();
    void GetConnStatus();
    void GetAudioInInfo();
    void GetAudioOutInfo();
    void GetCurrentUserHandle();
    void GetMuteInfo() ;
    void GetPrivilegeInfo(TSkypePrivilege priv   );
    void GetProfileInfo(TSkypeProfile prop   );
    void GetSkypeVersion() ;

    void SetProperty(int CallID ,  TSkypeCallProperty   CallProperty  , QString  Value = "" ); 

    void SetProperty(int MessageID ,TSkypeMessageProperty MessageProperty ); 

    void SetProperty( int ChatMsgID ,TSkypeChatMsgProperty ChatMsgProperty ) ;

    void SetUserStatus(TSkypeUserStatus Status   );
    void SetMute(bool Muted );

    void Call(QString Target); //Target can be a username or phone number
                                     // or speed dial number...

    void SendMsg(QString Username , QString aText );

    // the following methods can open some dialogs of Skype
    void AddContactDialog(); // Opens "Add a contact"
    void IMDialog(QString username , QString aText ); // Opens
                                                           // "Instant Message"
    void OpenChat(QString users ); // Opens a chat window with users
                                     // example for users "heinz, harry, hennes"
    void FocusSkype() ;

    // a method to test the connection to Skype. If successful a OnPong is
    // triggered
    void PingSkype() ;


    // the following methods gather information from Skype automatically. They
    // return not until the response message(s) from Skype is(are) received.
    QStringList blSearch(TSkypeSearchType aType , QString Target = "" ) ;

    QString blGetProperty( QString Username ,TSkypeUserProperty UserProperty   )   ;                                                                      

    QString blGetProperty(int CallID ,TSkypeCallProperty CallProperty  )  ;                                                                      

    QString blGetProperty(int MessageID ,TSkypeMessageProperty MessageProperty  ) ;                                                                      

    QString blGetProperty(QString Chatname ,TSkypeChatProperty ChatProperty  )  ;                                                                      

    QString blGetProperty(int ChatMsgID ,TSkypeChatMsgProperty ChatMsgProperty ) ;

    TSkypeUserStatus blGetUserStatus();
    TSkypeConnStatus blGetConnStatus() ;
    QString blGetAudioInInfo ();
    QString blGetAudioOutInfo ();
    QString blGetCurrentUserHandle ();
    bool blGetMuteInfo();
    bool blGetPrivilegeInfo(TSkypePrivilege priv   )  ;
    QString blGetProfileInfo(TSkypeProfile prop ) ;
    QString blGetSkypeVersion () ;

    // this function does not block during the hole call, it just returns the
    // callid of the starting call...
    int blCall(QString Target ) ;

      // This Name is used to identifiy the Client towards Skype
    bool SendData(QString aString ) ;   

/*

    //--------------------------------------------------------------------------
    //-- published Properties --------------------------------------------------

    // This Name is used to identifiy the Client towards Skype
    function SendData(aString : string) : boolean;
    property IdentName : string read FIdentName write FIdentName;

    // If set to true, then an attach-attempt is made on the
    // OnAPIAvailable event...
    property AutoReAttach : boolean read FAutoReAttach write FAutoReAttach;

    //--------------------------------------------------------------------------
    //-- published Events ------------------------------------------------------

    // Client is successfully attached
    property OnAttachSuccess : TNotifyEvent read  FOnAttachSuccess
                                            write FOnAttachSuccess;

    // Skype has acknowledged connection request and is waiting for confirmation
    // from the user.
    // The client is not yet attached and should wait for OnAttachSuccess.
    property OnAttachPending : TNotifyEvent read  FOnAttachPending
                                            write FOnAttachPending;

    // User has explicitly denied access to client
    property OnAttachRefused : TNotifyEvent read  FOnAttachRefused
                                            write FOnAttachRefused;

    // API is not available at the moment. For example, this happens when no
    // user is currently logged in. Client should wait for OnAPIAvailable before
    // making any further connection attempts.
    property OnAPINotAvailable : TNotifyEvent read  FOnAPINotAvailable
                                              write FOnAPINotAvailable;
    property OnAPIAvailable    : TNotifyEvent read  FOnAPIAvailable
                                              write FOnAPIAvailable;

    // The error response is sent by Skype each time Skype senses an error
    // condition, which includes syntactically incorrect commands, internal
    // inconsistencies etc.
    // Parameter "Code" is a number that uniquely identifies error condition and
    // Parameter "Description" is a optional brief description of the situation,
    // given in English.
    property OnError : TSkypeErrorNotify read FOnError write FOnError;

    // if Skype sends a USERSTATUS Messages (e.g. USERSTATUS AWAY) the event is
    // triggered.
    property OnUserStatus : TSkypeUserStatusNotify read  FOnUserStatus
                                                   write FOnUserStatus;

    // Notifications are sent by Skype either if the corresponding object
    // changes, or if the value of the property is asked with GET command. Also,
    // if the property is changed by SET command, the change is confirmed with a
    // notification. The OnUserInfo event is triggered, when a notification
    // about user object properties is received.
    property OnUserInfo : TSkypeUserNotify read FOnUserInfo write FOnUserInfo;

    // the event is triggered if NAME message is received, usually to confirm
    // the NAME command.
    property OnName : TSkypeNameNotify read FOnName write FOnName;

    // Whenever a new version of the API is released the protocol version number
    // is increased. When a client starts using the API then the client must
    // tell the Skype API the latest protocol version that it supports. Skype
    // will reply with its latest version number and the number reported by
    // Skype will be the protocol version used. Skype will never reply with a
    // protocol version which is newer than the version the client application
    // supports. Skype defaults to protocol version 1.
    // Skype-supported version can be queried with PROTOCOL 99999.
    // Example: Client speaks version 3 and tells Skype "PROTOCOL 3", Skype
    // knows version 2 and replies with "PROTOCOL 2". Version 2 will be the used
    // protocol in this case.
    property OnProtocol : TSkypeProtocolNotify read  FOnProtocol
                                               write FOnProtocol;

    // event is triggered if PONG message is received. Skype sends the PONG
    // message to confirm the PING command
    property OnPong : TNotifyEvent read FOnPong write FOnPong;

    // The OnCallInfo event is triggered, when a notification about call object
    // properties is received.
    property OnCallInfo : TSkypeCallNotify read FOnCallInfo write FOnCallInfo;

    // The OnMessageInfo event is triggered, when a notification about message
    // object properties is received.
    property OnMessageInfo : TSkypeMessageNotify read  FOnMessageInfo
                                                 write FOnMessageInfo;

    // if Skype sends a CONNSTATUS Message (e.g. CONNSTATUS ONLINE) the event is
    // triggered.
    property OnConnStatus : TSkypeConnStatusNotify read  FOnConnStatus
                                                   write FOnConnStatus;

    // event is triggered if CURRENTUSERHANDLE Message is received
    property OnCurrentUserHandle : TSkypeCurrentUserHandleNotify
                                                     read  FOnCurrentUserHandle
                                                     write FOnCurrentUserHandle;

    // event is triggered if result of the SEARCH command is received
    property OnSearchResult : TSkypeSearchNotify read  FOnSearchResult
                                                 write FOnSearchResult;

    // Notifies about call history being changed and that it needs to be
    // reloaded. Right now it only happens, when all of the call history has
    // been deleted.
    property OnCallHistoryChanged : TNotifyEvent read  FOnCallHistoryChanged
                                                 write FOnCallHistoryChanged;

    // Notifies about instant message history being changed and that it needs to
    // be reloaded. Right now it only happens, when IM history for the specific
    // user deleted.
    property OnIMHistoryChanged : TNotifyEvent read  FOnIMHistoryChanged
                                               write FOnIMHistoryChanged;

    // event is triggered if confirmation for SET AUDIO_IN command or
    // result of GET AUDIO_IN command is received.
    // Empty device name means "default device".
    property OnAudioInInfo : TSkypeAudioNotify read  FOnAudioInInfo
                                               write FOnAudioInInfo;

    // event is triggered if confirmation for SET AUDIO_OUT command or
    // result of GET AUDIO_OUT command is received.
    // Empty device name means "default device".
    property OnAudioOutInfo : TSkypeAudioNotify read  FOnAudioOutInfo
                                                write FOnAudioOutInfo;

    // event is triggered if confirmation for SET MUTE command or
    // result of GET MUTE command is received.
    property OnMuteInfo : TSkypeMuteNotify read FOnMuteInfo write FOnMuteInfo;

    // event is triggered if result of GET PRIVILEGE command is received.
    property OnPrivilegeInfo : TSkypePrivilegeNotify read  FOnPrivilegeInfo
                                                     write FOnPrivilegeInfo;

    // event is triggered if result of GET PROFILE command is received.
    property OnProfileInfo : TSkypeProfileNotify read  FOnProfileInfo
                                                 write FOnProfileInfo;

    // event is triggered if result of GET SKYPEVERSION command is received.
    property OnVersionInfo : TSkypeVersionNotify read  FOnVersionInfo
                                                 write FOnVersionInfo;

    // The OnChatInfo event is triggered, when a notification about chat
    // object properties is received.
    property OnChatInfo : TSkypeChatNotify read  FOnChatInfo
                                           write FOnChatInfo;

    // The OnChatMsgInfo event is triggered, when a notification about chatmsg
    // object properties is received.
    property OnChatMsgInfo : TSkypeChatMsgNotify read  FOnChatMsgInfo
                                                 write FOnChatMsgInfo;

    // the OnNotSupportedError event is triggered when a received message or
    // a executed method tries to do something not supported, e.g. setting
    // userstatus to usSkypeMe with protocol 1
    property OnNotSupportedError : TNotSupportedErrorNotify
                                     read  FOnNotSupportedError
                                     write FOnNotSupportedError;

    // event is triggered each time a WM_COPYDATA message from Skype is received
    property OnCopyData : TSkypeCopyData read FOnCopyData write FOnCopyData;

*/
};



#endif // SKYPECLIENTWND_H
