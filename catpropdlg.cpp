#include <QtCore>
#include <QtGui>
#include <QListWidget>
#include <QListView>
#include <QAction>
#include <QFileDialog>
#include <QDir>


#include "sqlitestorage.h"
#include "optionmanager.h"

#include "catpropdlg.h"

CatPropDlg::CatPropDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	QObject::connect(this->ui.cpd_pb_open_directory, SIGNAL(clicked()),
		this, SLOT(onOpenDefaultCategoryDirectory()));
}

CatPropDlg::~CatPropDlg()
{

}

void CatPropDlg::setCategoryModel(const QModelIndex & index)
{
	QModelIndex nindex ;

	nindex = index.model()->index(index.row(),index.column()+1,index.parent());

	this->ui.cpd_le_cat_name->setText(index.data().toString());
	this->ui.cpd_le_cat_directory->setText(nindex.data().toString());

}

void CatPropDlg::setCategoryParameters(int subCatCount,int totalFileCount , int finishedFileCount,long totalLength )
{
	this->ui.cpd_lb_sub_category_num->setText(QString("%1").arg(subCatCount));
	this->ui.cpd_lb_total_file_num->setText(QString("%1").arg(totalFileCount));
	this->ui.cpd_lb_downloaded_file_num->setText(QString("%1").arg(finishedFileCount));
	this->ui.cpd_lb_downloaded_size->setText(QString("%1").arg(totalLength));

}

void CatPropDlg::onOpenDefaultCategoryDirectory()
{
	QString oldpath = this->ui.cpd_le_cat_directory->text();
	
	QString path = QFileDialog::getExistingDirectory(this,"Choose a directory",   oldpath  ,
                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if ( path.isEmpty())
	{
	}
	else
	{
		this->ui.cpd_le_cat_directory->setText(QDir::convertSeparators(path));
	}
}

void CatPropDlg::paintEvent ( QPaintEvent * event )
{
	//qDebug()<<__FUNCTION__;
	//QPainter painter(this);
	//QPoint p(0,0);
	//painter.drawImage(p,QImage("4422b46f9b7e5389963077_zVzR8Bc2kwIf.jpg"));
}

/////////////////////////////
///////
/////////////////////////////

ColumnsManDlg::ColumnsManDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	QListWidget * lw = this->ui.colmd_lw_cols ;
    QVector<QString> columns = SqliteStorage::instance()->getTasksColumns();
	
    for (int i = 0 ; i < columns.count(); ++i) {
        this->mTaskViewTitle.insert(i, columns.at(i));
    }

	// this->mTaskViewTitle.insert(0,"ID");
	// this->mTaskViewTitle.insert(1,"Status");
	// this->mTaskViewTitle.insert(2,"Name");
	// this->mTaskViewTitle.insert(3,"Number");
	// this->mTaskViewTitle.insert(4,"URL");
	// this->mTaskViewTitle.insert(5,"Size");
	// this->mTaskViewTitle.insert(6,"Completed");
	// this->mTaskViewTitle.insert(7,"Percent");
	// this->mTaskViewTitle.insert(8,"Elapsed");
	// this->mTaskViewTitle.insert(9,"Left");
	// this->mTaskViewTitle.insert(10,"Speed");
	// this->mTaskViewTitle.insert(11,"Retry");
	// this->mTaskViewTitle.insert(12,"Comment");
	// this->mTaskViewTitle.insert(13,"CreateTime");
	// this->mTaskViewTitle.insert(14,"CompleteTime");
	// this->mTaskViewTitle.insert(15,"Category");
	// this->mTaskViewTitle.insert(16,"placeholder");

    OptionManager *om = OptionManager::instance();
    QString taskShowColumns = om->getTaskShowColumns();
    QStringList colList;
    if (taskShowColumns != "") {
        colList = taskShowColumns.split(',');
    }

	QListWidgetItem *lwi;
	QStringList sl;
	for (int i = 0 ; i < this->mTaskViewTitle.size() ; i ++)
	{
		lwi = new QListWidgetItem (this->mTaskViewTitle[i]);
        if (colList.count() == 0) {
            lwi->setCheckState(Qt::Checked);
        } else {
            if (colList.contains(QString::number(i))) {
                lwi->setCheckState(Qt::Checked);
            } else {
                lwi->setCheckState(Qt::Unchecked);
            }
        }
		lw->addItem(lwi);
	}	

	QObject::connect(lw, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onItemClicked(QListWidgetItem*)));
	QObject::connect(this->ui.colmd_pb_show, SIGNAL(clicked()), this, SLOT(onShowItemClicked()));
	QObject::connect(this->ui.colmd_pb_hide, SIGNAL(clicked()), this, SLOT(onHideItemClicked()));

    QObject::connect(this->ui.pushButton, SIGNAL(clicked()), this, SLOT(onApplyChange()));
    QObject::connect(this->ui.pushButton_2, SIGNAL(clicked()), this, SLOT(onRestoreDefault()));
    QObject::connect(this->ui.pushButton_3, SIGNAL(clicked()), this, SLOT(onSelectAll()));
    QObject::connect(this->ui.pushButton_4, SIGNAL(clicked()), this, SLOT(onRevertSelect()));
}

ColumnsManDlg::~ColumnsManDlg()
{
    
}

