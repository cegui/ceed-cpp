#ifndef IMAGEOFFSETMARK_H
#define IMAGEOFFSETMARK_H

#include "qgraphicsitem.h"

// A crosshair showing where the imaginary (0, 0) point of the image is. The actual offset
// is just a negated vector of the crosshair's position but this is easier to work with from
// the artist's point of view.

class ImageOffsetMark : public QGraphicsPixmapItem
{
public:
    ImageOffsetMark();
};

#endif // IMAGEOFFSETMARK_H
