#include "taskcontroller.h"

TaskController::TaskController(QObject *parent)
	: QObject(parent)
{
	this->mHandle.clear();
	this->mHMutex.lock();
	this->mHMutex.unlock();
}

TaskController::~TaskController()
{

}
