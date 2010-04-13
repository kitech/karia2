#include "dircompletermodel.h"

DirCompleterModel::DirCompleterModel(QObject *parent)
	: QDirModel(parent)
{

}

DirCompleterModel::~DirCompleterModel()
{

}

QVariant DirCompleterModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole && index.column() == 0) {
		QString path  = QDir::toNativeSeparators(filePath(index));
		if (path.endsWith(QDir::separator()))
			path.chop(1);
		return path;
	}

	return QDirModel::data(index, role);
}
