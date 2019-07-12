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

// Deselects all handles of this resizable
void ResizableRectItem::deselectAllHandles()
{
    for (QGraphicsItem* item : childItems())
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle) handle->setSelected(false);
    }
}

// Hides all handles. If a handle is given as the 'excluding' parameter, this handle is skipped over when hiding.
void ResizableRectItem::hideAllHandles(const QGraphicsItem* excluding)
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

// FIXME: dangerous overloading!
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

void ResizableRectItem::beginResizing(const QGraphicsItem& handle)
{
    assert(!_moveInProgress);
    _resizeInProgress = true;
    _resizeStartPos = pos();
    _resizeStartRect = rect();

    setPen(getPenWhileResizing());
    hideAllHandles(&handle);

    notifyResizeStarted();
}

// Adjusts the rectangle and returns actual used deltas (with restrictions accounted for) with in-out arguments
void ResizableRectItem::performResizing(qreal& deltaLeft, qreal& deltaTop, qreal& deltaRight, qreal& deltaBottom)
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

    QPointF newPos = pos() + rect().topLeft();
    QSizeF newSize(rect().width(), rect().height());
    notifyResizeProgress(newPos, newSize);
}

void ResizableRectItem::endResizing()
{
    if (!_resizeInProgress) return;

    _resizeInProgress = false;
    setPen(_mouseOver ? getHoverPen() : getNormalPen());

    QPointF newPos = pos() + rect().topLeft();
    QSizeF newSize(rect().width(), rect().height());

    _ignoreGeometryChanges = true;
    setPos(newPos);
    setRect(QRectF(0.0, 0.0, rect().width(), rect().height()));
    _ignoreGeometryChanges = false;

    notifyResizeFinished(newPos, newSize);
}

void ResizableRectItem::beginMoving()
{
    assert(!_resizeInProgress);
    _moveInProgress = true;
    _moveStartPos = pos();

    setPen(getPenWhileMoving());
    hideAllHandles();

    notifyMoveStarted();
}

// Moving may be performed internally by dragging an item itself
// or externally by dragging a handle (layout containers use it)
void ResizableRectItem::performMoving(QPointF& delta, bool external)
{
    auto currPos = pos() + rect().topLeft();
    auto newPos = constrainMovePoint(currPos + delta);

    delta = newPos - currPos;

    if (external)
    {
        // Emulate moving without touching our coord system,
        // so that handle moving happens in stable conditions
        setRect(rect().translated(delta));
    }

    notifyMoveProgress(newPos);
}

void ResizableRectItem::endMoving()
{
    if (!_moveInProgress) return;

    _moveInProgress = false;
    setPen(_mouseOver ? getHoverPen() : getNormalPen());

    QPointF newPos = pos() + rect().topLeft();

    _ignoreGeometryChanges = true;
    setPos(newPos);
    setRect(QRectF(0.0, 0.0, rect().width(), rect().height()));
    _ignoreGeometryChanges = false;

    notifyMoveFinished(newPos);
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
    endMoving();
    endResizing();
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
            deselectAllHandles();
        else
            hideAllHandles();
    }
    else if (change == ItemPositionChange)
    {
        if (!_ignoreGeometryChanges && !_moveInProgress)
            beginMoving();

        if (_moveInProgress)
        {
            auto delta = value.toPointF() - pos();
            performMoving(delta, false);
            return pos() + delta;
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
