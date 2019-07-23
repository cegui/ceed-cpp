#include "src/cegui/CEGUIManipulator.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/cegui/CEGUIManager.h"
#include "src/cegui/QtnPropertyUVector2.h"
#include "src/cegui/QtnPropertyUVector3.h"
#include "src/cegui/QtnPropertyUSize.h"
#include "src/cegui/QtnPropertyURect.h"
#include "src/cegui/QtnPropertyUBox.h"
#include "src/ui/CEGUIGraphicsScene.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include "qgraphicsscene.h"
#include "qpainter.h"
#include <qmessagebox.h>
#include <CEGUI/widgets/TabControl.h>
#include <CEGUI/widgets/ScrollablePane.h>
#include <CEGUI/widgets/ScrolledContainer.h>
#include <CEGUI/widgets/GridLayoutContainer.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/CoordConverter.h>
#include "QtnProperty/PropertySet.h"
#include "QtnProperty/Core/PropertyQString.h"
#include "QtnProperty/Core/PropertyBool.h"
#include "QtnProperty/Core/PropertyUInt.h"
#include "QtnProperty/Core/PropertyFloat.h"
#include "QtnProperty/Core/PropertyEnum.h"
#include "QtnProperty/Delegates/Core/PropertyDelegateQString.h"

// recursive - if true, even children of given widget are wrapped
// skipAutoWidgets - if true, auto widgets are skipped (only applicable if recursive is True)
CEGUIManipulator::CEGUIManipulator(QGraphicsItem* parent, CEGUI::Window* widget)
    : ResizableRectItem(parent)
    , _widget(widget)
{
    setFlags(ItemIsFocusable | ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);

    createPropertySet();
}

CEGUIManipulator::~CEGUIManipulator()
{
    delete _propertySet;
}

void CEGUIManipulator::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->save();

    if (preventManipulatorOverlap())
    {
        // NOTE: This is just an option because it's very performance intensive, most people editing big layouts will
        //       want this disabled. But it makes editing nicer and fancier :-)

        // We are drawing the outlines after CEGUI has already been rendered so he have to clip overlapping parts
        // we basically query all items colliding with ourselves and if that's a manipulator and is over us we subtract
        // that from the clipped path.
        QPainterPath clipPath;
        clipPath.addRect(QRectF(-scenePos().x(), -scenePos().y(), scene()->sceneRect().width(), scene()->sceneRect().height()));

        // FIXME: I used self.collidingItems() but that seems way way slower than just going over everything on the scene
        //        in reality we need siblings of ancestors recursively up to the top
        //
        //        this just begs for optimisation in the future
        auto collidingItems = scene()->items();
        for (QGraphicsItem* item : collidingItems)
        {
            if (!item->isVisible() || item == this) continue;

            CEGUIManipulator* manipulator = dynamic_cast<CEGUIManipulator*>(item);
            if (!manipulator) continue;

            // Check if the item is above us
            // FIXME: nasty nasty way to do this
            bool isAbove = false;
            for (QGraphicsItem* itm : scene()->items())
            {
                if (itm == this)
                {
                    isAbove = true;
                    break;
                }
                else if (itm == item) break;
            }

            if (isAbove)
            {
                QPainterPath boundingClipPath;
                boundingClipPath.addRect(item->boundingRect());
                clipPath = clipPath.subtracted(boundingClipPath.translated(item->scenePos() - scenePos()));
            }
        }

        // We clip using stencil buffers to prevent overlapping outlines appearing
        // FIXME: This could potentially get very slow for huge layouts
        painter->setClipPath(clipPath);
    }

    impl_paint(painter, option, widget);

    painter->restore();
}

