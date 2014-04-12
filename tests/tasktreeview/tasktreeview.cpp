#include "simplelog.h"

#include "sqlitetaskmodel.h"

#include "tasktreeview.h"
#include "ui_tasktreeview.h"

TaskTreeView::TaskTreeView(QWidget *parent)
{
    mwin = new Ui::TaskTreeView;
    mwin->setupUi(this);

    QObject::connect(mwin->pushButton, &QPushButton::clicked, this, &TaskTreeView::testInsertChild);
    QObject::connect(mwin->pushButton_2, &QPushButton::clicked, this, &TaskTreeView::testRemoveChild);
    QObject::connect(mwin->pushButton_3, &QPushButton::clicked, this, &TaskTreeView::testMoveTask);

	// init base storage db 
    this->mStorage = SqliteStorage::instance(this);
    QObject::connect(this->mStorage, &SqliteStorage::opened, this, &TaskTreeView::onStorageOpened);
	this->mStorage->open();
}

TaskTreeView::~TaskTreeView()
{

}

void TaskTreeView::onStorageOpened()
{
    SqliteTaskModel *model = SqliteTaskModel::instance(ng::cats::downloading);
    mModel = model;

    this->mwin->treeView->setModel(model);
}

void TaskTreeView::testInsertChild()
{
    qLogx()<<""<<mModel;
    
    QModelIndex pidx = mModel->index(0, 0);
    mModel->insertRows(0, 0, pidx);

    QModelIndex idx = mModel->index(0, 5, pidx);
    mModel->setData(idx, "56789");
    idx = mModel->index(0, 3, pidx);
    mModel->setData(idx, "34567");
}

void TaskTreeView::testRemoveChild()
{
    qLogx()<<""<<mModel;
    
    QModelIndex idx = mModel->index(0, 0);
    mModel->removeRows(0, 1, idx);
}

void TaskTreeView::testMoveTask()
{
    qLogx()<<""<<mModel;
    QModelIndexList mil = mwin->treeView->selectionModel()->selectedIndexes();
    
    mModel->moveTasks(ng::cats::downloading, ng::cats::downloaded, mil, false);
}




