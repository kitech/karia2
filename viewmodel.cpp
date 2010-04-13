
 #include <QtGui>

#include "viewmodel.h"

/**
 * 配置数据库类实现。
 */
ConfigDatabase * ConfigDatabase::mHandle = 0;

ConfigDatabase * ConfigDatabase::instance(QObject *parent)
{
	if( ConfigDatabase::mHandle == 0 )
	{
		ConfigDatabase::mHandle = new ConfigDatabase(parent);
	}

	return ConfigDatabase::mHandle ;
}

QDomNode &  ConfigDatabase::getCategoryTree(QDomNode &domroot)
{
	if( ! this->isOpened() ) this->open();

	domroot = this->mRootElem.elementsByTagName("category_block").item(0);
	//domroot = this->mDocument ;
	//qDebug()<<domroot.isNull();

	return domroot ;
}

QString ConfigDatabase::categoryTagName() 
{
	return QString("category");
}
QDomNode ConfigDatabase::createNode(QString tagName )
{
	return this->mDocument.createElement(tagName);

}

//初始化OPTION对话框属性值。
void ConfigDatabase::fillDefaultConfigurePairs()
{
		//option 对话框的变量默认值
		this->mDefaultConfigureValue.insert("language","Chinese");
		this->mDefaultConfigureValue.insert("skin","default");
		this->mDefaultConfigureValue.insert("usebandwidthlimit","false");
		this->mDefaultConfigureValue.insert("bandwidthlimittype","unlimited");
		this->mDefaultConfigureValue.insert("rememberbandwidthsetting","true");
		this->mDefaultConfigureValue.insert("maxbandwidth","10240");
		this->mDefaultConfigureValue.insert("maxtaskcount","20");
		this->mDefaultConfigureValue.insert("maxsegmentcount","20");
		this->mDefaultConfigureValue.insert("maxconnectretrytimes","3");
		this->mDefaultConfigureValue.insert("maxreadretrytimes","4");
		this->mDefaultConfigureValue.insert("maxwriteretrytimes","5");
		this->mDefaultConfigureValue.insert("retrydelay","3");
		this->mDefaultConfigureValue.insert("randomretrydelay","false");
		this->mDefaultConfigureValue.insert("cachebufferlength","204800");	//以K计
		this->mDefaultConfigureValue.insert("socketconnecttimeout","5");	
		this->mDefaultConfigureValue.insert("socketdatatimeout","5");
		this->mDefaultConfigureValue.insert("minsegmentsize","256");

		//connection
		this->mDefaultConfigureValue.insert("maxsimultaneousjobs" , "10" ) ;

}

	bool ConfigDatabase::writeDefaultDatabase()
	{
		
		QDomDocumentType ddt;
		QDomImplementation di;
		
		this->mDocument = di.createDocument( this->mNSUrl ,this->mRootString , ddt );

		qDebug()<<( this->mConfigPath+"/"+this->mConfigFileName ) ;
		
		//if file_exist
		if( QFile::exists( this->mConfigPath+"/"+this->mConfigFileName ) )
		{
			qDebug()<<"configure database already exist" ; 
		}
		else
		{
			this->mRootElem =  this->mDocument.documentElement();
			QDomElement configElem = this->mDocument.createElement("configure");
			QDomElement catBlockElem = this->mDocument.createElement("category_block");
			QDomElement catRootElem = this->mDocument.createElement("category");
			QDomElement tmpElem ;
			QDomAttr tmpAttr ;
			QDomText tmpText;
			
			catBlockElem.appendChild(catRootElem);
			this->mRootElem.appendChild(configElem);
			this->mRootElem.appendChild(catBlockElem);
			
			//create default database 
			catRootElem.setAttribute("name","NullGet");
			catRootElem.setAttribute("folded","yes");
			catRootElem.setAttribute("path","");
			
			QDomElement catElem = this->mDocument.createElement("category");
			catElem.setAttribute("name","Download");
			catElem.setAttribute("folded","no");
			catElem.setAttribute("path",this->mCatRootPath);
			catRootElem.appendChild(catElem);

			catElem = this->mDocument.createElement("category");
			catRootElem.appendChild(catElem);
						
			catElem.setAttribute("name","Downloaded");
			catElem.setAttribute("folded","no");
			catElem.setAttribute("path",this->mCatRootPath);

			tmpElem = this->mDocument.createElement("category");
			tmpElem.setAttribute("name","software");
			tmpElem.setAttribute("folded","no");
			tmpElem.setAttribute("path",QString("%1/%2").arg(this->mCatRootPath).arg("software"));
			catElem.appendChild(tmpElem);
			tmpElem = this->mDocument.createElement("category");
			tmpElem.setAttribute("name","game");
			tmpElem.setAttribute("folded","no");
			tmpElem.setAttribute("path",QString("%1/%2").arg(this->mCatRootPath).arg("game"));
			catElem.appendChild(tmpElem);
			tmpElem = this->mDocument.createElement("category");
			tmpElem.setAttribute("name","music");
			tmpElem.setAttribute("folded","no");
			tmpElem.setAttribute("path",QString("%1/%2").arg(this->mCatRootPath).arg("music"));
			catElem.appendChild(tmpElem);
			tmpElem = this->mDocument.createElement("category");
			tmpElem.setAttribute("name","movie");
			tmpElem.setAttribute("folded","no");
			tmpElem.setAttribute("path",QString("%1/%2").arg(this->mCatRootPath).arg("movie"));
			catElem.appendChild(tmpElem);

			catElem = this->mDocument.createElement("category");
			catRootElem.appendChild(catElem);
			catElem.setAttribute("name","Deleted");
			catElem.setAttribute("folded","no");
			catElem.setAttribute("path",this->mCatRootPath);
			
			//ceate configures
			configElem.setAttribute("name","default");
			QMapIterator<QString, QString> it(this->mDefaultConfigureValue);
			while (it.hasNext()) 
			{
				it.next();
				//cout << i.key() << ": " << i.value() << endl;
				tmpElem = this->mDocument.createElement(it.key());
				tmpText = this->mDocument.createTextNode(it.value());
				tmpElem.appendChild(tmpText);
				configElem.appendChild(tmpElem);
			}

			//save database
			this->save();

			qDebug()<<this->mDocument.toString();

			//this->mDocument.setContent(this->mDatabaseHeader+this->mDocument.toString());

			//qDebug()<<this->mDocument.toString();

		}

		return false ;
	}

