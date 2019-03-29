#ifndef IMAGEOFFSETMARK_H
#define IMAGEOFFSETMARK_H

#include "qgraphicsitem.h"

// A crosshair showing where the imaginary (0, 0) point of the image is. The actual offset
// is just a negated vector of the crosshair's position but this is easier to work with from
// the artist's point of view.

class ImageOffsetMark : public QGraphicsPixmapItem
{
public:

    ImageOffsetMark(QGraphicsItem* parent = nullptr);

    bool isHovered() const { return _isHovered; }
    QPointF getOldPos() const { return oldPosition; }

    void onPotentialMove(bool move);

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    QPointF oldPosition;        // Used for undo
    bool potentialMove = false; // Used for undo
    bool _isHovered = false;    // Internal attribute to help decide when to hide/show the offset crosshair
};

#endif // IMAGEOFFSETMARK_H
