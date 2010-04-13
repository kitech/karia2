#ifndef OPTIONDLG_H
#define OPTIONDLG_H

#include <QDialog>
#include "ui_optiondlg.h"

//单一实例模式全局设置
class GlobalOption : public QObject
{
	Q_OBJECT
public:
	static GlobalOption * instance();
	~GlobalOption();

private:
	GlobalOption(QObject * parent = 0 );
	static GlobalOption * mHandle ;
	void initDefaultValue();

public:	//Option name list

	//general

	//proxy

	//connection

	//Protocol

	//monitor

	//mirror

	//Graph / log

	//filemanager

	//dailup network

	//schedule

	//sound

	//site manager 

	//other
	int mIsLimitSpeed ;		//maybe 1 or 0
	int mMaxLimitSpeed ;	//maybe 0.1k or 1024M(G)	, 100 bytes to 1024M*1024*1024
	int mIsMemberLimitSpeed;	//maybe 1 or 0 

};
/////////////

/////////////

class OptionDlg : public QDialog
{
    Q_OBJECT

public:
    OptionDlg(QWidget *parent = 0);
    ~OptionDlg();

private:
    Ui::OptionDlgClass ui;
};

#endif // OPTIONDLG_H
