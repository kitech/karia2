#ifndef SEGMENTLOGMODEL_H
#define SEGMENTLOGMODEL_H

#include <QVector>
#include <QVariant>

#include <QAbstractItemModel>

class SegmentLogModel : public QAbstractItemModel
{
	Q_OBJECT

public:
    static SegmentLogModel * instance(int task_id , int seg_id , QObject *parent);
	static bool removeInstance( int task_id , int seg_id   );

    ~SegmentLogModel();

private:    
	SegmentLogModel(int task_id , int seg_id , QObject *parent);
	static QMap< QString , SegmentLogModel*> mHandle ;

	QVector<QVector< QVariant> >  mModelData ;
	QVector<QString >  mLogColumns ;
	QString  mColumnsLine ;

	int mTaskID;
	int mSegID ;

	char * logCols ;

public:

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const;
	QModelIndex parent(const QModelIndex &child) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );
	virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() ) ;
	virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() ) ;
	

public slots:
	//inherient
	bool submit () ;
	//inherient
	void revert () ;


};


#endif // SEGMENTLOGMODEL_H
