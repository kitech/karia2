#ifndef STORAGEFACTORY_H
#define STORAGEFACTORY_H

#include <cassert>
#include <QString>
#include "sqlitestorage.h"
#include "xmlstorage.h"

class StorageFactory : public QObject
{
	Q_OBJECT

public:
    StorageFactory(QObject *parent);
    ~StorageFactory();

	static AbstractStorage * instance(QString which);
private:
    
};

#endif // STORAGEFACTORY_H
