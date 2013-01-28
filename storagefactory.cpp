#include "storagefactory.h"

StorageFactory::StorageFactory(QObject *parent)
	: QObject(parent)
{

}

StorageFactory::~StorageFactory()
{

}

AbstractStorage * StorageFactory::instance(QString which)
{
	AbstractStorage * inst = 0 ;
	if( which == "xml")
	{
		assert( 1==2 );
	}
	else if( which == "sqlite")
	{
		inst = new SqliteStorage(0);
		inst->open();
		
	}
	else
	{
		assert(1==2);
	}
	return inst ;
	return 0 ;
}


