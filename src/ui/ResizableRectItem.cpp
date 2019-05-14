#include "src/ui/ResizableRectItem.h"
#include "src/ui/ResizingHandle.h"
#include "qcursor.h"
#include "qpen.h"

ResizableRectItem::ResizableRectItem(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setFlags(ItemSendsGeometryChanges | ItemIsMovable);
    setAcceptHoverEvents(true);

    topEdgeHandle = new ResizingHandle(ResizingHandle::Type::Top, this);
    bottomEdgeHandle = new ResizingHandle(ResizingHandle::Type::Bottom, this);
    leftEdgeHandle = new ResizingHandle(ResizingHandle::Type::Left, this);
    rightEdgeHandle = new ResizingHandle(ResizingHandle::Type::Right, this);
    topRightCornerHandle = new ResizingHandle(ResizingHandle::Type::TopRight, this);
    bottomRightCornerHandle = new ResizingHandle(ResizingHandle::Type::BottomRight, this);
    bottomLeftCornerHandle = new ResizingHandle(ResizingHandle::Type::BottomLeft, this);
    topLeftCornerHandle = new ResizingHandle(ResizingHandle::Type::TopLeft, this);

    hideAllHandles();

    setPen(getNormalPen()); // Doesn't work for derived classes so we have to call it there manually
    setCursor(Qt::OpenHandCursor);
}

// Unselects all handles of this resizable
void ResizableRectItem::unselectAllHandles()
{
    for (QGraphicsItem* item : childItems())
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle) handle->setSelected(false);
    }
}

// Hides all handles. If a handle is given as the 'excluding' parameter, this handle is skipped over when hiding.
void ResizableRectItem::hideAllHandles(ResizingHandle* excluding)
{
    for (QGraphicsItem* item : childItems())
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle && handle != excluding)
            handle->showHandle(false);
    }
}

// Makes it possible to disable or enable resizing
void ResizableRectItem::setResizingEnabled(bool enabled)
{
    for (QGraphicsItem* item : childItems())
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle) handle->setVisible(enabled);
    }
}

void ResizableRectItem::setRect(QRectF newRect)
{
    if (newRect != rect()) _handlesDirty = true;
    QGraphicsRectItem::setRect(newRect);
    updateHandles();
}

// Checks whether any of the 8 handles is selected.
// NB: At most 1 handle can be selected at a time!
bool ResizableRectItem::isAnyHandleSelected() const
{
    for (QGraphicsItem* item : childItems())
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle && handle->isSelected()) return true;
    }

    return false;
}

// Adjusts the rectangle and returns a 4-tuple of the actual used deltas (with restrictions accounted for)
// Deltas are in-out, returning the actual change applied. The default implementation doesn't use the handle parameter.
void ResizableRectItem::performResizing(const ResizingHandle& /*handle*/, qreal& deltaLeft, qreal& deltaTop, qreal& deltaRight, qreal& deltaBottom)
{
    auto newRect = rect().adjusted(deltaLeft, deltaTop, deltaRight, deltaBottom);
    newRect = constrainResizeRect(newRect, rect());

    // TODO: the rect moves as a whole when it can't be sized any less
    //       this is probably not the behavior we want!

    deltaLeft = newRect.left() - rect().left();
    deltaTop = newRect.top() - rect().top();
    deltaRight = newRect.right() - rect().right();
    deltaBottom = newRect.bottom() - rect().bottom();

    setRect(newRect);
}

QRectF ResizableRectItem::constrainResizeRect(QRectF rect, QRectF /*oldRect*/)
{
    auto minSize = getMinSize();
    auto maxSize = getMaxSize();

    QRectF minRect(rect.center() - QPointF(0.5 * minSize.width(), 0.5 * minSize.height()), minSize);
    rect = rect.united(minRect);

    QRectF maxRect(rect.center() - QPointF(0.5 * maxSize.width(), 0.5 * maxSize.height()), maxSize);
    rect = rect.intersected(maxRect);

    return rect;
}

QPen ResizableRectItem::getNormalPen() const
{
    QPen ret(Qt::DotLine);
    ret.setColor(QColor(255, 255, 255, 150));
    return ret;
}

QPen ResizableRectItem::getHoverPen() const
{
    return QPen(QColor(0, 255, 255, 255));
}

QPen ResizableRectItem::getPenWhileResizing() const
{
    return QPen(QColor(255, 0, 255, 255));
}

QPen ResizableRectItem::getPenWhileMoving() const
{
    return QPen(QColor(255, 0, 255, 255));
}

void ResizableRectItem::onScaleChanged(qreal scaleX, qreal scaleY)
{
    _currentScaleX = scaleX;
    _currentScaleY = scaleY;

    for (QGraphicsItem* item : childItems())
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle)
        {
            handle->onScaleChanged(scaleX, scaleY);
            continue;
        }

        ResizableRectItem* rectItem = dynamic_cast<ResizableRectItem*>(item);
        if (rectItem)
        {
            rectItem->onScaleChanged(scaleX, scaleY);
            continue;
        }
    }

    _handlesDirty = true;
    updateHandles();
}

void ResizableRectItem::mouseReleaseEventSelected(QMouseEvent* /*event*/)
{
    if (_moveInProgress)
    {
        _moveInProgress = false;
        notifyMoveFinished(pos());
    }
}

void ResizableRectItem::notifyResizeStarted(ResizingHandle* handle)
{
    _resizeInProgress = true;
    resizeOldPos = pos();
    resizeOldRect = rect();

    setPen(getPenWhileResizing());
    hideAllHandles(handle);
}

