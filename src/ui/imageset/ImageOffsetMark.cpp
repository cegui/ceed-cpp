#include "src/ui/imageset/ImageOffsetMark.h"
#include "qcursor.h"

ImageOffsetMark::ImageOffsetMark(QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent)
{
    setFlags(ItemIsMovable | ItemIsSelectable | ItemIgnoresTransformations | ItemSendsGeometryChanges);
    setCursor(Qt::OpenHandCursor);
    setPixmap(QPixmap(":/icons/imageset_editing/offset_crosshair.png"));

    // The crosshair pixmap is 15x15, (7, 7) is the centre pixel of it,
    // we want that to be the (0, 0) point of the crosshair
    setOffset(-7.0, -7.0);
    // Always show this above the label (which has ZValue = 0)
    setZValue(1.0);

    setAcceptHoverEvents(true);

    // By default Qt considers parts of the image with alpha = 0 not part of the image,
    // that would make it very hard to move the crosshair, we consider the whole
    // bounding rectangle to be part of the image
    setShapeMode(BoundingRectShape);
    setVisible(false);

    // Reset to unreachable value
    oldPosition.setX(-10000.0);
    oldPosition.setY(-10000.0);
}

QVariant ImageOffsetMark::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange)
    {
        if (potentialMove && oldPosition.x() < -9999.0) // FIXME: hack for 'not set' position
            oldPosition = pos();

        // Round the position to pixels
        QPointF newPosition = value.toPointF();
        newPosition.setX(round(newPosition.x() - 0.5) + 0.5);
        newPosition.setY(round(newPosition.y() - 0.5) + 0.5);
        return newPosition;
    }
    else if (change == ItemSelectedChange)
    {
        if (value.toBool())
            setVisible(true);
        else if (!parentItem()->isSelected())
            setVisible(false);
    }

    return QGraphicsPixmapItem::itemChange(change, value);
}

void ImageOffsetMark::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsPixmapItem::hoverEnterEvent(event);
    isHovered = true;
}

void ImageOffsetMark::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    isHovered = false;
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
