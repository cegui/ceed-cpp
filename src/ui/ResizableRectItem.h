#ifndef RESIZABLERECTITEM_H
#define RESIZABLERECTITEM_H

#include "qgraphicsitem.h"

// Rectangle that can be resized by dragging it's handles.
// Inherit from this class to gain resizing and moving capabilities.
// Depending on the size, the handles are shown outside the rectangle (if it's small)
// or inside (if it's large). All this is tweakable.

class ResizableRectItem : public QGraphicsRectItem
{
public:

    ResizableRectItem(QGraphicsItem *parent = nullptr);
};

#endif // RESIZABLERECTITEM_H