void CEGUIManipulator::impl_paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    ResizableRectItem::paint(painter, option, widget);

    if (!_widget) return;

    const bool drawGuides = (isSelected() || _resizeInProgress || isAnyHandleSelected());
    if (!drawGuides) return;

    // Draw guides

    const auto baseSize = getBaseSize();
    const auto widgetPosition = _widget->getPosition();
    const auto widgetSize = _widget->getSize();

    const auto scaleXInPixels = static_cast<qreal>(CEGUI::CoordConverter::asAbsolute(CEGUI::UDim(widgetPosition.d_x.d_scale, 0.f), baseSize.d_width));
    const auto offsetXInPixels = static_cast<qreal>(widgetPosition.d_x.d_offset);

    const auto scaleYInPixels = static_cast<qreal>(CEGUI::CoordConverter::asAbsolute(CEGUI::UDim(widgetPosition.d_y.d_scale, 0.f), baseSize.d_height));
    const auto offsetYInPixels = static_cast<qreal>(widgetPosition.d_y.d_offset);

    const auto scaleWidthInPixels = static_cast<qreal>(CEGUI::CoordConverter::asAbsolute(CEGUI::UDim(widgetSize.d_width.d_scale, 0.f), baseSize.d_width));
    const auto offsetWidthInPixels = static_cast<qreal>(widgetSize.d_width.d_offset);

    const auto scaleHeightInPixels = static_cast<qreal>(CEGUI::CoordConverter::asAbsolute(CEGUI::UDim(widgetSize.d_height.d_scale, 0.f), baseSize.d_height));
    const auto offsetHeightInPixels = static_cast<qreal>(widgetSize.d_height.d_offset);

    QPointF startXPoint(0.0, 0.0);
    QPointF startHPoint(0.0, 0.0);
    qreal hOffsetX = 1.0;
    switch (_widget->getHorizontalAlignment())
    {
        case CEGUI::HorizontalAlignment::Left:
        {
            startXPoint = (rect().topLeft() + rect().bottomLeft()) / 2.0;
            startHPoint = rect().topRight();
            break;
        }
        case CEGUI::HorizontalAlignment::Centre:
        {
            startXPoint = rect().center();
            startHPoint = rect().topRight();
            break;
        }
        case CEGUI::HorizontalAlignment::Right:
        {
            startXPoint = (rect().topRight() + rect().bottomRight()) / 2.0;
            startHPoint = rect().topLeft();
            hOffsetX = -1.0;
            break;
        }
    }

    QPointF startYPoint(0.0, 0.0);
    QPointF startWPoint(0.0, 0.0);
    qreal wOffsetY = 1.0;
    switch (_widget->getVerticalAlignment())
    {
        case CEGUI::VerticalAlignment::Top:
        {
            startYPoint = (rect().topLeft() + rect().topRight()) / 2.0;
            startWPoint = rect().bottomLeft();
            break;
        }
        case CEGUI::VerticalAlignment::Centre:
        {
            startYPoint = rect().center();
            startWPoint = rect().bottomLeft();
            break;
        }
        case CEGUI::VerticalAlignment::Bottom:
        {
            startYPoint = (rect().bottomLeft() + rect().bottomRight()) / 2.0;
            startWPoint = rect().topLeft();
            wOffsetY = -1.0;
            break;
        }
    }

    const auto midXPoint = startXPoint - QPointF(offsetXInPixels, 0.0);
    const auto endXPoint = midXPoint - QPointF(scaleXInPixels, 0.0);
    const auto xOffset = QPointF(0.0, (scaleXInPixels * offsetXInPixels < 0.0) ? 1.0 : 0.0);

    const auto midYPoint = startYPoint - QPointF(0.0, offsetYInPixels);
    const auto endYPoint = midYPoint - QPointF(0.0, scaleYInPixels);
    const auto yOffset = QPointF((scaleYInPixels * offsetYInPixels < 0) ? 1.0 : 0.0, 0.0);

    const auto midWPoint = startWPoint + QPointF(scaleWidthInPixels, 0.0);
    const auto endWPoint = midWPoint + QPointF(offsetWidthInPixels, 0.0);
    const auto wOffset = QPointF(0.0, (scaleWidthInPixels * offsetWidthInPixels < 0.0) ? wOffsetY : 0.0);

    const auto midHPoint = startHPoint + QPointF(0.0, scaleHeightInPixels);
    const auto endHPoint = midHPoint + QPointF(0.0, offsetHeightInPixels);
    const auto hOffset = QPointF((scaleHeightInPixels * offsetHeightInPixels < 0.0) ? hOffsetX : 0.0, 0.0);

    QPen pen(QColor(0, 255, 0, 255), 0); // Width 0 means 1 px size no matter the transformation
    painter->setPen(pen);
    painter->drawLine(startXPoint, midXPoint);
    painter->drawLine(startYPoint, midYPoint);
    painter->drawLine(midWPoint + wOffset, endWPoint + wOffset);
    painter->drawLine(midHPoint + hOffset, endHPoint + hOffset);
    pen.setColor(QColor(255, 0, 0, 255));
    painter->setPen(pen);
    painter->drawLine(startWPoint, midWPoint);
    painter->drawLine(startHPoint, midHPoint);
    painter->drawLine(midXPoint + xOffset, endXPoint + xOffset);
    painter->drawLine(midYPoint + yOffset, endYPoint + yOffset);
}

bool CEGUIManipulator::isLayoutContainer() const
{
    return !!dynamic_cast<CEGUI::LayoutContainer*>(_widget);
}

bool CEGUIManipulator::isInLayoutContainer() const
{
    return _widget->getParent() && dynamic_cast<CEGUI::LayoutContainer*>(_widget->getParent());
}

QSizeF CEGUIManipulator::getMinSize() const
{
    if (_widget)
    {
        auto size = CEGUI::CoordConverter::asAbsolute(_widget->getMinSize(), CEGUI::System::getSingleton().getRenderer()->getDisplaySize());
        return QSizeF(static_cast<qreal>(size.d_width), static_cast<qreal>(size.d_height));
    }

    return QSizeF();
}

QSizeF CEGUIManipulator::getMaxSize() const
{
    if (_widget)
    {
        auto size = CEGUI::CoordConverter::asAbsolute(_widget->getMaxSize(), CEGUI::System::getSingleton().getRenderer()->getDisplaySize());
        return QSizeF(static_cast<qreal>(size.d_width), static_cast<qreal>(size.d_height));
    }

    return QSizeF();
}

CEGUI::Sizef CEGUIManipulator::getBaseSize() const
{
    if (_widget && _widget->getParent() && !_widget->isNonClient())
        return _widget->getParent()->getUnclippedInnerRect().get().getSize();
    else
        return _widget->getParentPixelSize();
}

float CEGUIManipulator::getAnchorMinX() const
{
    return _widget->getPosition().d_x.d_scale;
}

float CEGUIManipulator::getAnchorMaxX() const
{
    return getAnchorMinX() + _widget->getSize().d_width.d_scale;
}

float CEGUIManipulator::getAnchorMinY() const
{
    return _widget->getPosition().d_y.d_scale;
}

