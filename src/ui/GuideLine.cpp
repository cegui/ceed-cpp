#include "src/ui/GuideLine.h"
#include <qgraphicsscene.h>
#include <qcursor.h>

GuideLine::GuideLine(bool horizontal, QGraphicsItem* parent,
                     size_t width, Qt::PenStyle style, QColor normalColor, QColor hoverColor, size_t mouseInteractionDistance)
    : QGraphicsLineItem(parent)
    , _style(style)
    , _normalColor(normalColor)
    , _hoverColor(hoverColor)
    , _width(width)
    , _mouseInteractionDistance(mouseInteractionDistance)
    , _horizontal(horizontal)
{
    setFlags(ItemSendsGeometryChanges | ItemIsMovable);
    setAcceptHoverEvents(true);

    updateLine();
    updatePen(false);
    setCursor(horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor);
    //setZValue(100.0);
}

QPainterPath GuideLine::shape() const
{
    const qreal thickness = 2.0 * _mouseInteractionDistance + static_cast<qreal>(_width);

    QPainterPath path;
    if (_horizontal)
    {
        path.addRect(0.0, -0.5 * thickness, scene()->width(), thickness);
    }
    else
    {
        path.addRect(-0.5 * thickness, 0.0, thickness, scene()->height());
    }
    return path;
}

void GuideLine::updateLine()
{
    //???base on parent and not on a scene? Use scene when no parent?
    if (!scene()) return;

    if (_horizontal)
        setLine(0.0, 0.0, scene()->width(), 0.0);
    else
        setLine(0.0, 0.0, 0.0, scene()->height());
}

void GuideLine::updatePen(bool hovered)
{
    QPen newPen(_style);
    newPen.setColor(hovered ? _hoverColor : _normalColor);
    newPen.setWidth(static_cast<int>(_width));
    newPen.setCosmetic(true);
    setPen(newPen);
}

QVariant GuideLine::itemChange(GraphicsItemChange change, const QVariant& value)
{
    //???if scene/parent resized too?
    if (change == GraphicsItemChange::ItemSceneHasChanged ||
        change == GraphicsItemChange::ItemParentHasChanged)
    {
        updateLine();
    }
    else if (change == ItemPositionChange)
    {
        auto newPos = value.toPointF();
        if (_horizontal) newPos.setX(pos().x());
        else newPos.setY(pos().y());

        return newPos;
    }

    return QGraphicsLineItem::itemChange(change, value);
}

void GuideLine::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsLineItem::hoverEnterEvent(event);
    updatePen(true);
}

void GuideLine::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    updatePen(false);
    QGraphicsLineItem::hoverLeaveEvent(event);
}
