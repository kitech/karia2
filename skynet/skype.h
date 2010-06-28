#ifndef _skype_H
#define _skype_H

/***************************************************************
 * skype.h
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Created:     2008-04-30.
 * @Last Change: 2008-04-30.
 * @Revision:    $Id$
 * Description:
 * Usage:
 * TODO:
 *CHANGES:
 ***************************************************************/
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include "skypeCommand.h"
#include "skypeComm.h"


class skype : public QObject { 
  Q_OBJECT
	private:
	  skypeComm sk;

	  QString appName;
	  bool connected;
	  QHash<QString, QByteArray> streams;
	  QHash<QString, int> activeStream;

	  QStringList contacts;
	  bool contactsUpToDate;


	  bool waitingForResponse;
	  QString waitForResponseID;
	  QEventLoop localEventLoop;
	  QTimer *pingTimer;
	  int TimeOut;
	  skypeResponse response;

	protected:
	  void readIncomingData(QString contact, int stream);
	  bool doCommand(QString cmd, bool blocking = true);
	  int waitForResponse ( QString commandID );

	public:
	  skype(QString AppName);
	  bool connectToSkype();
	  bool disconnectFromSkype();
	  void newStream(QString contact);
	  bool writeToStream(QByteArray data, QString contactName); //deprecated
	  bool writeToSock(QString contactName, QByteArray data) { return writeToStream( data, contactName ); };
	  QByteArray readFromStream(QString contact);

	  QStringList getContacts();

	signals:
	  void skypeError(int errNo, QString Msg);
	  void dataInStream(QString contactName);
	  void newStreamCreated(QString contactName);

	protected slots:
	  void processMessage(const QString &message);
	  void timeOut();
	  void ping();
};



#endif /* _skype_H */
