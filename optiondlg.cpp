#include "optiondlg.h"

GlobalOption *GlobalOption::mHandle = 0 ;

GlobalOption::GlobalOption(QObject * parent )
:QObject(parent)
{
	this->initDefaultValue();
}

GlobalOption::~GlobalOption()
{
	
}

GlobalOption * GlobalOption::instance()
{
	if( GlobalOption::mHandle == 0 )
	{
		GlobalOption::mHandle = new GlobalOption();
	}
	return GlobalOption::mHandle ;
}
void GlobalOption::initDefaultValue()
{
	//other
	mIsLimitSpeed = 1 ;	
	mMaxLimitSpeed = 10*1024 ;	//10KB/s
	mIsMemberLimitSpeed = 1;	
}


////////////////////////////////////////
OptionDlg::OptionDlg(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}

OptionDlg::~OptionDlg()
{

}
