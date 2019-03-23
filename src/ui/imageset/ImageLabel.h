#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include "qgraphicsitem.h"

// Text item showing image's label when the image is hovered or selected.
// You should not use this directly! Use ImageEntry.name instead to get the name.

class ImageLabel : public QGraphicsTextItem
{
public:
    ImageLabel();
};

#endif // IMAGELABEL_H