/////////////////////
//////////////////
    DomItem::DomItem(QDomNode &node, int row, DomItem *parent)
    {
        domNode = node;
        // Record the item's location within its parent.
        rowNumber = row;
        parentItem = parent;
    }

    DomItem::~DomItem()
    {
        QHash<int,DomItem*>::iterator it;
        for (it = childItems.begin(); it != childItems.end(); ++it)
            delete it.value();
    }

    QDomNode DomItem::node() const
    {
        return domNode;
    }

    DomItem *DomItem::parent()
    {
        return parentItem;
    }

    DomItem *DomItem::child(int i)
    {
        if (childItems.contains(i))
            return childItems[i];

        if (i >= 0 && i < domNode.childNodes().count()) {
            QDomNode childNode = domNode.childNodes().item(i);
            DomItem *childItem = new DomItem(childNode, i, this);
            childItems[i] = childItem;
            return childItem;
        }
        return 0;
    }

    int DomItem::row()
    {
        return rowNumber;
    }
	int DomItem::childCount()
	{
		return this->childItems.count() ;
	}

	bool DomItem::insertRows(int rows,DomItem *child ) 
	{
		this->childItems.insert(rows,child);
		this->rowNumber+= rows ; 
		return true ;
	} 
	bool DomItem::removeRow(int row )
	{
		this->childItems.clear();		
		return true;
	}

