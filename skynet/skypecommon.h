#ifndef _SKYPECOMM_H
#define _SKYPECOMM_H

/***************************************************************
 * skypeComm.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2008-05-15.
 * @Last Change: 2008-05-15.
 * @Revision:    $Id$
 * Description:
 * Usage:
 * TODO:
 * CHANGES:
 ***************************************************************/

#include <QtCore/QString>
#include <QtGui/QWidget>

#ifdef Q_WS_X11
#include "xmessages.h"
#include <X11/Xlib.h>
#endif

#ifdef Q_WS_WIN
#include <QtCore/QEventLoop>
#include <windows.h>
#endif

class SkypeCommon : public QObject {
    Q_OBJECT;
private:
    WId skype_win;

#ifdef Q_WS_X11
    XMessages *msg;
#endif

#ifdef Q_WS_WIN
    static QWidget *mainWin;
    static WId main_window;
    bool connected, refused, tryLater;
    static UINT attachMSG, discoverMSG;

    QEventLoop localEventLoop;
    long TimeOut;
    bool waitingForConnect;

private slots:
    void timeOut();
#endif

public:
    SkypeCommon();
    virtual ~SkypeCommon();

    void sendMsgToSkype(const QString &message);
    bool attachToSkype();
    bool detachSkype();
    bool is_skype_running();

signals:
    void newMsgFromSkype(const QString &message);

	
protected slots:

#ifdef Q_WS_X11
    void processX11Message(int win, const QString &message);
#endif

#ifdef Q_WS_WIN
    void processWINMessage( MSG *msg );
#endif

};


#endif /* _SKYPECOMM_H */
