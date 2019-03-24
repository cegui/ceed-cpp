#ifndef RESIZINGHANDLE_H
#define RESIZINGHANDLE_H

#include "qgraphicsitem.h"

// A rectangle that when moved resizes the parent resizable rect item.
// The reason to go with a child GraphicsRectItem instead of just overriding mousePressEvent et al
// is to easily support multi selection resizing (you can multi-select various edges in all imaginable
// combinations and resize many things at once).

class ResizingHandle : public QGraphicsRectItem
{
public:

    ResizingHandle(QGraphicsItem* parent = nullptr);
};

#endif // RESIZINGHANDLE_H
