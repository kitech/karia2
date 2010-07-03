#include "skyserv.h"
#include "ui_skyserv.h"

SkyServ::SkyServ(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SkyServ)
{
    ui->setupUi(this);
}

SkyServ::~SkyServ()
{
    delete ui;
}
