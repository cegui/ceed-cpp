#ifndef LAYOUTCONTAINERHANDLE_H
#define LAYOUTCONTAINERHANDLE_H

#include <qgraphicsitem.h>

class LayoutManipulator;

class LayoutContainerHandle : public QGraphicsPixmapItem
{
public:

    LayoutContainerHandle(LayoutManipulator& host);
};

#endif // LAYOUTCONTAINERHANDLE_H