float CEGUIManipulator::getAnchorMaxY() const
{
    return getAnchorMinY() + _widget->getSize().d_height.d_scale;
}

// Returns an effective parent rect in a scene coord system
QRectF CEGUIManipulator::getParentSceneRect() const
{
    if (!_widget || !scene()) return QRectF();

    if (_widget->getParent())
    {
        const CEGUI::Rectf& parentRect = _widget->getParent()->getUnclippedInnerRect().get();
        return QRectF(static_cast<qreal>(parentRect.left()),
                      static_cast<qreal>(parentRect.top()),
                      static_cast<qreal>(parentRect.getWidth()),
                      static_cast<qreal>(parentRect.getHeight()));
    }
    else
    {
        const CEGUI::Sizef& ctxSize = _widget->getGUIContext().getSurfaceSize();
        return QRectF(0.0, 0.0, static_cast<qreal>(ctxSize.d_width), static_cast<qreal>(ctxSize.d_height));
    }
}

// Calculates a rect composed of anchor limits (relative parts of the widget area) in a scene coord system
QRectF CEGUIManipulator::getAnchorsSceneRect() const
{
    QRectF parentRect = getParentSceneRect();

    // LC children have only sizing anchors. These anchors are visually based on the widget position
    // rather than on the parent position. It makes sizing more intuitive.
    if (isInLayoutContainer())
        parentRect.moveTo(scenePos());

    const auto& widgetPos = getWidget()->getPosition();
    const auto& widgetSize = getWidget()->getSize();
    return QRectF(parentRect.x() + parentRect.width() * static_cast<qreal>(widgetPos.d_x.d_scale),
                  parentRect.y() + parentRect.height() * static_cast<qreal>(widgetPos.d_y.d_scale),
                  parentRect.width() * static_cast<qreal>(widgetSize.d_width.d_scale),
                  parentRect.height() * static_cast<qreal>(widgetSize.d_height.d_scale));
}

QPointF CEGUIManipulator::scenePixelToAnchor(QPointF scenePixel) const
{
    QRectF parentRect = getParentSceneRect();

    // LC children have only sizing anchors. These anchors are visually based on the widget position
    // rather than on the parent position. It makes sizing more intuitive.
    if (isInLayoutContainer())
        parentRect.moveTo(scenePos());

    return QPointF(qFuzzyIsNull(parentRect.width()) ? 0.0 : (scenePixel.x() - parentRect.x()) / parentRect.width(),
                   qFuzzyIsNull(parentRect.height()) ? 0.0 : (scenePixel.y() - parentRect.y()) / parentRect.height());
}

void CEGUIManipulator::setAnchors(float minX, float maxX, float minY, float maxY, bool preserveEffectiveSize)
{
    const CEGUI::Sizef baseSize = getBaseSize();

    CEGUI::UVector2 deltaPos;
    CEGUI::USize deltaSize;

    // Min X
    {
        float deltaMinX = minX - getAnchorMinX();
        if (preserveEffectiveSize)
        {
            float pixelDelta = deltaMinX * baseSize.d_width;
            if (useIntegersForAbsoluteResize())
            {
                pixelDelta = std::round(pixelDelta);
                deltaMinX = qFuzzyIsNull(baseSize.d_width) ? 0.f : pixelDelta / baseSize.d_width;
            }
            deltaPos.d_x.d_offset -= pixelDelta;
            deltaSize.d_width.d_offset += pixelDelta;
        }
        deltaPos.d_x.d_scale += deltaMinX;
        deltaSize.d_width.d_scale -= deltaMinX;
    }

    // Min Y
    {
        float deltaMinY = minY - getAnchorMinY();
        if (preserveEffectiveSize)
        {
            float pixelDelta = deltaMinY * baseSize.d_height;
            if (useIntegersForAbsoluteResize())
            {
                pixelDelta = std::round(pixelDelta);
                deltaMinY = qFuzzyIsNull(baseSize.d_height) ? 0.f : pixelDelta / baseSize.d_height;
            }
            deltaPos.d_y.d_offset -= pixelDelta;
            deltaSize.d_height.d_offset += pixelDelta;
        }
        deltaPos.d_y.d_scale += deltaMinY;
        deltaSize.d_height.d_scale -= deltaMinY;
    }

    // Max X
    {
        float deltaMaxX = maxX - getAnchorMaxX();
        if (preserveEffectiveSize)
        {
            float pixelDelta = deltaMaxX * baseSize.d_width;
            if (useIntegersForAbsoluteResize())
            {
                pixelDelta = std::round(pixelDelta);
                deltaMaxX = qFuzzyIsNull(baseSize.d_width) ? 0.f : pixelDelta / baseSize.d_width;
            }
            deltaSize.d_width.d_offset -= pixelDelta;
        }
        deltaSize.d_width.d_scale += deltaMaxX;
    }

    // Max Y
    {
        float deltaMaxY = maxY - getAnchorMaxY();
        if (preserveEffectiveSize)
        {
            float pixelDelta = deltaMaxY * baseSize.d_height;
            if (useIntegersForAbsoluteResize())
            {
                pixelDelta = std::round(pixelDelta);
                deltaMaxY = qFuzzyIsNull(baseSize.d_height) ? 0.f : pixelDelta / baseSize.d_height;
            }
            deltaSize.d_height.d_offset -= pixelDelta;
        }
        deltaSize.d_height.d_scale += deltaMaxY;
    }

    adjustPositionDeltaOnResize(deltaPos, deltaSize);

    _widget->setPosition(_widget->getPosition() + deltaPos);
    _widget->setSize(_widget->getSize() + deltaSize);

    updateFromWidget();
    updatePropertiesFromWidget({"Size", "Position", "Area"});
}

