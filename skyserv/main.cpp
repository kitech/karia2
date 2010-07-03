#include <QtGui/QApplication>
#include "skyserv.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SkyServ w;
    w.show();

    return a.exec();
}
