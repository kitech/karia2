#ifndef _VIEW_MODEL_H
#define _VIEW_MODEL_H

#include <QtCore>
#include <QtGui>
#include <QVector>
#include <QVariant>
#include <QAbstractListModel>
#include <QStandardItemModel>
#include <QPixmap>
#include <QIcon>
#include <QTextStream>
#include <QtXml>
#include <QDomImplementation>
#include <QDomDocumentType>
#include <QDomDocument>
#include <QDomComment>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomElement>
#include <QDomText>
#include <QDomNamedNodeMap>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMap>

//////////////////////
    #include <QDomNode>
    #include <QHash>

    class DomItem
    {
    public:
        DomItem(QDomNode &node, int row, DomItem *parent = 0);
        ~DomItem();
        DomItem *child(int i);
        DomItem *parent();
        QDomNode node() const;
        int row();
		bool insertRows(int rows,DomItem *child ) ;
		int childCount();
		bool removeRow(int row ) ;
    private:
        QDomNode domNode;
        QHash<int,DomItem*> childItems;
        DomItem *parentItem;
        int rowNumber;
    };


//全局配置，使用单例模式
class ConfigDatabase : public QObject
{
	Q_OBJECT
public:
	static ConfigDatabase * instance(QObject *parent=0) ;
	static ConfigDatabase * mHandle ;
		
	ConfigDatabase(QObject *parent=0)
		:QObject(parent)
	{
		this->mConfigFileName = "ngc.xml";
		this->mDatabaseHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
		this->mNSUrl = "http://www.gzl.org";
		this->mRootString = "nullget";

		this->mHomePath = QDir::homePath();	//only use on unix system

		this->fillDefaultConfigurePairs();

		this->mAppPath = QCoreApplication::applicationDirPath();
		this->mAppName = QCoreApplication::applicationName();


#ifdef WIN32

		this->mConfigPath = this->mAppPath ;
		this->mCatRootPath = "C:/NGDownload";
#else
		this->mConfigPath = this->mHomePath + QString("/.%1").arg(qApp->applicationName()) ;
		this->mCatRootPath = QString("%1/%2").arg(this->mHomePath).arg("NGDownload");

#endif
		QDir tmpDir ;
		if( ! tmpDir.exists(this->mConfigPath) )	//如果目录目录不存在则创建之
		{
			tmpDir.mkdir(this->mConfigPath);
			tmpDir.mkpath(this->mConfigPath);
		}
		if( ! tmpDir.exists(this->mCatRootPath))
		{
			tmpDir.mkdir(this->mCatRootPath);
			tmpDir.mkpath(this->mCatRootPath);
		}

		this->mOpened = false ;

		this->mCatModel = new QStandardItemModel(this);
		this->mTaskListModel = new QStandardItemModel(this);

	}

	~ConfigDatabase(){ delete this->mCatModel ;	}
	
	bool writeDefaultDatabase();	//新写一只有默认设置的配置数据库

	/**
	 * 打开默认的数据库。
	 */
	bool open()
	{
		if(QFile::exists(this->mConfigPath+"/"+this->mConfigFileName) )
		{
			QString errorMsg;
			int errorLine;
			int errorColumn;

			QFile fp(this->mConfigPath+"/"+this->mConfigFileName);
			fp.open(QIODevice::ReadOnly);
			this->mDocument.setContent(&fp,&errorMsg,&errorLine,&errorColumn);
			this->mRootElem = this->mDocument.documentElement();

			qDebug()<<"load done: "<< errorMsg<<errorLine<<errorColumn ;
			//qDebug()<<this->mDocument.toString();
		}
		else
		{
			this->writeDefaultDatabase();
		}
		this->mOpened = true ;		

		return true ;
	}
	bool close()
	{
		if( !this->mOpened ) return false ;

		this->mOpened = false ;
		this->save();
		this->mDocument.clear();
		this->mTaskListModel->clear();
		this->mCatModel->clear();

		return true ;
	}
	bool save()
	{
			//save database
			QFile cf(this->mConfigPath+"/"+this->mConfigFileName);
			cf.open(QIODevice::WriteOnly);
			
			QTextStream ts (&cf);

			//ts<<this->mDatabaseHeader<<endl ;
			this->mDocument.save(ts,4);

			qDebug()<<this->mDocument.toString()<<"===============================";

			return true ;
	}