void CEGUIManipulator::notifyHandleSelected(ResizingHandle* handle)
{
    ResizableRectItem::notifyHandleSelected(handle);
    moveToFront();
}

void CEGUIManipulator::notifyResizeStarted()
{
    ResizableRectItem::notifyResizeStarted();

    _resizeStarted = true;
    _prevPos = _widget->getPosition();
    _prevSize = _widget->getSize();

    for (QGraphicsItem* childItem : childItems())
    {
        if (dynamic_cast<CEGUIManipulator*>(childItem))
            childItem->setVisible(false);
    }

    // Hide siblings in the same layout container
    if (isInLayoutContainer())
        for (auto item : parentItem()->childItems())
            if (item != this && dynamic_cast<CEGUIManipulator*>(item))
                item->setVisible(false);
}

static inline void roundPointFloor(QPointF& point)
{
    point.setX(std::floor(point.x()));
    point.setY(std::floor(point.y()));
}

static inline void roundSizeFloor(QSizeF& size)
{
    size.setWidth(std::floor(size.width()));
    size.setHeight(std::floor(size.height()));
}

void CEGUIManipulator::notifyResizeProgress(QPointF newPos, QSizeF newSize)
{
    assert(_resizeStarted);

    ResizableRectItem::notifyResizeProgress(newPos, newSize);

    // Absolute pixel deltas
    auto pixelDeltaPos = newPos - _resizeStartPos;
    auto pixelDeltaSize = newSize - _resizeStartRect.size();

    CEGUI::UVector2 deltaPos;
    CEGUI::USize deltaSize;
    if (useAbsoluteCoordsForResize())
    {
        if (useIntegersForAbsoluteResize())
        {
            roundPointFloor(pixelDeltaPos);
            roundSizeFloor(pixelDeltaSize);
        }

        deltaPos = CEGUI::UVector2(
                    CEGUI::UDim(0.f, static_cast<float>(pixelDeltaPos.x())),
                    CEGUI::UDim(0.f, static_cast<float>(pixelDeltaPos.y())));
        deltaSize = CEGUI::USize(
                    CEGUI::UDim(0.f, static_cast<float>(pixelDeltaSize.width())),
                    CEGUI::UDim(0.f, static_cast<float>(pixelDeltaSize.height())));
    }
    else
    {
        auto baseSize = getBaseSize();
        deltaPos = CEGUI::UVector2(
                    CEGUI::UDim(static_cast<float>(pixelDeltaPos.x()) / baseSize.d_width, 0.f),
                    CEGUI::UDim(static_cast<float>(pixelDeltaPos.y()) / baseSize.d_height, 0.f));
        deltaSize = CEGUI::USize(
                    CEGUI::UDim(static_cast<float>(pixelDeltaSize.width()) / baseSize.d_width, 0.f),
                    CEGUI::UDim(static_cast<float>(pixelDeltaSize.height()) / baseSize.d_height, 0.f));
    }

    adjustPositionDeltaOnResize(deltaPos, deltaSize);

    _widget->setPosition(_prevPos + deltaPos);
    _widget->setSize(_prevSize + deltaSize);

    updatePropertiesFromWidget({"Size", "Position", "Area"});
}

void CEGUIManipulator::notifyResizeFinished(QPointF newPos, QSizeF newSize)
{
    ResizableRectItem::notifyResizeFinished(newPos, newSize);

    updateFromWidget();

    for (QGraphicsItem* childItem : childItems())
    {
        CEGUIManipulator* child = dynamic_cast<CEGUIManipulator*>(childItem);
        if (child)
        {
            child->updateFromWidget();
            child->setVisible(true);
        }
    }

    // Show siblings in the same layout container
    if (isInLayoutContainer())
        for (auto item : parentItem()->childItems())
            if (item != this && dynamic_cast<CEGUIManipulator*>(item))
                item->setVisible(true);

    if (parentItem())
        static_cast<CEGUIManipulator*>(parentItem())->updateFromWidget(true);
}

void CEGUIManipulator::notifyMoveStarted()
{
    ResizableRectItem::notifyMoveStarted();

    _moveStarted = true;
    _prevPos = _widget->getPosition();

    for (QGraphicsItem* childItem : childItems())
    {
        CEGUIManipulator* child = dynamic_cast<CEGUIManipulator*>(childItem);
        if (child) child->setVisible(false);
    }
}

void CEGUIManipulator::notifyMoveProgress(QPointF newPos)
{
    assert(_moveStarted);

    ResizableRectItem::notifyMoveProgress(newPos);

    // Absolute pixel deltas
    // NB: sometimes we move pos (self), sometimes rect (child handles), so we must take both into account
    auto pixelDeltaPos = newPos - _moveStartPos;

    CEGUI::UVector2 deltaPos;
    if (useAbsoluteCoordsForMove())
    {
        if (useIntegersForAbsoluteMove()) roundPointFloor(pixelDeltaPos);

        deltaPos = CEGUI::UVector2(
                    CEGUI::UDim(0.f, static_cast<float>(pixelDeltaPos.x())),
                    CEGUI::UDim(0.f, static_cast<float>(pixelDeltaPos.y())));
    }
    else
    {
        auto baseSize = getBaseSize();
        deltaPos = CEGUI::UVector2(
                    CEGUI::UDim(static_cast<float>(pixelDeltaPos.x()) / baseSize.d_width, 0.f),
                    CEGUI::UDim(static_cast<float>(pixelDeltaPos.y()) / baseSize.d_height, 0.f));
    }

    _widget->setPosition(_prevPos + deltaPos);

    updatePropertiesFromWidget({"Position", "Area"});
}

