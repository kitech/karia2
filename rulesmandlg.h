#ifndef RULESMANDLG_H
#define RULESMANDLG_H

#include <QDialog>
#include "ui_rulesmandlg.h"

class RulesManDlg : public QDialog
{
    Q_OBJECT

public:
    RulesManDlg(QWidget *parent = 0);
    ~RulesManDlg();

private:
    Ui::RulesManDlgClass ui;
};

#endif // RULESMANDLG_H
