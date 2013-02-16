// optionui.h --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2013 liuguangzhao@users.sf.net
// URL: 
// Created: 2013-02-05 16:26:34 +0000
// Version: $Id: 897c173469acf4ab69e66febda3514c66ae5d17a $
// 
#ifndef _OPTIONUI_H_
#define _OPTIONUI_H_

#include "abstractui.h"


class TaskQueue;
class EAria2Man;
class TaskOption;

class OptionUi : public AbstractUi
{
    Q_OBJECT;
public:
    explicit OptionUi(Karia2 *pwin);
    virtual ~OptionUi();

    // virtual bool init();

public slots:

public:

private:

private:

    // temporary
    TaskQueue *mTaskMan;
    EAria2Man *mEAria2Man;
    Ui::Karia2 *mainUI;
};


#endif /* _OPTIONUI_H_ */