void CEGUIManipulator::notifyMoveFinished(QPointF newPos)
{
    ResizableRectItem::notifyMoveFinished(newPos);

    updateFromWidget();

    for (QGraphicsItem* childItem : childItems())
    {
        CEGUIManipulator* child = dynamic_cast<CEGUIManipulator*>(childItem);
        if (child)
        {
            child->updateFromWidget();
            child->setVisible(true);
        }
    }
}

// Updates this manipulator with associated widget properties. Mainly position and size.
// callUpdate - if True we also call update on the widget itself before
//              querying its properties
// updateParentLCs - if True we update ancestor layout containers
void CEGUIManipulator::updateFromWidget(bool callUpdate, bool updateAncestorLCs)
{
    assert(_widget);
    if (!_widget) return;

    if (callUpdate) _widget->update(0.f);

    // Just in case widget or widget name changed
    onWidgetNameChanged();

    if (updateAncestorLCs)
    {
        // We are trying to find a topmost LC (in case of nested LCs) and recursively update it
        QGraphicsItem* topmostLC = nullptr;
        auto item = parentItem();
        while (item && static_cast<CEGUIManipulator*>(item)->isLayoutContainer())
        {
            topmostLC = item;
            item = item->parentItem();
        }

        if (topmostLC)
        {
            static_cast<CEGUIManipulator*>(topmostLC)->updateFromWidget(true, false);

            // No need to continue, this method will get called again with updateAncestorLCs = false
            return;
        }
    }

    auto unclippedOuterRect = _widget->getUnclippedOuterRect().getFresh(true);
    auto pos = unclippedOuterRect.getPosition();
    auto size = unclippedOuterRect.getSize();
    auto parentWidget = _widget->getParent();
    if (parentWidget)
        pos -= parentWidget->getUnclippedOuterRect().get().getPosition();

    _ignoreGeometryChanges = true;
    setPos(QPointF(static_cast<qreal>(pos.x), static_cast<qreal>(pos.y)));
    setRect(QRectF(0.0, 0.0, static_cast<qreal>(size.d_width), static_cast<qreal>(size.d_height)));
    _ignoreGeometryChanges = false;

    // If we are updating top to bottom we don't need to update ancestor
    // layout containers, they will already be updated
    for (auto item : childItems())
        if (auto manip = dynamic_cast<CEGUIManipulator*>(item))
            manip->updateFromWidget(callUpdate, false);
}

// Detaches itself from the GUI hierarchy and the manipulator hierarchy.
// detachWidget - should we detach the CEGUI widget as well?
// destroyWidget - should we destroy the CEGUI widget after it's detached?
// recursive - recurse into children?
// This method doesn't destroy this instance immediately but it will be destroyed automatically
// when nothing is referencing it.
void CEGUIManipulator::detach(bool detachWidget, bool destroyWidget, bool recursive)
{
    // Descend if recursive
    if (recursive)
    {
        for (QGraphicsItem* childItem : childItems())
        {
            CEGUIManipulator* child = dynamic_cast<CEGUIManipulator*>(childItem);
            if (child)
            {
                child->detach(detachWidget, destroyWidget, true);
                delete child;
            }
        }
    }

    if (detachWidget)
    {
        // Detach from the GUI hierarchy
        auto parentWidget = _widget->getParent();
        if (parentWidget) parentWidget->removeChild(_widget);
    }

    // Detach from the parent manipulator
    scene()->removeItem(this);

    if (detachWidget && destroyWidget)
    {
        CEGUI::WindowManager::getSingleton().destroyWindow(_widget);
        _widget = nullptr;
        setToolTip("");
    }
}

QString CEGUIManipulator::getWidgetName() const
{
    return _widget ? CEGUIUtils::stringToQString(_widget->getName()) : "<Unknown>";
}

QString CEGUIManipulator::getWidgetType() const
{
    return _widget ? CEGUIUtils::stringToQString(_widget->getType()) : "<Unknown>";
}

QString CEGUIManipulator::getWidgetPath() const
{
    return _widget ? CEGUIUtils::stringToQString(_widget->getNamePath()) : "<Unknown>";
}

size_t CEGUIManipulator::getWidgetIndexInParent() const
{
    return (_widget && _widget->getParent()) ? _widget->getParent()->getChildIndex(_widget) : 0;
}

// Creates a child manipulator suitable for a child widget of manipulated widget
// This is there to allow overriding (if user subclasses the Manipulator, child manipulators are likely to be also subclassed)
CEGUIManipulator* CEGUIManipulator::createChildManipulator(CEGUI::Window* childWidget)
{
    return new CEGUIManipulator(this, childWidget);
}

void CEGUIManipulator::getChildManipulators(std::vector<CEGUIManipulator*>& outList, bool recursive)
{
    for (QGraphicsItem* item : childItems())
    {
        CEGUIManipulator* manipulator = dynamic_cast<CEGUIManipulator*>(item);
        if (manipulator)
        {
            outList.push_back(manipulator);
            if (recursive) manipulator->getChildManipulators(outList, true);
        }
    }
}

