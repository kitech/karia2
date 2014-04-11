
#include "sqlitetaskmodel.h"

#include "tasktreeview.h"
#include "ui_tasktreeview.h"

TaskTreeView::TaskTreeView(QWidget *parent)
{
    mwin = new Ui::TaskTreeView;
    mwin->setupUi(this);

    SqliteTaskModel *model = SqliteTaskModel::instance(ng::cats::downloading);
    this->mwin->treeView->setModel(model);
}

TaskTreeView::~TaskTreeView()
{

}
