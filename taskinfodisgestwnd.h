#ifndef TASKINFODISGESTWND_H
#define TASKINFODISGESTWND_H

#include <QWidget>
#include "ui_taskinfodisgestwnd.h"

class TaskInfoDisgestWnd : public QWidget
{
    Q_OBJECT

public:
    TaskInfoDisgestWnd(QWidget *parent = 0);
    ~TaskInfoDisgestWnd();

private:
    Ui::TaskInfoDisgestWndClass ui;
};

#endif // TASKINFODISGESTWND_H