// Retrieves a manipulator relative to this manipulator by given widget path
CEGUIManipulator* CEGUIManipulator::getManipulatorByPath(const QString& widgetPath) const
{
    auto sepPos = widgetPath.indexOf('/');
    QString baseName;
    if (sepPos < 0)
    {
        baseName = widgetPath;
    }
    else if (dynamic_cast<CEGUI::TabControl*>(_widget) || dynamic_cast<CEGUI::ScrollablePane*>(_widget))
    {
        // These widgets store their children in auto container, but CEED treats them as
        // immediate children. Autocontainer therefore is skipped here.
        const auto startPos = sepPos + 1;
        sepPos = widgetPath.indexOf('/', startPos);
        baseName = widgetPath.mid(startPos, sepPos - startPos);
    }
    else
    {
        baseName = widgetPath.left(sepPos);
    }

    for (QGraphicsItem* item : childItems())
    {
        if (auto manipulator = dynamic_cast<CEGUIManipulator*>(item))
            if (manipulator->getWidgetName() == baseName)
                return (sepPos >= 0) ? manipulator->getManipulatorByPath(widgetPath.mid(sepPos + 1)) : manipulator;
    }

    return nullptr;
}

void CEGUIManipulator::forEachChildWidget(std::function<void (CEGUI::Window*)> callback) const
{
    if (auto tabControl = dynamic_cast<CEGUI::TabControl*>(_widget))
    {
        const size_t count = tabControl->getTabCount();
        for (size_t i = 0; i < count; ++i)
            callback(tabControl->getTabContentsAtIndex(i));
    }
    else if (auto scrollablePane = dynamic_cast<CEGUI::ScrollablePane*>(_widget))
    {
        const size_t count = scrollablePane->getContentPane()->getChildCount();
        for (size_t i = 0; i < count; ++i)
            callback(scrollablePane->getContentPane()->getChildAtIdx(i));
    }
    else
    {
        const size_t count = _widget->getChildCount();
        for (size_t i = 0; i < count; ++i)
            callback(_widget->getChildAtIdx(i));
    }
}

// Goes through child widgets of the manipulated widget and creates manipulator for each one.
// recursive - recurse into children?
// skipAutoWidgets - if true, auto widgets will be skipped over
// checkExisting - hint to skip search, useful for initial construction
void CEGUIManipulator::createChildManipulators(bool recursive, bool skipAutoWidgets, bool checkExisting)
{
    forEachChildWidget([this, skipAutoWidgets, recursive, checkExisting](CEGUI::Window* childWidget)
    {
        if (checkExisting && getManipulatorByPath(CEGUIUtils::stringToQString(childWidget->getName())))
            return;

        if (childWidget->isAutoWindow())
        {
            // Grid LC creates dummy placeholder auto-widgets. We don't want manipulators for them.
            if (skipAutoWidgets || dynamic_cast<CEGUI::GridLayoutContainer*>(_widget))
                return;
        }

        auto childManipulator = createChildManipulator(childWidget);
        childManipulator->updateFromWidget();
        if (recursive)
            childManipulator->createChildManipulators(true, skipAutoWidgets, checkExisting);
    });
}

void CEGUIManipulator::moveToFront()
{
    _widget->moveToFront();

    if (!parentItem()) return;

    for (auto&& item : parentItem()->childItems())
        if (item != this)
            item->stackBefore(this);

    static_cast<CEGUIManipulator*>(parentItem())->moveToFront();
}

// Notify the property manager that the values of the given properties have changed for this widget
void CEGUIManipulator::updatePropertiesFromWidget(const QStringList& propertyNames)
{
    for (const QString& propertyName : propertyNames)
    {
        auto it = _propertyMap.find(propertyName);
        if (it != _propertyMap.end())
        {
            const CEGUI::Property* ceguiProp = it->second.first;
            QtnProperty* prop = it->second.second;
            prop->fromStr(CEGUIUtils::stringToQString(ceguiProp->get(_widget)));

            if (propertyName == "Name")
                onWidgetNameChanged();
        }
    }
}

void CEGUIManipulator::updateAllPropertiesFromWidget()
{
    for (const auto& pair : _propertyMap)
    {
        const CEGUI::Property* ceguiProp = pair.second.first;
        QtnProperty* prop = pair.second.second;
        prop->fromStr(CEGUIUtils::stringToQString(ceguiProp->get(_widget)));
    }

    onWidgetNameChanged();
}

