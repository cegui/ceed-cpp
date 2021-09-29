#include "src/ui/layout/AnchorCornerHandle.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/Application.h"
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

void AnchorCornerHandle::updateStatusMessage(bool newMoveOpposite)
{
    auto layoutScene = static_cast<LayoutScene*>(scene());
    if (auto target = layoutScene->getAnchorTarget())
    {
        const bool ctrl = (QApplication::keyboardModifiers() & Qt::ControlModifier);
        QString helpMsg = "Drag to change anchors of <i>" + target->getWidgetPath(true) + "</i>, " +
                (ctrl ?
                     "release <b>Ctrl</b> to preserve current widget size." :
                     "hold <b>Ctrl</b> to resize the widget accordingly.") +
                (newMoveOpposite ?
                     " Handle will shift opposite anchor on collision, grab farther from the tip to change this." :
                     " Handle will stop at opposite anchor on collision, grab closer to the tip to change this.")
                + " <b>Right click</b> to show presets.";
        qobject_cast<Application*>(qApp)->getMainWindow()->setStatusMessage(helpMsg);
    }
    else
    {
        qobject_cast<Application*>(qApp)->getMainWindow()->setStatusMessage("");
    }
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
    const bool newMoveOpposite = QPointF::dotProduct(event->pos(), event->pos()) < _moveOppositeThreshold;
    updateStatusMessage(newMoveOpposite);
}

void AnchorCornerHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    _moveOpposite = false;
    updateBrush();
    updatePen(false);
    QGraphicsPolygonItem::hoverLeaveEvent(event);

    qobject_cast<Application*>(qApp)->getMainWindow()->setStatusMessage("");
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
    updateStatusMessage(newMoveOpposite);
    QGraphicsPolygonItem::hoverMoveEvent(event);
}
