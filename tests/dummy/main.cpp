#include <stdlib.h>
#include <stdio.h>
#include <sys/prctl.h>

#include <QtCore>
#include <QtWidgets>

#include "dummy.h"

int main(int argc, char**argv)
{
    int ret = prctl(PR_SET_NAME, (unsigned long) "mytestproc");
    QApplication a(argc, argv);

    ret = prctl(PR_SET_NAME, (unsigned long) "mytestproc2");
    qDebug()<<ret;

    Dummy ttv;
    ttv.show();

    return a.exec();
    return 0;
}