void CEGUIManipulator::createPropertySet()
{
    _propertyMap.clear();
    if (_propertySet) delete _propertySet;

    if (!_widget)
    {
        _propertySet = nullptr;
        return;
    }

    _propertySet = new QtnPropertySet(nullptr);

    std::map<QString, QtnPropertySet*> subsets;
    auto it = _widget->getPropertyIterator();
    while (!it.isAtEnd())
    {
        CEGUI::Property* ceguiProp = it.getCurrentValue();

        if (!ceguiProp->isReadable())
        {
            ++it;
            continue;
        }

        // Categorize properties by CEGUI property origin
        QtnPropertySet* parentSet = _propertySet;
        QString category = CEGUIUtils::stringToQString(ceguiProp->getOrigin());
        if (category.startsWith("CEGUI/")) category = category.mid(6);
        if (!category.isEmpty())
        {
            auto it = subsets.find(category);
            if (it == subsets.end())
            {
                // Insertion delayed for sorting, see below
                parentSet = new QtnPropertySet(_propertySet);
                parentSet->setName(category);
                subsets.emplace(std::move(category), parentSet);
            }
            else parentSet = it->second;
        }

        QtnProperty* prop = nullptr;
        const auto& propertyDataType = ceguiProp->getDataType(); // could be overridden through a property map
        if (propertyDataType == "bool")
            prop = new QtnPropertyBool(parentSet);
        else if (propertyDataType == "std::uint32_t")
        {
            auto uintProp = new QtnPropertyUInt(parentSet);
            uintProp->setDefaultValue(CEGUI::PropertyHelper<std::uint32_t>().fromString(ceguiProp->getDefault(_widget)));
            prop = uintProp;
        }
        else if (propertyDataType == "float")
        {
            auto floatProp = new QtnPropertyFloat(parentSet);
            floatProp->setDefaultValue(CEGUI::PropertyHelper<float>().fromString(ceguiProp->getDefault(_widget)));
            prop = floatProp;
        }
        else if (propertyDataType == "HorizontalAlignment")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumHorizontalAlignment());
            prop = enumProp;
        }
        else if (propertyDataType == "VerticalAlignment")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumVerticalAlignment());
            prop = enumProp;
        }
        else if (propertyDataType == "AspectMode")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumAspectMode());
            prop = enumProp;
        }
        else if (propertyDataType == "DefaultParagraphDirection")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumDefaultParagraphDirection());
            prop = enumProp;
        }
        else if (propertyDataType == "WindowUpdateMode")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumWindowUpdateMode());
            prop = enumProp;
        }
        else if (propertyDataType == "VerticalFormatting")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumVerticalFormatting());
            prop = enumProp;
        }
        else if (propertyDataType == "HorizontalFormatting")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumHorizontalFormatting());
            prop = enumProp;
        }
        else if (propertyDataType == "VerticalTextFormatting")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumVerticalTextFormatting());
            prop = enumProp;
        }
        else if (propertyDataType == "HorizontalTextFormatting")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumHorizontalTextFormatting());
            prop = enumProp;
        }
        else if (propertyDataType == "AutoPositioning")
        {
            auto enumProp = new QtnPropertyEnum(parentSet);
            enumProp->setEnumInfo(&CEGUIManager::Instance().enumAutoPositioning());
            prop = enumProp;
        }
        else if (propertyDataType == "Font")
        {
            prop = new QtnPropertyQString(parentSet);
            prop->setDelegateInfo({"Callback"});

            //???FIXME: Qtn - can use central string list without per-property copying?
            QtnGetCandidatesFn getCb = []() { return CEGUIManager::Instance().getAvailableFonts(); };
            prop->setDelegateAttribute("GetCandidatesFn", QVariant::fromValue(getCb));

            QtnCreateCandidateFn createCb = [](QWidget* /*parent*/, QString /*candidate*/) { return QString{}; };
            prop->setDelegateAttribute("CreateCandidateFn", QVariant::fromValue(createCb));
        }
        else if (propertyDataType == "Image")
        {
            prop = new QtnPropertyQString(parentSet);
            prop->setDelegateInfo({"Callback"});

            //???FIXME: Qtn - can use central string list without per-property copying?
            QtnGetCandidatesFn getCb = []() { return CEGUIManager::Instance().getAvailableImages(); };
            prop->setDelegateAttribute("GetCandidatesFn", QVariant::fromValue(getCb));

            QtnCreateCandidateFn createCb = [](QWidget* /*parent*/, QString /*candidate*/) { return QString{}; };
            prop->setDelegateAttribute("CreateCandidateFn", QVariant::fromValue(createCb));
        }
        else if (propertyDataType == "UVector2")
            prop = new QtnPropertyUVector2(parentSet);
        else if (propertyDataType == "UVector3")
            prop = new QtnPropertyUVector3(parentSet);
        else if (propertyDataType == "USize")
            prop = new QtnPropertyUSize(parentSet);
        else if (propertyDataType == "URect")
            prop = new QtnPropertyURect(parentSet);
        else if (propertyDataType == "UBox")
            prop = new QtnPropertyUBox(parentSet);
        else if (propertyDataType == "vec2")
        {
            // TODO: implement
            prop = new QtnPropertyQString(parentSet);
        }
        else if (propertyDataType == "Rectf")
        {
            // TODO: implement
            prop = new QtnPropertyQString(parentSet);
        }
        else if (propertyDataType == "quat")
        {
            // TODO: implement
            prop = new QtnPropertyQString(parentSet);
        }
        else if (propertyDataType == "ColourRect")
        {
            // TODO: implement
            //!!!4x Color subproperty!
            //!!!Preview rect. OpenGL triangles rendering? Or QPainter handles gradient?
            prop = new QtnPropertyQString(parentSet);
        }
        else if (propertyDataType == "NumOfTextLinesToShow")
        {
            // TODO: implement
            prop = new QtnPropertyQString(parentSet);
        }
        else // "String" and any other
        {
            if (propertyDataType != "String")
            {
                assert(false && "propertyDataType unknown");
            }
            prop = new QtnPropertyQString(parentSet);
        }

        /*
        "SortMode": ceguitypes.SortMode,
        "Colour": ceguitypes.Colour,
        */

        prop->setName(CEGUIUtils::stringToQString(ceguiProp->getName()));
        prop->setDescription(CEGUIUtils::stringToQString(ceguiProp->getHelp()));
        prop->fromStr(CEGUIUtils::stringToQString(ceguiProp->get(_widget)));
        prop->addState(QtnPropertyStateCollapsed);
        if (!ceguiProp->isWritable())
            prop->addState(QtnPropertyStateImmutable);

        parentSet->addChildProperty(prop, true);

        QObject::connect(prop, &QtnProperty::propertyDidChange, [this, prop, ceguiProp](QtnPropertyChangeReason reason)
        {
            if (reason & QtnPropertyChangeReasonEditValue)
                onPropertyChanged(prop, ceguiProp);
        });

        _propertyMap.emplace(prop->name(), std::pair<CEGUI::Property*, QtnProperty*>{ ceguiProp, prop });

        ++it;
    }

    // We want to see some categories at the beginning of the list
    const QStringList fixedOrderFirst = { "Element", "NamedElement", "Window" };
    for (const QString& name : fixedOrderFirst)
    {
        auto itSet = subsets.find(name);
        if (itSet != subsets.end())
        {
            _propertySet->addChildProperty(itSet->second, true);
            subsets.erase(itSet);
        }
    }

    // Unknown is always the last
    QtnPropertySet* unknownSet = nullptr;
    auto itSet = subsets.find("Unknown");
    if (itSet != subsets.end())
    {
        unknownSet = itSet->second;
        subsets.erase(itSet);
    }

    for (const auto& pair : subsets)
        _propertySet->addChildProperty(pair.second, true);

    if (unknownSet) _propertySet->addChildProperty(unknownSet, true);
}

