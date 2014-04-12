#include <stdlib.h>
#include <stdio.h>

#include <QtCore>
#include <QtWidgets>

#include "simplelog.h"
#include "tasktreeview.h"

int main(int argc, char**argv)
{
    ::qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);

    TaskTreeView ttv;
    ttv.show();

    return a.exec();
    return 0;
}
