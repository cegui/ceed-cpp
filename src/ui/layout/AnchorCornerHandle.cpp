#include "src/ui/layout/AnchorCornerHandle.h"
#include <qcursor.h>

AnchorCornerHandle::AnchorCornerHandle(bool left, bool top, QGraphicsItem* parent, qreal size, const QPen& pen, QColor hoverColor)
    : QGraphicsPolygonItem(parent)
    , _normalPen(pen)
    , _hoverColor(hoverColor)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

    QPolygonF poly;
    poly << QPointF(0.0, 0.0);
    poly << QPointF(size * (left ? -2.0 : 2.0), top ? -size : size);
    poly << QPointF(left ? -size : size, size * (top ? -2.0 : 2.0));
    setPolygon(poly);

    updatePen(false);
    setCursor(Qt::SizeAllCursor);
}

void AnchorCornerHandle::updatePen(bool hovered)
{
    if (hovered)
    {
        QPen newPen(_normalPen);
        newPen.setColor(_hoverColor);
        setPen(newPen);
    }
    else
        setPen(_normalPen);
}

QVariant AnchorCornerHandle::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange)
    {
        //
    }

    return QGraphicsPolygonItem::itemChange(change, value);
}

void AnchorCornerHandle::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsPolygonItem::hoverEnterEvent(event);
    updatePen(true);
}

void AnchorCornerHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    updatePen(false);
    QGraphicsPolygonItem::hoverLeaveEvent(event);
}
