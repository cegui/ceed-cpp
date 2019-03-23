#ifndef IMAGEENTRY_H
#define IMAGEENTRY_H

//!!!DBG TMP!
#include "qgraphicsitem.h"

// Represents the image of the imageset, can be drag moved, selected, resized, ...

//!!!ResizableRectItem subclass!
class ImageEntry : public QGraphicsRectItem
{
public:

    ImageEntry(QGraphicsItem* parent = nullptr);
};

#endif // IMAGEENTRY_H
