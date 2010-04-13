#ifndef BATCHJOBMANDLG_H
#define BATCHJOBMANDLG_H

#include <QDialog>
#include "ui_batchjobmandlg.h"

class BatchJobManDlg : public QDialog
{
    Q_OBJECT

public:
    BatchJobManDlg(QWidget *parent = 0);
    ~BatchJobManDlg();
	
	QStringList getUrlList();

private:
    Ui::BatchJobManDlgClass ui;

	QStringList  mUrlList ;

private slots:

	void onWildcardOptionChanged(QString u = QString());
	
};

#endif // BATCHJOBMANDLG_H
