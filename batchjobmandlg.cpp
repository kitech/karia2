#include <QtCore>
#include "batchjobmandlg.h"

BatchJobManDlg::BatchJobManDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);

	///
	QObject::connect(ui.bjmd_le_wildcard_url,SIGNAL(textChanged(QString)),this,SLOT(onWildcardOptionChanged(QString )));
	QObject::connect(ui.bjmd_le_wildcard_begin,SIGNAL(textChanged(QString)),this,SLOT(onWildcardOptionChanged()));
	QObject::connect(ui.bjmd_le_wildcard_end,SIGNAL(textChanged(QString)),this,SLOT(onWildcardOptionChanged()));
	QObject::connect(ui.bjmd_rb_wildcard_char,SIGNAL(clicked()),this,SLOT(onWildcardOptionChanged()));
	QObject::connect(ui.bjmd_rb_wildcard_digit,SIGNAL(clicked()),this,SLOT(onWildcardOptionChanged()));
	QObject::connect(ui.bjmd_sb_wildcard_begin,SIGNAL(valueChanged(int)),this,SLOT(onWildcardOptionChanged()));
	QObject::connect(ui.bjmd_sb_wildcard_end,SIGNAL(valueChanged(int)),this,SLOT(onWildcardOptionChanged()));
	QObject::connect(ui.bjmd_sb_wildcard_size,SIGNAL(valueChanged(int)),this,SLOT(onWildcardOptionChanged()));

}

BatchJobManDlg::~BatchJobManDlg()
{

}

//每一个参数的改变都会调用这个值。
void BatchJobManDlg::onWildcardOptionChanged( QString u )
{
	QString wurl ;
	int ifrom =0, ito = 0 ;
	int isize = 0;
	QString sfrom , sto ;
	char * cfrom  = 0 , * cto = 0  ;
	char tmp[8] = {0};
	
	QString wildstr ;

	wurl = this->ui.bjmd_le_wildcard_url->text();
	this->mUrlList.clear();

	if( this->ui.bjmd_rb_wildcard_digit->isChecked())
	{
		ifrom = ui.bjmd_sb_wildcard_begin->value();
		ito = ui.bjmd_sb_wildcard_end->value();
		isize = ui.bjmd_sb_wildcard_size->value();

		if( ifrom <= ito )
		{
			for( int i = ifrom ; i <= ito ; ++i)
			{
				wildstr = QString("%1").arg(i) ;
				if(wildstr.length()<isize)
				{
					wildstr = wildstr.rightJustified(isize,QLatin1Char('0'));
				}
				//qDebug()<<wildstr;
				wurl = wurl.replace(QString("(*)"),QString("%1")) ;
				wildstr = wurl.arg(wildstr);
				//qDebug()<<wildstr;
				this->mUrlList.append(wildstr) ;
			}
		}
	}
	else if( this->ui.bjmd_rb_wildcard_char->isChecked() )
	{
		sfrom = ui.bjmd_le_wildcard_begin->text();
		sto = ui.bjmd_le_wildcard_end->text();

		if( sfrom.toAscii().at(0) < sto.toAscii().at(0) )
		{
			for( char i = sfrom.toAscii().at(0) ; i <= sto.toAscii().at(0) ; ++i)
			{				
				wildstr = QString("%1").arg(i) ;
				//qDebug()<<"for"<<wildstr;
				this->mUrlList.append((wurl.replace("(*)","%1")).arg(wildstr));
			}			
		}
	}
	else
	{
		//no op need
	}

	this->ui.bjmd_lw_url_list->clear();
	for( int i = 0 ; i < this->mUrlList.size() ; ++ i)
	{
		this->ui.bjmd_lw_url_list->addItem(this->mUrlList.at(i));
	}
}

QStringList BatchJobManDlg::getUrlList()
{	
	return this->mUrlList ;
}