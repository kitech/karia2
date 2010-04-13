#ifndef XMLSTORAGE_H
#define XMLSTORAGE_H

#include "abstractstorage.h"

class XMLStorage : public AbstractStorage
{
	Q_OBJECT

public:
    XMLStorage(QObject *parent);
    ~XMLStorage();
	
	bool init()  ;

private:
    
};

#endif // XMLSTORAGE_H
