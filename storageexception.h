#ifndef STORAGEEXCEPTION_H
#define STORAGEEXCEPTION_H

#include <exception>
#include <QObject>

class StorageException : public QObject , std::exception 
{
	Q_OBJECT

public:
    StorageException(QObject *parent);
    ~StorageException() throw() ;
	virtual const char* what() const throw();

private:
    
};

#endif // STORAGEEXCEPTION_H
