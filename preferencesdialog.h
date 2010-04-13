#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtCore>
#include <QtGui>

#include <QDialog>
#include "ui_preferencesdialog.h"

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

private:
    Ui::PreferencesDialogClass ui;
	bool  mIsModified ;

private slots:
	void onPreferencesSelectChanged(int index);

};

#endif // PREFERENCESDIALOG_H