//////////////////////////

	
	CategoryModel * CategoryModel::mHandle = 0 ;

	CategoryModel * CategoryModel::instance ( QObject *parent  ) 
	{
		if( CategoryModel::mHandle == 0 )
		{
			CategoryModel::mHandle = new CategoryModel(parent);
		}

		return CategoryModel::mHandle ;

	}

    CategoryModel::CategoryModel( QObject *parent)
        : QAbstractItemModel(parent) 
    {
		mConfigDB = ConfigDatabase::instance();

		catRootNode = mConfigDB->getCategoryTree(catRootNode);

        rootItem = new DomItem( catRootNode , 0);
    }

	bool  CategoryModel::reload()	//重新从配置文件中读取
	{
		mConfigDB = ConfigDatabase::instance();

		catRootNode = mConfigDB->getCategoryTree(catRootNode);

		DomItem *tmpRoot = rootItem ;
        rootItem = new DomItem( catRootNode , 0);
		delete tmpRoot ; tmpRoot = 0 ;

		return true ;
	}
    CategoryModel::~CategoryModel()
    {
        delete rootItem;
		CategoryModel::mHandle = 0 ;
    }

    int CategoryModel::columnCount(const QModelIndex &/*parent*/) const
    {
       // return 3 ;
		 return 2 ;
    }

    QVariant CategoryModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();
		if( role == Qt::DecorationRole )
		{
			return QImage("icons/crystalsvg/16x16/filesystems/folder_violet_open.png");
		}

        if (role != Qt::DisplayRole)
            return QVariant();

        DomItem *item = static_cast<DomItem*>(index.internalPointer());

        QDomNode node = item->node();
        QStringList attributes;
        QDomNamedNodeMap attributeMap = node.attributes();
		
        switch (index.column()) {
            case 0:
                //return node.nodeName();
				return node.toElement().attribute("name");
            case 2:
                for (int i = 0; (unsigned int)(i) < attributeMap.count(); ++i) {
                    QDomNode attribute = attributeMap.item(i);
                    attributes << attribute.nodeName() + "=\""
                                  +attribute.nodeValue() + "\"";
                }
                return attributes.join(" ");
            case 1:
                //return node.nodeValue().split("\n").join(" ");
				return node.toElement().attribute("path");
            default:
                return QVariant();
        }
    }

    Qt::ItemFlags CategoryModel::flags(const QModelIndex &index) const
    {
        if (!index.isValid())
            return Qt::ItemIsEnabled;

        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    QVariant CategoryModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
                case 0:
                    return tr("Cat Name");
                case 1:
                    return tr("Cat Path");
				case 2:
                    return tr("Cat Attributes");
                default:
                    return QVariant();
            }
        }

        return QVariant();
    }

    QModelIndex CategoryModel::index(int row, int column, const QModelIndex &parent)
                const
    {
        DomItem *parentItem;

        if (!parent.isValid())
            parentItem = rootItem;
        else
            parentItem = static_cast<DomItem*>(parent.internalPointer());

        DomItem *childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }

    QModelIndex CategoryModel::parent(const QModelIndex &child) const
    {
        if (!child.isValid())
            return QModelIndex();

        DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
        DomItem *parentItem = childItem->parent();

        if (!parentItem || parentItem == rootItem)
            return QModelIndex();

        return createIndex(parentItem->row(), 0, parentItem);
    }

    int CategoryModel::rowCount(const QModelIndex &parent) const
    {
        DomItem *parentItem;

        if (!parent.isValid())
            parentItem = rootItem;
        else
            parentItem = static_cast<DomItem*>(parent.internalPointer());

        return parentItem->node().childNodes().count();
    }

	bool CategoryModel::insertRows ( int row, int count, const QModelIndex & parent  )
	{
		qDebug()<<__FUNCTION__<<row;
		
		DomItem *item = static_cast<DomItem*>(parent.internalPointer());
		QDomNode pnode = item->node();

		QDomElement nelem ;
		

		nelem = this->mConfigDB->createNode("category").toElement() ;
		qDebug()<<nelem.tagName();
		nelem.setAttribute("folder","no");
		pnode.appendChild(nelem);
		
		qDebug()<<item->row()<<nelem.tagName();
		//item->insertRows(count,new DomItem(nelem,row,item));
		
		//delete rootItem ;
		//rootItem = new DomItem( catRootNode , 0);

		
		return true ;
	}

	bool CategoryModel::removeRows ( int row, int count, const QModelIndex & parent  )
	{
		qDebug()<<__FUNCTION__<<row;
		
		DomItem *item = static_cast<DomItem*>(parent.internalPointer());
		DomItem *ditem ;
		QDomNode pnode = item->node();
		QDomNode cnode , dnode ;

		dnode = item->child(row)->node();

		//delete this->rootItem;
		qDebug()<<pnode.childNodes().count() ;
		pnode.removeChild(dnode) ;
		qDebug()<<pnode.childNodes().count();
		item->removeRow(row);

		return true ;
	}

	bool CategoryModel::setData ( const QModelIndex & index , const QVariant & value, int role )
	{
		qDebug()<<__FUNCTION__<<index.row()<<index.column()<<index.data();

		//return true ;

        DomItem *item = static_cast<DomItem*>(index.internalPointer());
		QDomElement cnode = item->node().toElement() ;
		qDebug()<<cnode.tagName() ;

		switch( index.column() )
		{
		case 0:
			cnode.setAttribute("name",value.toString());
			break;
		case 1 :
			cnode.setAttribute("path",value.toString());
		}
		
		return true ;
	}

	bool CategoryModel::rowMoveTo( const QModelIndex & from , const QModelIndex & to )
	{
		QDomNode fnode , tnode ;
		DomItem * fitem , * titem ;

		bool isChild = false ;
		//see if to is child or sub child of from 
		{
			QModelIndex idx , cidx;
			QModelIndexList mil ;
			mil.append(from);
			while( mil.count() > 0 )
			{
				idx = mil.takeFirst();
				for( int i = 0 ;i < idx.model()->rowCount(idx) ; i ++ )
				{
					cidx = idx.child(i,0);
					if( cidx == to ) 
					{
						isChild = true ; break ;
					}
					else
					{
						mil.append(cidx);
					}
				}
			}
		}

		qDebug()<<isChild;
		if( isChild ) 
		{
			qDebug()<<"Can't move parent to child cat";
			return false ;
		}

		fitem = static_cast<DomItem*>(from.internalPointer()); 
		titem = static_cast<DomItem*>(to.internalPointer()); 

		fnode = fitem->node();
		tnode = titem->node();

		tnode.appendChild(fnode);
		titem->removeRow(0);
		
		this->removeRows(from.row(),1,from.parent());


		return true ;
	}

