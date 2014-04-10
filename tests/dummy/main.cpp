#include <stdlib.h>
#include <stdio.h>

#include <QtCore>
#include <QtWidgets>

#include "dummy.h"

int main(int argc, char**argv)
{
    QApplication a(argc, argv);

    Dummy ttv;
    ttv.show();

    return a.exec();
    return 0;
}
