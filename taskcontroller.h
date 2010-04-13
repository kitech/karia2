#ifndef TASKCONTROLLER_H
#define TASKCONTROLLER_H

#include <QObject>
#include <QtCore>
#include <QMap>
#include <QVector>
#include <QThread>
#include <QMutex>

#include "task.h"

/**
 * 下载任务控制类
 *
 */
class TaskController : public QObject
{
	Q_OBJECT;
public:
	TaskController(QObject *parent = 0);
	~TaskController();

private:
	QMap< int , Task *> mHandle;
	QMutex mHMutex ;

};

#endif // TASKCONTROLLER_H
