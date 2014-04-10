
#include "tasktreeview.h"
#include "ui_tasktreeview.h"

TaskTreeView::TaskTreeView(QWidget *parent)
{
    mwin = new Ui::TaskTreeView;
    mwin->setupUi(this);
}

TaskTreeView::~TaskTreeView()
{

}
