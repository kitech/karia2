// globalpreferences.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-04-07 21:02:39 +0800
// Version: $Id$
// 
#include "globalpreferences.h"

GlobalPreferences * GlobalPreferences::mInstance = 0 ;

//static 
GlobalPreferences * GlobalPreferences::instance() 
{
	if( GlobalPreferences::mInstance == 0 )
	{
		GlobalPreferences::mInstance = new GlobalPreferences(0);
	}

	return GlobalPreferences::mInstance ;

}
GlobalPreferences::GlobalPreferences(QObject *parent)
	: QObject(parent)
{

}

GlobalPreferences::~GlobalPreferences()
{

}

//从永久存储中加载值。
bool GlobalPreferences::loadFromStorage()
{

	return true ;
}

//将值全部存储到永久存储中
bool GlobalPreferences::saveToStorage()
{

	return true ;
}

