#include "src/ui/layout/AnchorCornerHandle.h"
#include "src/ui/layout/LayoutScene.h"
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

void AnchorCornerHandle::setPosSilent(const QPointF& newPos)
{
    setFlag(ItemSendsGeometryChanges, false);
    setPos(newPos);
    setFlag(ItemSendsGeometryChanges, true);
}

void AnchorCornerHandle::setPosSilent(qreal x, qreal y)
{
    setFlag(ItemSendsGeometryChanges, false);
    setPos(x, y);
    setFlag(ItemSendsGeometryChanges, true);
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
        QPointF delta = value.toPointF() - pos();
        static_cast<LayoutScene*>(scene())->anchorHandleMoved(this, delta);
        return pos() + delta;
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
