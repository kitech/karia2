// optionmanager.cpp --- 
// 
// Author: liuguangzhao
// Copyright (C) 2007-2010 liuguangzhao@users.sf.net
// URL: 
// Created: 2010-05-05 14:01:52 +0800
// Version: $Id$
// 

#include "optionmanager.h"


OptionManager *OptionManager::mInstance = 0;

//static 
OptionManager *OptionManager::instance() 
{
	if (OptionManager::mInstance == 0) {
		OptionManager::mInstance = new OptionManager(0);
	}

	return OptionManager::mInstance;

}
OptionManager::OptionManager(QObject *parent)
	: QObject(parent)
{
    this->storage = SqliteStorage::instance();
}

OptionManager::~OptionManager()
{
}

QString OptionManager::loadKey(QString key, QString dvalue)
{
    QString ov = QString::null;
    QString optionValue;

    optionValue = this->storage->getUserOption(key);
    if (optionValue == QString::null) {
        optionValue = this->storage->getDefaultOption(key);
        if (optionValue == QString::null) {
            this->storage->addDefaultOption(key, dvalue, "auto");
            ov = dvalue;
        } else {
            ov = optionValue;
        }
    } else {
        ov = optionValue;
    }

    return ov;
}

bool OptionManager::saveKey(QString key, QString value, QString type)
{
    QString ov = QString::null;
    QString optionValue;

    optionValue = this->loadKey(key, "");
    if (optionValue != value) {
        this->storage->deleteUserOption(key);
        this->storage->addUserOption(key, value, type);
    }

    return true;
}

// template

QString OptionManager::getMaxSpeed(QString key) 
{
    if (this->mUserOptions.contains(key)) {
        return this->mUserOptions.value(key).value;
    }
    return QString();
}
