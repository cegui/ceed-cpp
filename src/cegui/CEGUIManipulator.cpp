#include "src/cegui/CEGUIManipulator.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include "qgraphicsscene.h"
#include "qpainter.h"
#include <CEGUI/widgets/TabControl.h>
#include <CEGUI/widgets/ScrollablePane.h>
#include <CEGUI/widgets/ScrolledContainer.h>
#include <CEGUI/widgets/LayoutContainer.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/CoordConverter.h>

// recursive - if true, even children of given widget are wrapped
// skipAutoWidgets - if true, auto widgets are skipped (only applicable if recursive is True)
CEGUIManipulator::CEGUIManipulator(QGraphicsItem* parent, CEGUI::Window* widget)
    : ResizableRectItem(parent)
    , _widget(widget)
{
    setFlags(ItemIsFocusable | ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
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
        return  _widget->getParent()->getUnclippedInnerRect().get().getSize();
    else
        return _widget->getParentPixelSize();
}

void CEGUIManipulator::notifyHandleSelected(ResizingHandle* handle)
{
    ResizableRectItem::notifyHandleSelected(handle);
    moveToFront();
}

void CEGUIManipulator::notifyResizeStarted(ResizingHandle* handle)
{
    ResizableRectItem::notifyResizeStarted(handle);
/*
    self.preResizePos = _widget->getPosition()
    self.preResizeSize = _widget->getSize()
*/

    for (QGraphicsItem* childItem : childItems())
    {
        if (dynamic_cast<CEGUIManipulator*>(childItem))
            childItem->setVisible(false);
    }

    // Hide siblings in the same layout container
    auto parent = _widget->getParent();
    if (parent && dynamic_cast<CEGUI::LayoutContainer*>(parent))
        for (auto item : parentItem()->childItems())
            if (item != this && dynamic_cast<CEGUIManipulator*>(item))
                item->setVisible(false);
}

void CEGUIManipulator::notifyResizeProgress(QPointF newPos, QRectF newRect)
{
    ResizableRectItem::notifyResizeProgress(newPos, newRect);

    // Absolute pixel deltas
/*
        auto pixelDeltaPos = newPos - _resizeOldPos;
        auto pixelDeltaSize = newRect.size() - _resizeOldRect.size()

        deltaPos = None
        deltaSize = None

        if self.useAbsoluteCoordsForResize():
            if self.useIntegersForAbsoluteResize():
                deltaPos = CEGUI::UVector2(CEGUI::UDim(0, math.floor(pixelDeltaPos.x())), CEGUI::UDim(0, math.floor(pixelDeltaPos.y())))
                deltaSize = CEGUI::USize(CEGUI::UDim(0, math.floor(pixelDeltaSize.width())), CEGUI::UDim(0, math.floor(pixelDeltaSize.height())))
            else:
                deltaPos = CEGUI::UVector2(CEGUI::UDim(0, pixelDeltaPos.x()), CEGUI::UDim(0, pixelDeltaPos.y()))
                deltaSize = CEGUI::USize(CEGUI::UDim(0, pixelDeltaSize.width()), CEGUI::UDim(0, pixelDeltaSize.height()))

        else:
            baseSize = self.getBaseSize()

            deltaPos = CEGUI::UVector2(CEGUI::UDim(pixelDeltaPos.x() / baseSize.d_width, 0), CEGUI::UDim(pixelDeltaPos.y() / baseSize.d_height, 0))
            deltaSize = CEGUI::USize(CEGUI::UDim(pixelDeltaSize.width() / baseSize.d_width, 0), CEGUI::UDim(pixelDeltaSize.height() / baseSize.d_height, 0))

        # because the Qt manipulator is always top left aligned in the CEGUI sense,
        # we have to process the size to factor in alignments if they differ
        processedDeltaPos = CEGUI::UVector2()

        hAlignment = _widget->getHorizontalAlignment()
        if hAlignment == CEGUI::HorizontalAlignment.HA_LEFT:
            processedDeltaPos.d_x = deltaPos.d_x
        elif hAlignment == CEGUI::HorizontalAlignment.HA_CENTRE:
            processedDeltaPos.d_x = deltaPos.d_x + CEGUI::UDim(0.5, 0.5) * deltaSize.d_width
        elif hAlignment == CEGUI::HorizontalAlignment.HA_RIGHT:
            processedDeltaPos.d_x = deltaPos.d_x + deltaSize.d_width
        else:
            assert(False)

        vAlignment = _widget->getVerticalAlignment()
        if vAlignment == CEGUI::VerticalAlignment.VA_TOP:
            processedDeltaPos.d_y = deltaPos.d_y
        elif vAlignment == CEGUI::VerticalAlignment.VA_CENTRE:
            processedDeltaPos.d_y = deltaPos.d_y + CEGUI::UDim(0.5, 0.5) * deltaSize.d_height
        elif vAlignment == CEGUI::VerticalAlignment.VA_BOTTOM:
            processedDeltaPos.d_y = deltaPos.d_y + deltaSize.d_height
        else:
            assert(False)

        _widget->setPosition(self.preResizePos + processedDeltaPos)
        _widget->setSize(self.preResizeSize + deltaSize)

        self.lastResizeNewPos = newPos
        self.lastResizeNewRect = newRect
*/
}

void CEGUIManipulator::notifyResizeFinished(QPointF newPos, QRectF newRect)
{
    ResizableRectItem::notifyResizeFinished(newPos, newRect);

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
    auto parent = _widget->getParent();
    if (parent && dynamic_cast<CEGUI::LayoutContainer*>(parent))
        for (auto item : parentItem()->childItems())
            if (item != this && dynamic_cast<CEGUIManipulator*>(item))
                item->setVisible(true);

    if (parentItem())
        static_cast<CEGUIManipulator*>(parentItem())->updateFromWidget(true);

/*
        self.lastResizeNewPos = None
        self.lastResizeNewRect = None
*/
}

void CEGUIManipulator::notifyMoveStarted()
{
    ResizableRectItem::notifyMoveStarted();

    _preMovePos = _widget->getPosition();

    for (QGraphicsItem* childItem : childItems())
    {
        CEGUIManipulator* child = dynamic_cast<CEGUIManipulator*>(childItem);
        if (child) child->setVisible(false);
    }
}

void CEGUIManipulator::notifyMoveProgress(QPointF newPos)
{
    ResizableRectItem::notifyMoveProgress(newPos);

    // Absolute pixel deltas
    auto pixelDeltaPos = newPos - moveOldPos;

    CEGUI::UVector2 deltaPos;
    if (useAbsoluteCoordsForMove())
    {
        if (useIntegersForAbsoluteMove())
            pixelDeltaPos = QPointF(std::floor(pixelDeltaPos.x()), std::floor(pixelDeltaPos.y()));

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

    _widget->setPosition(_preMovePos + deltaPos);

    _lastMoveNewPos = newPos;
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
/*
        self.lastMoveNewPos = None
*/
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

    if (updateAncestorLCs)
    {
        // We are trying to find a topmost LC (in case of nested LCs) and recursively update it
        QGraphicsItem* topmostLC = nullptr;
        auto item = parentItem();
        while (item && dynamic_cast<CEGUI::LayoutContainer*>(static_cast<CEGUIManipulator*>(item)->_widget))
        {
            topmostLC = item;
            item = item->parentItem();
        }

        if (topmostLC)
        {
            static_cast<CEGUIManipulator*>(topmostLC)->updateFromWidget(true, false);

            // No need to continue, this method will get called again with updateAncestorLCs = False
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
            if (child) child->detach(detachWidget, destroyWidget, true);
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

// Creates a child manipulator suitable for a child widget of manipulated widget
// This is there to allow overriding (if user subclasses the Manipulator, child manipulators are likely to be also subclassed)
CEGUIManipulator* CEGUIManipulator::createChildManipulator(CEGUI::Window* childWidget, bool recursive, bool skipAutoWidgets)
{
    auto ret = new CEGUIManipulator(this, childWidget);
    ret->createChildManipulators(recursive, skipAutoWidgets, false);
    ret->updateFromWidget();
    return ret;
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
    if (dynamic_cast<CEGUI::TabControl*>(_widget) || dynamic_cast<CEGUI::ScrollablePane*>(_widget))
    {
        auto manipulator = getManipulatorFromChildContainerByPath(widgetPath);
        if (manipulator) return manipulator;
    }

    const auto sepPos = widgetPath.indexOf('/');
    QString baseName = (sepPos >= 0) ? widgetPath.left(sepPos) : widgetPath;
    QString remainder = (sepPos >= 0) ? widgetPath.mid(sepPos + 1) : "";
    for (QGraphicsItem* item : childItems())
    {
        if (auto manipulator = dynamic_cast<CEGUIManipulator*>(item))
            if (manipulator->getWidgetName() == baseName)
                return (sepPos < 0) ? manipulator : manipulator->getManipulatorByPath(widgetPath.mid(sepPos + 1));
    }

    return nullptr;
}

// Retrieves a manipulator relative to this manipulator by given widget path for widgets that use
// autoWindow containers, such as ScrollablePanes and TabControl. The children in these case should
// be treated as if they were attached to the window directly, whereas in reality they use a container
// widget, which forces us to handle these cases using this function.
CEGUIManipulator* CEGUIManipulator::getManipulatorFromChildContainerByPath(const QString& widgetPath) const
{
    const auto sepPos = widgetPath.indexOf('/');
    QString directChildPath = (sepPos >= 0) ? widgetPath.mid(sepPos + 1) : "";
    for (QGraphicsItem* item : childItems())
    {
        CEGUIManipulator* manipulator = dynamic_cast<CEGUIManipulator*>(item);
        if (manipulator && manipulator->getWidgetName() == directChildPath)
            return manipulator;
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

        if (!skipAutoWidgets || !childWidget->isAutoWindow())
        {
            // NB: we don't have to assign or attach the child manipulator here
            //     just passing parent to the constructor is enough
            auto childManipulator = createChildManipulator(childWidget, recursive, skipAutoWidgets);
            if (recursive)
                childManipulator->createChildManipulators(true, skipAutoWidgets, checkExisting);
        }
    });
}

void CEGUIManipulator::moveToFront()
{
    _widget->moveToFront();

    if (!parentItem()) return;

    for (auto&& item : parentItem()->childItems())
    {
        if (item == this) continue;

        // For some reason this is the opposite of what (IMO) it should be
        // which is self.stackBefore(item). Is Qt documentation flawed or something?!
        item->stackBefore(this);
    }

    static_cast<CEGUIManipulator*>(parentItem())->moveToFront();
}

// Notify the property manager that the values of the given properties have changed for this widget
void CEGUIManipulator::triggerPropertyManagerCallback(QStringList propertyNames)
{
/*
        widget = self.widget

        # if the property manager has set callbacks on this widget
        if hasattr(widget, "propertyManagerCallbacks"):
            for propertyName in propertyNames:
                # if there's a callback for this property
                if propertyName in widget.propertyManagerCallbacks:
                    # call it
                    widget.propertyManagerCallbacks[propertyName]()
*/
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

QVariant CEGUIManipulator::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        if (value.toBool()) moveToFront();
    }

    return ResizableRectItem::itemChange(change, value);
}
