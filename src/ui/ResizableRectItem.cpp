#include "src/ui/ResizableRectItem.h"
#include "src/ui/ResizingHandle.h"
#include <qcursor.h>
#include <qpen.h>
#include <cmath>

ResizableRectItem::ResizableRectItem(QGraphicsItem* parent)
    : QGraphicsRectItem(parent)
{
    setFlags(ItemSendsGeometryChanges | ItemIsMovable);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

    topEdgeHandle = new ResizingHandle(ResizingHandle::Type::Top, this);
    bottomEdgeHandle = new ResizingHandle(ResizingHandle::Type::Bottom, this);
    leftEdgeHandle = new ResizingHandle(ResizingHandle::Type::Left, this);
    rightEdgeHandle = new ResizingHandle(ResizingHandle::Type::Right, this);
    topRightCornerHandle = new ResizingHandle(ResizingHandle::Type::TopRight, this);
    bottomRightCornerHandle = new ResizingHandle(ResizingHandle::Type::BottomRight, this);
    bottomLeftCornerHandle = new ResizingHandle(ResizingHandle::Type::BottomLeft, this);
    topLeftCornerHandle = new ResizingHandle(ResizingHandle::Type::TopLeft, this);

    hideAllHandles();

    setPen(ResizableRectItem::getNormalPen()); // Doesn't work for derived classes so we have to call it there manually
    setCursor(Qt::OpenHandCursor);
}

// Checks whether any of the 8 handles is selected.
// NB: At most 1 handle can be selected at a time!
bool ResizableRectItem::isAnyHandleSelected() const
{
    const auto children = childItems();
    for (QGraphicsItem* item : children)
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle && handle->isSelected()) return true;
    }

    return false;
}

// Deselects all handles of this resizable
void ResizableRectItem::deselectAllHandles()
{
    const auto children = childItems();
    for (QGraphicsItem* item : children)
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle) handle->setSelected(false);
    }
}

// Hides all handles. If a handle is given as the 'excluding' parameter, this handle is skipped over when hiding.
void ResizableRectItem::hideAllHandles(const QGraphicsItem* excluding)
{
    const auto children = childItems();
    for (QGraphicsItem* item : children)
    {
        ResizingHandle* handle = dynamic_cast<ResizingHandle*>(item);
        if (handle && handle != excluding)
            handle->showHandle(false);
    }
}

// Makes it possible to disable or enable resizing
void ResizableRectItem::setResizingEnabled(bool enabled)
{
    const auto children = childItems();
    for (QGraphicsItem* item : children)
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

void ResizableRectItem::performResizing(qreal deltaLeft, qreal deltaTop, qreal deltaRight, qreal deltaBottom)
{
    const auto desiredRect = rect().adjusted(deltaLeft, deltaTop, deltaRight, deltaBottom);
    setRect(constrainResizeRect(desiredRect, rect()));
    notifyResizeProgress(pos() + rect().topLeft(), rect().size());
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

QRectF ResizableRectItem::constrainResizeRect(QRectF rect, QRectF oldRect)
{
    qreal w = rect.width();
    qreal h = rect.height();

    auto minSize = getMinSize();
    const auto maxSize = getMaxSize();
    if (maxSize.width() > 0.0 && minSize.width() > maxSize.width())
        minSize.setWidth(maxSize.width());
    if (maxSize.height() > 0.0 && minSize.height() > maxSize.height())
        minSize.setHeight(maxSize.height());

    if (w < minSize.width())
        w = minSize.width();
    else if (maxSize.width() > 0.0 && w > maxSize.width())
        w = maxSize.width();

    if (h < minSize.height())
        h = minSize.height();
    else if (maxSize.height() > 0.0 && h > maxSize.height())
        h = maxSize.height();

    // If limited, subtract from each change proportionally to aviod unintended movement
    const auto desiredSizeChange = rect.size() - oldRect.size();
    const auto constrainedSizeChange = QSizeF(w, h) - oldRect.size();
    if (desiredSizeChange != constrainedSizeChange)
    {
        qreal deltaLeft = rect.left() - oldRect.left();
        qreal deltaRight = rect.right() - oldRect.right();
        qreal deltaTop = rect.top() - oldRect.top();
        qreal deltaBottom = rect.bottom() - oldRect.bottom();
        if (desiredSizeChange.width() != 0.0 && desiredSizeChange.width() != constrainedSizeChange.width())
        {
            const qreal coeff = constrainedSizeChange.width() / desiredSizeChange.width();
            deltaLeft *= coeff;
            deltaRight *= coeff;
        }
        if (desiredSizeChange.height() != 0.0 && desiredSizeChange.height() != constrainedSizeChange.height())
        {
            const qreal coeff = constrainedSizeChange.height() / desiredSizeChange.height();
            deltaTop *= coeff;
            deltaBottom *= coeff;
        }
        rect = oldRect.adjusted(deltaLeft, deltaTop, deltaRight, deltaBottom);
    }

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

    const auto children = childItems();
    for (QGraphicsItem* item : children)
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

void ResizableRectItem::mouseReleaseEventSelected()
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
        {
            deselectAllHandles();
        }
        else
        {
            hideAllHandles();
            endMoving();
            endResizing();
        }
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