void CEGUIManipulator::adjustPositionDeltaOnResize(CEGUI::UVector2& deltaPos, const CEGUI::USize& deltaSize)
{
    // Because the Qt manipulator is always top left aligned in the CEGUI sense,
    // we have to process the size to factor in alignments if they differ
    switch (_widget->getHorizontalAlignment())
    {
        case CEGUI::HorizontalAlignment::Centre:
            deltaPos.d_x += CEGUI::UDim(0.5f, 0.5f) * deltaSize.d_width; break;
        case CEGUI::HorizontalAlignment::Right:
            deltaPos.d_x += deltaSize.d_width; break;
        default: break;
    }
    switch (_widget->getVerticalAlignment())
    {
        case CEGUI::VerticalAlignment::Centre:
            deltaPos.d_y += CEGUI::UDim(0.5f, 0.5f) * deltaSize.d_height; break;
        case CEGUI::VerticalAlignment::Bottom:
            deltaPos.d_y += deltaSize.d_height; break;
        default: break;
    }
}

void CEGUIManipulator::onWidgetNameChanged()
{
    setToolTip(getWidgetName());
}

void CEGUIManipulator::onPropertyChanged(const QtnPropertyBase* property, CEGUI::Property* ceguiProperty)
{
    QString value;
    if (property->toStr(value))
    {
        ceguiProperty->set(_widget, CEGUIUtils::qStringToString(value));
        updateFromWidget(false, true);
        update();
    }
}

bool CEGUIManipulator::shouldBeSkipped() const
{
    if (!_widget->isAutoWindow()) return false;
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    const bool hideDeadEndAutoWidgets = settings->getEntryValue("layout/visual/hide_deadend_autowidgets").toBool();
    return hideDeadEndAutoWidgets && !hasNonAutoWidgetDescendants();
}

static bool impl_hasNonAutoWidgetDescendants(CEGUI::Window* widget)
{
    if (!widget->isAutoWindow()) return true;

    const size_t count = widget->getChildCount();
    for (size_t i = 0; i < count; ++i)
    {
        if (impl_hasNonAutoWidgetDescendants(widget->getChildAtIdx(i)))
            return true;
    }
    return false;
}

// Checks whether there are non-auto widgets nested in this widget.
// Self is a descendant of self in this context!
bool CEGUIManipulator::hasNonAutoWidgetDescendants() const
{
    return impl_hasNonAutoWidgetDescendants(_widget);
}

bool CEGUIManipulator::canAcceptChildren(size_t count, bool showErrorMessages) const
{
    if (!count) return true;

    // Grid layout accepts fixed number of children.
    // TODO: it is possible to add auto-extension in the CEGUI::GridLayoutContainer class.
    auto glc = dynamic_cast<CEGUI::GridLayoutContainer*>(_widget);
    if (glc)
    {
        const size_t capacity = glc->getGridWidth() * glc->getGridHeight();
        if (!capacity)
        {
            if (showErrorMessages)
                QMessageBox::warning(nullptr, "Can't accept a child", "Grid layout container must have non-zero dimensions to accept children");
            return false;
        }
        else
        {
            auto childCount = glc->getActualChildCount();
            if (capacity < childCount + count)
            {
                if (showErrorMessages)
                    QMessageBox::warning(nullptr, "Can't accept a child",
                                         QString("You must increase grid dimensions to insert %1 children.\n"
                                                 "There are %2 free cells.").arg(count).arg(capacity - childCount));
                return false;
            }
        }
    }

    return true;
}

QVariant CEGUIManipulator::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        if (value.toBool()) moveToFront();
    }

    return ResizableRectItem::itemChange(change, value);
}