	QStandardItemModel * createCatModel()
	{
		if(this->mOpened == false ) this->open();
		
		int catCount ;
		this->mCatModel->clear();

		QModelIndex idx , rootIndex ;
		QDomNode node ;
		QDomNodeList dnl = this->mRootElem.elementsByTagName("category");

		this->mCatModel->insertColumn(0);
		this->mCatModel->insertRow(0);
		rootIndex = idx = this->mCatModel->index(0,0);
		this->mCatModel->setData(idx,"NullGet");
		this->mCatModel->setHeaderData(0 , Qt::Horizontal," ");
		
		catCount = dnl.size();
		if( catCount > 0 )

		for(int i =0 ; i < catCount ; ++i)
		{
			if( i == 0 ) 
			{
				this->mCatModel->insertColumn(0,rootIndex);
			}
			node = dnl.at(i);
			QDomElement elem = node.toElement();
			QString an = elem.attribute("name");
			this->mCatModel->insertRow(i,rootIndex);
			idx = this->mCatModel->index(i,0,rootIndex);
			this->mCatModel->setData(idx,an);
			
		}
		qDebug()<<dnl.size();

		return this->mCatModel ;
	}

	bool isOpened() { return this->mOpened ; }
	/**
	 * 
	 */
	bool saveTaskList( QVector<QMap<QString , QString> >& pTaskList , QMap<QString,QVector<QMap<QString,QString> > >  & pSegList )
	{
		if(this->mOpened == false ) this->open();

		int taskCount ;
		QDomElement elem,segElem ;
		QMap<QString,QString> cat , seg;
		QVector<QMap<QString,QString> > segList ;

		taskCount = pTaskList.size();

		//delete exist task elem,problem here
		QVector<QDomNode> nv;
		QDomNodeList tmp;
		QDomNode cnode = this->mRootElem.firstChild();
		while( ! cnode .isNull () )
		{
			qDebug()<<cnode.toElement().tagName();
			if( cnode.toElement().tagName().compare(QString("task")) == 0  )
			{
				nv.append(cnode);				
			}
			cnode = cnode.nextSibling();
		}
		for(int i=0 ; i< nv.size() ; i ++ )
		{
			this->mRootElem.removeChild(nv.at(i));
			//qDebug()<<tmp.at(i).nodeName();
		}
		
		qDebug()<<this->mDocument.toString()<<"++++++++++++++++++++++++++++++++++++";
		qDebug()<<taskCount<<nv.size() ;
		
		//save new task elem
		for(int i = 0 ; i < taskCount ; i ++ )
		{
			QMapIterator<QString,QString> it ( pTaskList[i] );
			elem = this->mDocument.createElement("task");
			while(it.hasNext())
			{
				it.next();
				elem.setAttribute(it.key(),it.value());				
				
				if( it.key() == QString("ID")  )
				{									
					segList = pSegList[it.value()];
					
					for( int k = 0 ;k < segList.size() ; ++ k )
					{
						segElem =this->mDocument.createElement("segment");
						seg = segList[k];
						QMapIterator<QString,QString> sit ( seg );
						while(sit.hasNext())
						{
							sit.next();
							segElem.setAttribute(sit.key(),sit.value());
						}
						elem.appendChild(segElem);
					}
				}
			}
			this->mRootElem.appendChild(elem);
		}
		
		return this->save();
	}

	bool loadTaskList( QVector<QMap<QString , QString> >& pTaskList , QMap<QString,QVector<QMap<QString,QString> > >  & pSegList )
	{
		if(this->mOpened == false ) this->open();

		int count = 7 ;
		int cols ;
		int segcnt;

		QVector<QMap<QString , QString> > taskList;
		
		QVector<QMap<QString,QString> > seg;
		QMap<QString,QVector<QMap<QString,QString> > > segList;

		QMap<QString , QString> taskParam,segParam;
		QModelIndex idx ;

		QDomElement taskElem , segElem ;
		QDomNodeList taskNodeList = this->mRootElem.elementsByTagName("task");
		QDomNodeList segNodeList;

		count = taskNodeList.size();
			
		pTaskList.clear();
		pSegList.clear();

		for(int i = 0 ;i < count ; ++i)
		{		
			QString taskId;
			taskParam.clear();
			taskElem = taskNodeList.at(i).toElement();
			QDomNamedNodeMap taskNodeMap = taskElem.attributes();
			
			cols = taskNodeMap.size();
			for(int j=0;j<cols ; ++j)
			{			
				taskParam.insert( taskNodeMap.item(j).nodeName(),taskNodeMap.item(j).toAttr().value() );
				if( taskNodeMap.item(j).nodeName() == "ID" )
				{
					taskId =  taskNodeMap.item(j).toAttr().value() ;
					segNodeList = taskElem.elementsByTagName("segment");
					segcnt = segNodeList.size();
					seg.clear();
					for(int k = 0 ; k < segcnt ; ++k )
					{
						segParam.clear() ;
						QDomNamedNodeMap segNodeMap = segNodeList.at(k).attributes();
						for(int m = 0 ; m < taskNodeMap.size() ; ++m)
						{
							segParam.insert( segNodeMap.item(m).nodeName() ,segNodeMap.item(m).toAttr().value());
						}
						seg.append(segParam);
					}
					pSegList.insert(taskId,seg);
				}
			}
			pTaskList.append(taskParam);
		}

		//qDebug()<<pTaskList;
		//qDebug()<<pSegList.size();

		return true  ;
	}

