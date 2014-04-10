
#include "dummy.h"
#include "ui_dummy.h"

Dummy::Dummy(QWidget *parent)
{
    mwin = new Ui::Dummy;
    mwin->setupUi(this);
}

Dummy::~Dummy()
{

}
