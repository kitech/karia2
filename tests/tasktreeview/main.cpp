#include <stdlib.h>
#include <stdio.h>

#include <QtCore>
#include <QtWidgets>

#include "tasktreeview.h"

int main(int argc, char**argv)
{
    QApplication a(argc, argv);

    TaskTreeView ttv;
    ttv.show();

    return a.exec();
    return 0;
}
