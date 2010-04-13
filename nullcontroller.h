#ifndef NULLCONTROLLER_H
#define NULLCONTROLLER_H

#include <QObject>

class NullController : public QObject
{
	Q_OBJECT

public:
    NullController(QObject *parent);
    ~NullController();

private:
    
};

#endif // NULLCONTROLLER_H
