#ifndef CATPROPDLG_H
#define CATPROPDLG_H

#include <QDialog>
#include <QFileDialog>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QListWidgetItem>

#include "ui_catpropdlg.h"

#include "ui_columnsmandlg.h"
#include "ui_columnsmandlg.h"

class CatPropDlg : public QDialog
{
    Q_OBJECT

public:
    CatPropDlg(QWidget *parent = 0);
    ~CatPropDlg();
	void setCategoryModel(const QModelIndex & index);
	void setCategoryParameters(int subCatCount,int fileCount , int finishedFileCount,long totalLength );

public slots:
	void onOpenDefaultCategoryDirectory();

private:
    Ui::CatPropDlgClass ui;
	

protected:
	virtual void paintEvent ( QPaintEvent * event ) ;
};




class ColumnsManDlg : public QDialog
{
    Q_OBJECT

public:
    ColumnsManDlg(QWidget *parent = 0);
    ~ColumnsManDlg();

private:
    Ui::ColumnsManDlgClass ui;
	QMap<int , QString> mTaskViewTitle;		//task list view header titles
	QMap<int , int> mTaskViewTitleWidth;		//task list view header titles
	QMap<int , int> mTaskViewTitleShowState;		//task list view header titles
private slots:
	
	void onItemClicked(QListWidgetItem * item );
	void onShowItemClicked( );
	void onHideItemClicked( );
};

#endif // CATPROPDLG_H
