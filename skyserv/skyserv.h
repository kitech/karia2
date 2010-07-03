#ifndef SKYSERV_H
#define SKYSERV_H

#include <QMainWindow>

namespace Ui {
    class SkyServ;
}

class SkyServ : public QMainWindow
{
    Q_OBJECT

public:
    explicit SkyServ(QWidget *parent = 0);
    ~SkyServ();

private:
    Ui::SkyServ *ui;
};

#endif // SKYSERV_H
