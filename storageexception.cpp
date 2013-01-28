#include "storageexception.h"

StorageException::StorageException(QObject *parent)
	: std::exception(/*parent*/),QObject(parent)
{

}

StorageException::~StorageException() throw()
{

}

const char* StorageException::what() const throw()
{
	
	return 0;
}