void ColumnsManDlg::onItemClicked(QListWidgetItem * item )
{
	qDebug()<<__FUNCTION__;

	QListWidget *lw = this->ui.colmd_lw_cols;

	if ( item->checkState() == Qt::Checked) {
		this->ui.colmd_pb_show->setEnabled(false);
		this->ui.colmd_pb_hide->setEnabled(true);
	} else {
		this->ui.colmd_pb_show->setEnabled(true);
		this->ui.colmd_pb_hide->setEnabled(false);
	}
	if ( item == lw->item(0) ) {
		this->ui.colmd_pb_move_up->setEnabled(false);		
	} else {		
		this->ui.colmd_pb_move_up->setEnabled(true);
	}	
	if ( item == lw->item(this->mTaskViewTitle.size()-1) ) {
		this->ui.colmd_pb_move_down->setEnabled(false);		
	} else {		
		this->ui.colmd_pb_move_down->setEnabled(true);
	}

    this->ui.pushButton->setEnabled(true);
    this->ui.pushButton_2->setEnabled(true);
}

void ColumnsManDlg::onShowItemClicked( )
{
	QListWidget * lw = this->ui.colmd_lw_cols ;
	QList<QListWidgetItem *>  slwi;
	slwi = lw->selectedItems();
	if ( slwi.size() == 1 ) {
		slwi.at(0)->setCheckState(Qt::Checked);
		this->ui.colmd_pb_show->setEnabled(false);
		this->ui.colmd_pb_hide->setEnabled(true);
	}

    this->ui.pushButton->setEnabled(true);
    this->ui.pushButton_2->setEnabled(true);
}

void ColumnsManDlg::onHideItemClicked( )
{
	QListWidget * lw = this->ui.colmd_lw_cols ;
	QList<QListWidgetItem *>  slwi;
	slwi = lw->selectedItems();
	if ( slwi.size() == 1 ) {
		slwi.at(0)->setCheckState(Qt::Unchecked);
		this->ui.colmd_pb_show->setEnabled(true);
		this->ui.colmd_pb_hide->setEnabled(false);
	}
    this->ui.pushButton->setEnabled(true);
    this->ui.pushButton_2->setEnabled(true);
}

void ColumnsManDlg::onApplyChange()
{
    int rowCount = this->ui.colmd_lw_cols->count();
    QListWidgetItem *item = NULL;
    QStringList checkedList;
    QStringList defaultList;
    for (int i = 0 ; i < rowCount; ++i) {
        item = this->ui.colmd_lw_cols->item(i);
        if (item->checkState() == Qt::Checked) {
            checkedList << QString("%1").arg(i);
        }
        defaultList<< QString("%1").arg(i);
    }
    QString checkedItems = checkedList.join(",");
    QString defaultItems = defaultList.join(",");
    // qDebug()<<checkedItems;

    // SqliteStorage *storage = SqliteStorage::instance();
    // QString currSetItems = this->loadKey("taskshowcolumns", defaultItems);
    QString currSetItems = OptionManager::instance()->getTaskShowColumns();

    if (currSetItems != checkedItems) {
        // storage->deleteUserOption("taskshowcolumns");
        // storage->addUserOption("taskshowcolumns", checkedItems, "auto");
        emit taskShowColumnsChanged(checkedItems);
        OptionManager::instance()->saveTaskShowColumns(checkedItems);
    }
    this->ui.pushButton->setEnabled(false);
    this->ui.pushButton_2->setEnabled(false);
}

void ColumnsManDlg::onRestoreDefault()
{
    int rowCount = this->ui.colmd_lw_cols->count();
    QListWidgetItem *item = NULL;
    QStringList checkedList;
    for (int i = 0 ; i < rowCount; ++i) {
        item = this->ui.colmd_lw_cols->item(i);
        item->setCheckState(Qt::Checked);
    }

    this->onApplyChange(); // user trigger this action
}

void ColumnsManDlg::onMoveUp()
{

}

void ColumnsManDlg::onMoveDown()
{

}

void ColumnsManDlg::onSelectAll()
{
    int rowCount = this->ui.colmd_lw_cols->count();
    QListWidgetItem *item = NULL;
    QStringList checkedList;
    for (int i = 0 ; i < rowCount; ++i) {
        item = this->ui.colmd_lw_cols->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void ColumnsManDlg::onRevertSelect()
{
    int rowCount = this->ui.colmd_lw_cols->count();
    QListWidgetItem *item = NULL;
    QStringList checkedList;
    for (int i = 0 ; i < rowCount; ++i) {
        item = this->ui.colmd_lw_cols->item(i);
        item->setCheckState(item->checkState() == Qt::Checked 
                            ? Qt::Unchecked : Qt::Checked);
    }
}

// QString ColumnsManDlg::loadKey(QString key, QString dvalue)
// {
//     SqliteStorage *storage = SqliteStorage::instance();
//     QString ov = QString::null;
//     QString optionValue;

//     optionValue = storage->getUserOption(key);
//     if (optionValue == QString::null) {
//         optionValue = storage->getDefaultOption(key);
//         if (optionValue == QString::null) {
//             storage->addDefaultOption(key, dvalue, "auto");
//             ov = dvalue;
//         } else {
//             ov = optionValue;
//         }
//     } else {
//         ov = optionValue;
//     }

//     return ov;
// }


