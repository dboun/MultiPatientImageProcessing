#ifndef MITK_IO_H
#define MITK_IO_H

#include <QString>

#include <mitkDataNode.h>

namespace MitkIO
{
    static mitk::DataNode::Pointer CreateNodeFromFile(QString path);
    
    static bool WriteNodeToFile(mitk::DataNode::Pointer node);
};

#endif // ! MITK_IO_H