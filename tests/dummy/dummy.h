#ifndef _DUMMY_H_
#define _DUMMY_H_

#include <QtCore>
#include <QtWidgets>
#include <QtGui>

namespace Ui {
    class Dummy;
};

class Dummy : public QMainWindow
{
    Q_OBJECT;
public:
    Dummy(QWidget *parent = 0);
    virtual ~Dummy();

private:
    Ui::Dummy *mwin = NULL;
};

#endif /* _DUMMY_H_ */
