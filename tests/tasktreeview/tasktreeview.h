#ifndef _TASKTREEVIEW_H_
#define _TASKTREEVIEW_H_

#include <QtCore>
#include <QtWidgets>
#include <QtGui>

namespace Ui {
    class TaskTreeView;
};

class TaskTreeView : public QMainWindow
{
    Q_OBJECT;
public:
    TaskTreeView(QWidget *parent = 0);
    virtual ~TaskTreeView();

private:
    Ui::TaskTreeView *mwin = NULL;
};

#endif /* _TASKTREEVIEW_H_ */
