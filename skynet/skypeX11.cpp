/***************************************************************
 * skypeX11.cpp
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2008-05-14.
 * @Last Change: 2008-05-14.
 * @Revision:    $Id$
 * Description:
 * Usage:
 * TODO:
 * CHANGES:
 ***************************************************************/
#include <QtGui/QApplication>
#include "skypeComm.h"

#ifdef Q_WS_X11
#include <QtGui/QX11Info>

#include <X11/Xatom.h>



static const char *skypemsg = "SKYPECONTROLAPI_MESSAGE";

skypeComm::skypeComm() { 
    msg = new XMessages( skypemsg, (QWidget *) QApplication::desktop() );
    connect( msg, SIGNAL( gotMessage(int, const QString &) ), this, SLOT( processX11Message(int, const QString &) ) );
    skype_win = 0;
}

void skypeComm::sendMsgToSkype(const QString &message) {
    if ( skype_win > 0 ) msg->sendMessage(skype_win,skypemsg, message);

}

bool skypeComm::attachToSkype() {
    Atom skype_inst = XInternAtom(QX11Info::display(), "_SKYPE_INSTANCE", True);
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret;
    unsigned long bytes_after_ret;
    unsigned char *prop;
    int status;
    QString dbgMsg;

    status = XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(), skype_inst, 0, 1, False, XA_WINDOW, &type_ret, &format_ret, &nitems_ret, &bytes_after_ret, &prop);

    // sanity check
    if(status != Success || format_ret != 32 || nitems_ret != 1)
        {
            skype_win = (WId) -1;
            qDebug("skype::connectToInstance(): Skype not detected, status %d\n", status );
            return false;
        } else  {
        skype_win = * (const unsigned long *) prop & 0xffffffff;
        qDebug("skype::connectToInstance(): Skype instance found, window id %d\n", skype_win);
        return true;
    }
}


void skypeComm::processX11Message(int win, const QString &message) {
    if ( win == skype_win ) emit newMsgFromSkype( message );
}


// #include "skypeComm.moc"
#endif /* Q_WS_X11 */
