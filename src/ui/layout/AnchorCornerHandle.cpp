#include "src/ui/layout/AnchorCornerHandle.h"
#include "src/ui/layout/LayoutScene.h"
#include <qcursor.h>
#include <qgraphicssceneevent.h>

AnchorCornerHandle::AnchorCornerHandle(bool left, bool top, QGraphicsItem* parent, qreal size, const QPen& pen, QColor hoverColor)
    : QGraphicsPolygonItem(parent)
    , _normalPen(pen)
    , _hoverColor(hoverColor)
    , _moveOppositeThreshold(size * size * 1.5)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges | ItemIgnoresTransformations);
    setAcceptHoverEvents(true);

    QPolygonF poly;
    poly << QPointF(0.0, 0.0);
    poly << QPointF(size * (left ? -2.0 : 2.0), top ? -size : size);
    poly << QPointF(left ? -size : size, size * (top ? -2.0 : 2.0));
    setPolygon(poly);

    updatePen(false);
    setCursor(Qt::SizeAllCursor);
    setZValue(1.0);
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

void AnchorCornerHandle::updateBrush()
{
    setBrush(_moveOpposite ? QBrush(QColor(_hoverColor.red(), _hoverColor.green(), _hoverColor.blue(), 127)) : QBrush());
}

QVariant AnchorCornerHandle::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemPositionChange)
    {
        QPointF newPos = value.toPointF();
        static_cast<LayoutScene*>(scene())->anchorHandleMoved(this, newPos, _moveOpposite);
        return newPos;
    }
    else if (change == ItemSelectedHasChanged)
    {
        if (value.toBool())
            static_cast<LayoutScene*>(scene())->anchorHandleSelected(this);
    }

    return QGraphicsPolygonItem::itemChange(change, value);
}

void AnchorCornerHandle::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsPolygonItem::hoverEnterEvent(event);
    updatePen(true);
    //???call some LayoutScene method to set status message in a main window?
}

void AnchorCornerHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    //???call some LayoutScene method to clear status message in a main window?
    _moveOpposite = false;
    updateBrush();
    updatePen(false);
    QGraphicsPolygonItem::hoverLeaveEvent(event);
}

void AnchorCornerHandle::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    //!!!ONLY WHEN NOT DRAGGING! start drag is easy, stop drag must be catched!
    const bool newMoveOpposite = QPointF::dotProduct(event->pos(), event->pos()) < _moveOppositeThreshold;
    if (_moveOpposite != newMoveOpposite)
    {
        //???call some LayoutScene method to set status message in a main window?
        _moveOpposite = newMoveOpposite;
        updateBrush();
    }
    QGraphicsPolygonItem::hoverMoveEvent(event);
}
