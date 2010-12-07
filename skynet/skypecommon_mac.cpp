/***************************************************************
 * skypeX11.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2008-05-14.
 * @Last Change: 2008-05-14.
 * @Revision:    $Id: skypecommon_mac.cpp 165 2010-08-21 06:26:42Z drswinghead $
 * Description:
 * Usage:
 * TODO:
 * CHANGES:
 ***************************************************************/
#include <QtGui/QApplication>
#include "skypecommon.h"

#ifdef Q_WS_MAC

static const char *skypemsg = "SKYPECONTROLAPI_MESSAGE";

SkypeCommon::SkypeCommon() { 
    // msg = new XMessages( skypemsg, (QWidget *) QApplication::desktop() );
    // connect( msg, SIGNAL( gotMessage(int, const QString &) ), this, SLOT( processX11Message(int, const QString &) ) );
    // skype_win = 0;
}

SkypeCommon::~SkypeCommon() 
{

}

void SkypeCommon::sendMsgToSkype(const QString &message) {
    // if ( skype_win > 0 ) msg->sendMessage(skype_win,skypemsg, message);
}

bool SkypeCommon::attachToSkype() {
    // Atom skype_inst = XInternAtom(QX11Info::display(), "_SKYPE_INSTANCE", True);
    // Atom type_ret;
    // int format_ret;
    // unsigned long nitems_ret;
    // unsigned long bytes_after_ret;
    // unsigned char *prop;
    // int status;
    // QString dbgMsg;

    // status = XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(), skype_inst, 0, 1, False, XA_WINDOW, &type_ret, &format_ret, &nitems_ret, &bytes_after_ret, &prop);

    // // sanity check
    // if(status != Success || format_ret != 32 || nitems_ret != 1) {
    //     skype_win = (WId) -1;
    //     qDebug("skype::connectToInstance(): Skype not detected, status %d\n", status );
    //     return false;
    // } else  {
    //     // skype_win = * (const unsigned long *) prop & 0xffffffff;
    //     skype_win = * (const unsigned long *) prop & 0xffffffffffffffff; // test for x64
    //     qDebug("skype::connectToInstance(): Skype instance found, window id %d\n", skype_win);
    //     return true;
    // }
}


// void SkypeCommon::processX11Message(int win, const QString &message) {
    // if ( win == skype_win ) emit newMsgFromSkype( message );
// }


// #include "SkypeCommon.moc"
#endif /* Q_WS_MAC */
