#ifndef _SQLITETASKTREEMODEL_H_
#define _SQLITETASKTREEMODEL_H_

#include <stdio.h>

#include <QtCore>

class ModelTreeNode
{
public:
    ModelTreeNode();
    virtual ~ModelTreeNode();

    // private:
    ModelTreeNode * _parent = NULL;
    QVector<ModelTreeNode*> _childs;
    void * _data = NULL;
    QString _nid;
};

#endif /* _SQLITETASKTREEMODEL_H_ */
