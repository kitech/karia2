#include "preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	////////
	mIsModified = false ;

	//QObject::connect(this->ui.listWidget,SIGNAL( currentRowChanged ( int  )),
	//	this->ui.stackedWidget,SLOT( setCurrentIndex ( int  ) ) );
	QObject::connect(this->ui.listWidget,SIGNAL( currentRowChanged ( int  )),
		this ,SLOT( onPreferencesSelectChanged ( int  ) ) );	
}

PreferencesDialog::~PreferencesDialog()
{

}

void PreferencesDialog::onPreferencesSelectChanged(int index)
{
	QString iconPath ;
	QString title ;
	//const QPixmap *oldIcon = this->ui.label_6->pixmap()
	QIcon newIcon = this->ui.listWidget->item(index)->icon() ;
	this->ui.label_6->setPixmap( newIcon.pixmap( this->ui.label_6->size() ) );
	title = this->ui.listWidget->item(index)->text();
	this->ui.label_7->setText(title);
	this->ui.stackedWidget->setCurrentIndex(index);
}

