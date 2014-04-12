#ifndef _TASKTREEVIEW_H_
#define _TASKTREEVIEW_H_

#include <QtCore>
#include <QtWidgets>
#include <QtGui>

class SqliteStorage;
class SqliteTaskModel;

namespace Ui {
    class TaskTreeView;
};

class TaskTreeView : public QMainWindow
{
    Q_OBJECT;
public:
    TaskTreeView(QWidget *parent = 0);
    virtual ~TaskTreeView();

public slots:
    void onStorageOpened();

    void testInsertChild();
    void testRemoveChild();
    void testMoveTask();

private:
    Ui::TaskTreeView *mwin = NULL;
    SqliteStorage *mStorage = NULL;
    SqliteTaskModel *mModel = NULL;
};

#endif /* _TASKTREEVIEW_H_ */
