#ifndef DIRCOMPLETERMODEL_H
#define DIRCOMPLETERMODEL_H

#include <QDirModel>

class DirCompleterModel : public QDirModel
{
	Q_OBJECT

public:
	DirCompleterModel(QObject *parent);
	~DirCompleterModel();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
	
};

#endif // DIRCOMPLETERMODEL_H