void ResizableRectItem::notifyResizeFinished(QPointF newPos, QRectF newRect)
{
    _resizeInProgress = false;
    setPen(_mouseOver ? getHoverPen() : getNormalPen());

    _ignoreGeometryChanges = true;
    setRect(newRect);
    setPos(newPos);
    _ignoreGeometryChanges = false;
}

// Updates all the handles according to geometry
void ResizableRectItem::updateHandles()
{
    // Updating handles while resizing would mess things up big times, so we just ignore the update in that circumstance
    if (!_handlesDirty || _resizeInProgress) return;

    auto absoluteWidth = _currentScaleX * rect().width();
    auto absoluteHeight = _currentScaleY * rect().height();

    const bool useInnerHandles = (absoluteWidth < 4 * _outerHandleSize || absoluteHeight < 4 * _outerHandleSize);

    topEdgeHandle->ignoreGeometryChanges(true);
    topEdgeHandle->setPos(0.0, 0.0);
    if (useInnerHandles)
        topEdgeHandle->setRect(0.0, -_innerHandleSize, rect().width(), _innerHandleSize);
    else
        topEdgeHandle->setRect(0.0, 0.0, rect().width(), _outerHandleSize);
    topEdgeHandle->ignoreGeometryChanges(false);

    bottomEdgeHandle->ignoreGeometryChanges(true);
    bottomEdgeHandle->setPos(0.0, rect().height());
    if (useInnerHandles)
        bottomEdgeHandle->setRect(0.0, 0.0, rect().width(), _innerHandleSize);
    else
        bottomEdgeHandle->setRect(0.0, -_outerHandleSize, rect().width(), _outerHandleSize);
    bottomEdgeHandle->ignoreGeometryChanges(false);

    leftEdgeHandle->ignoreGeometryChanges(true);
    leftEdgeHandle->setPos(0.0, 0.0);
    if (useInnerHandles)
        leftEdgeHandle->setRect(-_innerHandleSize, 0.0, _innerHandleSize, rect().height());
    else
        leftEdgeHandle->setRect(0.0, 0.0, _outerHandleSize, rect().height());
    leftEdgeHandle->ignoreGeometryChanges(false);

    rightEdgeHandle->ignoreGeometryChanges(true);
    rightEdgeHandle->setPos(rect().width(), 0.0);
    if (useInnerHandles)
        rightEdgeHandle->setRect(0.0, 0.0, _innerHandleSize, rect().height());
    else
        rightEdgeHandle->setRect(-_outerHandleSize, 0.0, _outerHandleSize, rect().height());
    rightEdgeHandle->ignoreGeometryChanges(false);

    topRightCornerHandle->ignoreGeometryChanges(true);
    topRightCornerHandle->setPos(rect().width(), 0.0);
    if (useInnerHandles)
        topRightCornerHandle->setRect(0.0, -_innerHandleSize, _innerHandleSize, _innerHandleSize);
    else
        topRightCornerHandle->setRect(-_outerHandleSize, 0.0, _outerHandleSize, _outerHandleSize);
    topRightCornerHandle->ignoreGeometryChanges(false);

    bottomRightCornerHandle->ignoreGeometryChanges(true);
    bottomRightCornerHandle->setPos(rect().width(), rect().height());
    if (useInnerHandles)
        bottomRightCornerHandle->setRect(0.0, 0.0, _innerHandleSize, _innerHandleSize);
    else
        bottomRightCornerHandle->setRect(-_outerHandleSize, -_outerHandleSize, _outerHandleSize, _outerHandleSize);
    bottomRightCornerHandle->ignoreGeometryChanges(false);

    bottomLeftCornerHandle->ignoreGeometryChanges(true);
    bottomLeftCornerHandle->setPos(0.0, rect().height());
    if (useInnerHandles)
        bottomLeftCornerHandle->setRect(-_innerHandleSize, 0.0, _innerHandleSize, _innerHandleSize);
    else
        bottomLeftCornerHandle->setRect(0.0, -_outerHandleSize, _outerHandleSize, _outerHandleSize);
    bottomLeftCornerHandle->ignoreGeometryChanges(false);

    topLeftCornerHandle->ignoreGeometryChanges(true);
    topLeftCornerHandle->setPos(0.0, 0.0);
    if (useInnerHandles)
        topLeftCornerHandle->setRect(-_innerHandleSize, -_innerHandleSize, _innerHandleSize, _innerHandleSize);
    else
        topLeftCornerHandle->setRect(0.0, 0.0, _outerHandleSize, _outerHandleSize);
    topLeftCornerHandle->ignoreGeometryChanges(false);

    _handlesDirty = false;
}

QVariant ResizableRectItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        if (value.toBool())
            unselectAllHandles();
        else
            hideAllHandles();
    }
    else if (change == ItemPositionChange)
    {
        auto point = constrainMovePoint(value.toPointF());
        if (!_moveInProgress && !_ignoreGeometryChanges)
        {
            _moveInProgress = true;
            moveOldPos = pos();

            setPen(getPenWhileMoving());

            hideAllHandles();

            notifyMoveStarted();
        }

        if (_moveInProgress)
        {
            // 'point' is the new position, self.pos() is the old position,
            // we use 'point' to avoid the 1 pixel lag
            notifyMoveProgress(point);
        }
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void ResizableRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsRectItem::hoverEnterEvent(event);
    setPen(getHoverPen());
    _mouseOver = true;
}

void ResizableRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    _mouseOver = false;
    setPen(getNormalPen());
    QGraphicsRectItem::hoverLeaveEvent(event);
}
