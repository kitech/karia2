#ifndef DLRULES_H
#define DLRULES_H

#include <QDialog>
#include "ui_dlrules.h"

class DLRules : public QDialog
{
    Q_OBJECT

public:
    DLRules(QWidget *parent = 0);
    ~DLRules();

private:
    Ui::DLRulesClass ui;
};

#endif // DLRULES_H