	//
	
	QDomNode & getCategoryTree(QDomNode &domroot);

	static QString categoryTagName() ;

	QDomNode createNode(QString tagName );  

	//取单独一个配置值。
	QString  getConfigValue(QString name) { return this->mDefaultConfigureValue[name] ; } 

	//取配置文件所在的全路径
	QString getConfigFileName(){ return this->mConfigPath+"/"+this->mConfigFileName ; } 

private:
	QString mDatabaseHeader ;
	QString mAppPath ;
	QString mAppName ;
	QString mConfigPath ;
	QString mConfigFileName ;
	QString mHomePath ;
	QString mCatRootPath ;
	QDomDocument mDocument ;
	QDomElement mRootElem;
	QString mNSUrl ;
	QString mRootString ;
	bool mOpened ;

	QMap<QString,QString> mDefaultConfigureValue ;	//默认值。
	QMap<QString,QString> mUserDefaultConfigureValue ;	//用户已经修改了的默认值。

	QStandardItemModel *mCatModel;
	QStandardItemModel *mTaskListModel;

	bool initialMemoryDatabase();
	void fillDefaultConfigurePairs() ;//初始化OPTION对话框属性值。

};

//这个类现在用了吗？没有吧。
class TaskListViewModel : public QAbstractListModel
{
	 Q_OBJECT

public:
	TaskListViewModel(QObject * parent = 0) : QAbstractListModel(parent){}
	~TaskListViewModel(){};

    int rowCount(const QModelIndex &parent) const
	{
		qDebug()<<this->mList.size();

		return this->mList.size() ;
	}
    int columnCount(const QModelIndex &parent) const
	{
		return this->mList[0].size();
	}

    QVariant data(const QModelIndex &index, int role) const
	{
		//qDebug()<<index.row()<< index.column() ;

		static QIcon folder(QPixmap(":/images/folder.png"));

		if (role == Qt::DisplayRole)
		return "Item " + QString::number(index.row()) + ":" + QString::number(index.column());
		if (role == Qt::DecorationRole)
		return qVariantFromValue(folder);
		return QVariant();

	}
    QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		//return QVariant(QString("header%1").arg(section) );

		static QIcon services(QPixmap(":/images/services.png"));
		if (role == Qt::DisplayRole)
			return QString::number(section);
		if (role == Qt::DecorationRole)
			return qVariantFromValue(services);
		return QAbstractItemModel::headerData( section, orientation, role );

		return QVariant();
	}
	bool setItemData ( const QModelIndex & index, const QMap<int, QVariant> & roles ) 
	{
		return true;
	}
	bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole )
	{
		int row = index.row();
		int col = index.column();
		QVector<QVariant> trow;
		if( row >= this->mList.size())
		{
			trow.append(value);
			this->mList.append(trow);
		}
		else
		{
			trow= this->mList.value(col);
			if( col >= trow.size() )
			{
				trow.append(value);
			}
		}
		qDebug()<<index.row()<<index.column()<<value;
		//this->mList.value(index.row()).replace(index.column(),value);
		return true;
	}
	bool insertTaskInfo(QVector<QVariant> row)
	{		
		QVariant aa;
		int i=0;
		foreach(aa,row)
		{
			this->setData(this->createIndex(this->mList.size(),i++),aa);
		}

		return true ;
	}

private:
	QVector< QVector<QVariant> > mList ;
};



///category model , like QDirModol , but the item source is not file system directorys
	class CategoryModel : public QAbstractItemModel   
    {
        Q_OBJECT

    public:

		static CategoryModel * instance ( QObject *parent = 0 ) ;
		static CategoryModel * mHandle ;

		bool reload();	//重新从配置文件中读取
        ~CategoryModel();

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
		bool rowMoveTo( const QModelIndex & from , const QModelIndex & to );

    private:

		 CategoryModel( QObject *parent = 0);
        //QDomDocument domDocument;
		QDomNode catRootNode ;

        DomItem *rootItem;

		ConfigDatabase *mConfigDB;
    };

#endif
