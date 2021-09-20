#include "src/ui/ResizingHandle.h"
#include "src/ui/ResizableRectItem.h"
#include "qcursor.h"
#include "qpen.h"

ResizingHandle::ResizingHandle(Type type, ResizableRectItem* parent)
    : QGraphicsRectItem(parent)
    , _type(type)
{
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

    showHandle(false);

    if (isEdge())
    {
        setCursor(isHorizontal() ? Qt::SizeVerCursor : Qt::SizeHorCursor);
    }
    else
    {
        setZValue(1.0);

        if (_type == Type::TopRight || _type == Type::BottomLeft)
            setCursor(Qt::SizeBDiagCursor);
        else
            setCursor(Qt::SizeFDiagCursor);
    }

}

void ResizingHandle::onScaleChanged(qreal scaleX, qreal scaleY)
{
    auto tfm = transform();

    const qreal counterScaleX = isHorizontal() ? 1.0 : (1.0 / scaleX);
    const qreal counterScaleY = isVertical() ? 1.0 : (1.0 / scaleY);

    tfm = QTransform(counterScaleX, tfm.m12(), tfm.m13(),
                     tfm.m21(), counterScaleY, tfm.m23(),
                     tfm.m31(), tfm.m32(), tfm.m33());

    setTransform(tfm);
}

// Adjusts the parent rectangle and returns a position to use for this handle (with restrictions accounted for)
QPointF ResizingHandle::performResizing(QPointF value)
{
    const auto delta = value - pos();

    qreal left = 0.0;
    qreal top = 0.0;
    qreal right = 0.0;
    qreal bottom = 0.0;

    // Each handle type influences only a subset of params
    switch (_type)
    {
        case Type::Top: top = delta.y(); break;
        case Type::Bottom: bottom = delta.y(); break;
        case Type::Left: left = delta.x(); break;
        case Type::Right: right = delta.x(); break;
        case Type::TopLeft: top = delta.y(); left = delta.x(); break;
        case Type::BottomRight: bottom = delta.y(); right = delta.x(); break;
        case Type::BottomLeft: bottom = delta.y(); left = delta.x(); break;
        case Type::TopRight: top = delta.y(); right = delta.x(); break;
    }

    // Modifies left, right, top & bottom inside, results are actual changes
    const QPointF offset = static_cast<ResizableRectItem*>(parentItem())->performResizing(left, top, right, bottom);

    return pos() + offset;
}

void ResizingHandle::showHandle(bool show)
{
    QPen pen;
    if (show)
    {
        pen.setColor(QColor(0, 255, 255, 255));
        pen.setWidth(2.0);
        pen.setCosmetic(true);
    }
    else
    {
        pen.setColor(Qt::transparent);
    }

    setPen(pen);
}

// This method does most of the resize work
QVariant ResizingHandle::itemChange(GraphicsItemChange change, const QVariant& value)
{
    ResizableRectItem* parentResizable = static_cast<ResizableRectItem*>(parentItem());

    if (change == ItemSelectedChange)
    {
        // We disallow multi-selecting a resizable item and one of it's handles
        if (parentResizable->isSelected()) return false;
    }
    else if (change == ItemSelectedHasChanged)
    {
        // If we have indeed been selected, make sure all our sibling handles are unselected.
        // We allow multi-selecting multiple handles but only one handle per resizable is allowed.

        // Make sure all siblings of this handle are unselected
        const auto children = parentResizable->childItems();
        for (QGraphicsItem* item : children)
        {
            ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
            if (handle && handle != this) handle->setSelected(false);
        }

        parentResizable->notifyHandleSelected(this);
    }
    else if (change == ItemPositionChange)
    {
        // This is the money code
        // Changing position of the handle resizes the whole resizable
        if (!_ignoreGeometryChanges && !parentResizable->resizeInProgress())
            parentResizable->beginResizing(*this);

        if (parentResizable->resizeInProgress())
            return performResizing(value.toPointF());
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void ResizingHandle::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsRectItem::hoverEnterEvent(event);
    showHandle(true);
}

void ResizingHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    showHandle(false);
    QGraphicsRectItem::hoverLeaveEvent(event);
}
