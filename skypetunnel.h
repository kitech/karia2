// skypetunnel.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-09-08 14:11:53 +0800
// Version: $Id$
// 

#ifndef _SKYPETUNNEL_H_
#define _SKYPETUNNEL_H_

#include <QtCore>

class SkypePackage;
class Skype;
class SkypeTunnel : public QObject
{
    Q_OBJECT;
public:
    SkypeTunnel(QObject *parent = 0);
    virtual ~SkypeTunnel();

    void setSkype(Skype *skype);

public slots:
    void onSkypeError(int errNo, QString msg);
    void onSkypeConnected(QString skypeName);
    void onSkypeDisconnected(QString skypeName);
    void onNewStreamCreated(QString contactName, int stream);
    void onSkypePackageArrived(QString contactName, int stream, QString data);
    void processRequest(QString contactName, int stream, SkypePackage *sp);

private:
    Skype *mSkype;
};

#endif /* _SKYPETUNNEL_H_ */
