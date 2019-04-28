#include "src/cegui/CEGUIManipulator.h"
#include "src/cegui/CEGUIProjectManager.h"
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
CEGUIManipulator::CEGUIManipulator(QGraphicsItem* parent, CEGUI::Window* widget, bool recursive, bool skipAutoWidgets)
    : ResizableRectItem(parent)
    , _widget(widget)
{
    setFlags(ItemIsFocusable | ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);

    if (recursive)
    {
        forEachChildWidget([this, skipAutoWidgets](CEGUI::Window* childWidget)
        {
            // NB: we don't have to assign or attach the child manipulator here
            // just passing parent to the constructor is enough
            if (!skipAutoWidgets || !childWidget->isAutoWindow())
                createChildManipulator(childWidget, true, skipAutoWidgets);
        });
    }
/*
        self.preResizePos = None
        self.preResizeSize = None
        self.lastResizeNewPos = None
        self.lastResizeNewRect = None

        self.preMovePos = None
        self.lastMoveNewPos = None
*/
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

    const bool drawGuides = (isSelected() || _resizeInProgress || isAnyHandleSelected());
    if (!drawGuides) return;

/*
        baseSize = self.getBaseSize()
        self.paintHorizontalGuides(baseSize, painter, option, widget)
        self.paintVerticalGuides(baseSize, painter, option, widget)


    def paintHorizontalGuides(self, baseSize, painter, option, widget):
        """Paints horizontal dimension guides - position X and width guides"""

        widgetPosition = self.widget.getPosition()
        widgetSize = self.widget.getSize()

        # x coordinate
        scaleXInPixels = PyCEGUI.CoordConverter.asAbsolute(PyCEGUI.UDim(widgetPosition.d_x.d_scale, 0), baseSize.d_width)
        offsetXInPixels = widgetPosition.d_x.d_offset

        # width
        scaleWidthInPixels = PyCEGUI.CoordConverter.asAbsolute(PyCEGUI.UDim(widgetSize.d_width.d_scale, 0), baseSize.d_width)
        offsetWidthInPixels = widgetSize.d_width.d_offset

        hAlignment = self.widget.getHorizontalAlignment()
        startXPoint = 0
        if hAlignment == PyCEGUI.HorizontalAlignment.HA_LEFT:
            startXPoint = (self.rect().topLeft() + self.rect().bottomLeft()) / 2
        elif hAlignment == PyCEGUI.HorizontalAlignment.HA_CENTRE:
            startXPoint = self.rect().center()
        elif hAlignment == PyCEGUI.HorizontalAlignment.HA_RIGHT:
            startXPoint = (self.rect().topRight() + self.rect().bottomRight()) / 2
        else:
            assert(False)

        midXPoint = startXPoint - QtCore.QPointF(offsetXInPixels, 0)
        endXPoint = midXPoint - QtCore.QPointF(scaleXInPixels, 0)
        xOffset = QtCore.QPointF(0, 1) if scaleXInPixels * offsetXInPixels < 0 else QtCore.QPointF(0, 0)

        pen = QtGui.QPen()
        # 0 means 1px size no matter the transformation
        pen.setWidth(0)
        pen.setColor(QtGui.QColor(0, 255, 0, 255))
        painter.setPen(pen)
        painter.drawLine(startXPoint, midXPoint)
        pen.setColor(QtGui.QColor(255, 0, 0, 255))
        painter.setPen(pen)
        painter.drawLine(midXPoint + xOffset, endXPoint + xOffset)

        vAlignment = self.widget.getVerticalAlignment()
        startWPoint = 0
        if vAlignment == PyCEGUI.VerticalAlignment.VA_TOP:
            startWPoint = self.rect().bottomLeft()
        elif vAlignment == PyCEGUI.VerticalAlignment.VA_CENTRE:
            startWPoint = self.rect().bottomLeft()
        elif vAlignment == PyCEGUI.VerticalAlignment.VA_BOTTOM:
            startWPoint = self.rect().topLeft()
        else:
            assert(False)

        midWPoint = startWPoint + QtCore.QPointF(scaleWidthInPixels, 0)
        endWPoint = midWPoint + QtCore.QPointF(offsetWidthInPixels, 0)
        # FIXME: epicly unreadable
        wOffset = QtCore.QPointF(0, -1 if vAlignment == PyCEGUI.VerticalAlignment.VA_BOTTOM else 1) if scaleWidthInPixels * offsetWidthInPixels < 0 else QtCore.QPointF(0, 0)

        pen = QtGui.QPen()
        # 0 means 1px size no matter the transformation
        pen.setWidth(0)
        pen.setColor(QtGui.QColor(255, 0, 0, 255))
        painter.setPen(pen)
        painter.drawLine(startWPoint, midWPoint)
        pen.setColor(QtGui.QColor(0, 255, 0, 255))
        painter.setPen(pen)
        painter.drawLine(midWPoint + wOffset, endWPoint + wOffset)

    def paintVerticalGuides(self, baseSize, painter, option, widget):
        """Paints vertical dimension guides - position Y and height guides"""

        widgetPosition = self.widget.getPosition()
        widgetSize = self.widget.getSize()

        # y coordinate
        scaleYInPixels = PyCEGUI.CoordConverter.asAbsolute(PyCEGUI.UDim(widgetPosition.d_y.d_scale, 0), baseSize.d_height)
        offsetYInPixels = widgetPosition.d_y.d_offset

        # height
        scaleHeightInPixels = PyCEGUI.CoordConverter.asAbsolute(PyCEGUI.UDim(widgetSize.d_height.d_scale, 0), baseSize.d_height)
        offsetHeightInPixels = widgetSize.d_height.d_offset

        vAlignment = self.widget.getVerticalAlignment()
        startYPoint = 0
        if vAlignment == PyCEGUI.VerticalAlignment.VA_TOP:
            startYPoint = (self.rect().topLeft() + self.rect().topRight()) / 2
        elif vAlignment == PyCEGUI.VerticalAlignment.VA_CENTRE:
            startYPoint = self.rect().center()
        elif vAlignment == PyCEGUI.VerticalAlignment.VA_BOTTOM:
            startYPoint = (self.rect().bottomLeft() + self.rect().bottomRight()) / 2
        else:
            assert(False)

        midYPoint = startYPoint - QtCore.QPointF(0, offsetYInPixels)
        endYPoint = midYPoint - QtCore.QPointF(0, scaleYInPixels)
        yOffset = QtCore.QPointF(1, 0) if scaleYInPixels * offsetYInPixels < 0 else QtCore.QPointF(0, 0)

        pen = QtGui.QPen()
        # 0 means 1px size no matter the transformation
        pen.setWidth(0)
        pen.setColor(QtGui.QColor(0, 255, 0, 255))
        painter.setPen(pen)
        painter.drawLine(startYPoint, midYPoint)
        pen.setColor(QtGui.QColor(255, 0, 0, 255))
        painter.setPen(pen)
        painter.drawLine(midYPoint + yOffset, endYPoint + yOffset)

        hAlignment = self.widget.getHorizontalAlignment()
        startHPoint = 0
        if hAlignment == PyCEGUI.HorizontalAlignment.HA_LEFT:
            startHPoint = self.rect().topRight()
        elif hAlignment == PyCEGUI.HorizontalAlignment.HA_CENTRE:
            startHPoint = self.rect().topRight()
        elif hAlignment == PyCEGUI.HorizontalAlignment.HA_RIGHT:
            startHPoint = self.rect().topLeft()
        else:
            assert(False)

        midHPoint = startHPoint + QtCore.QPointF(0, scaleHeightInPixels)
        endHPoint = midHPoint + QtCore.QPointF(0, offsetHeightInPixels)
        # FIXME: epicly unreadable
        hOffset = QtCore.QPointF(-1 if hAlignment == PyCEGUI.HorizontalAlignment.HA_RIGHT else 1, 0) if scaleHeightInPixels * offsetHeightInPixels < 0 else QtCore.QPointF(0, 0)

        pen = QtGui.QPen()
        # 0 means 1px size no matter the transformation
        pen.setWidth(0)
        pen.setColor(QtGui.QColor(255, 0, 0, 255))
        painter.setPen(pen)
        painter.drawLine(startHPoint, midHPoint)
        pen.setColor(QtGui.QColor(0, 255, 0, 255))
        painter.setPen(pen)
        painter.drawLine(midHPoint + hOffset, endHPoint + hOffset)
*/
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

void CEGUIManipulator::notifyHandleSelected(ResizingHandle* handle)
{
    ResizableRectItem::notifyHandleSelected(handle);
    moveToFront();
}

void CEGUIManipulator::notifyResizeStarted(ResizingHandle* handle)
{
    ResizableRectItem::notifyResizeStarted(handle);
/*
    self.preResizePos = self.widget.getPosition()
    self.preResizeSize = self.widget.getSize()
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
                deltaPos = PyCEGUI.UVector2(PyCEGUI.UDim(0, math.floor(pixelDeltaPos.x())), PyCEGUI.UDim(0, math.floor(pixelDeltaPos.y())))
                deltaSize = PyCEGUI.USize(PyCEGUI.UDim(0, math.floor(pixelDeltaSize.width())), PyCEGUI.UDim(0, math.floor(pixelDeltaSize.height())))
            else:
                deltaPos = PyCEGUI.UVector2(PyCEGUI.UDim(0, pixelDeltaPos.x()), PyCEGUI.UDim(0, pixelDeltaPos.y()))
                deltaSize = PyCEGUI.USize(PyCEGUI.UDim(0, pixelDeltaSize.width()), PyCEGUI.UDim(0, pixelDeltaSize.height()))

        else:
            baseSize = self.getBaseSize()

            deltaPos = PyCEGUI.UVector2(PyCEGUI.UDim(pixelDeltaPos.x() / baseSize.d_width, 0), PyCEGUI.UDim(pixelDeltaPos.y() / baseSize.d_height, 0))
            deltaSize = PyCEGUI.USize(PyCEGUI.UDim(pixelDeltaSize.width() / baseSize.d_width, 0), PyCEGUI.UDim(pixelDeltaSize.height() / baseSize.d_height, 0))

        # because the Qt manipulator is always top left aligned in the CEGUI sense,
        # we have to process the size to factor in alignments if they differ
        processedDeltaPos = PyCEGUI.UVector2()

        hAlignment = self.widget.getHorizontalAlignment()
        if hAlignment == PyCEGUI.HorizontalAlignment.HA_LEFT:
            processedDeltaPos.d_x = deltaPos.d_x
        elif hAlignment == PyCEGUI.HorizontalAlignment.HA_CENTRE:
            processedDeltaPos.d_x = deltaPos.d_x + PyCEGUI.UDim(0.5, 0.5) * deltaSize.d_width
        elif hAlignment == PyCEGUI.HorizontalAlignment.HA_RIGHT:
            processedDeltaPos.d_x = deltaPos.d_x + deltaSize.d_width
        else:
            assert(False)

        vAlignment = self.widget.getVerticalAlignment()
        if vAlignment == PyCEGUI.VerticalAlignment.VA_TOP:
            processedDeltaPos.d_y = deltaPos.d_y
        elif vAlignment == PyCEGUI.VerticalAlignment.VA_CENTRE:
            processedDeltaPos.d_y = deltaPos.d_y + PyCEGUI.UDim(0.5, 0.5) * deltaSize.d_height
        elif vAlignment == PyCEGUI.VerticalAlignment.VA_BOTTOM:
            processedDeltaPos.d_y = deltaPos.d_y + deltaSize.d_height
        else:
            assert(False)

        self.widget.setPosition(self.preResizePos + processedDeltaPos)
        self.widget.setSize(self.preResizeSize + deltaSize)

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
/*
        parent = self.widget.getParent()
        if parent and isinstance(parent, PyCEGUI.LayoutContainer):
            # show siblings in the same layout container
            for item in self.parentItem().childItems():
                if item is not self and isinstance(item, Manipulator):
                    item.setVisible(True)

            self.parentItem().updateFromWidget(True)

        self.lastResizeNewPos = None
        self.lastResizeNewRect = None
*/
}

void CEGUIManipulator::notifyMoveStarted()
{
    ResizableRectItem::notifyMoveStarted();
/*
        self.preMovePos = self.widget.getPosition()
*/

    for (QGraphicsItem* childItem : childItems())
    {
        CEGUIManipulator* child = dynamic_cast<CEGUIManipulator*>(childItem);
        if (child) child->setVisible(false);
    }
}

void CEGUIManipulator::notifyMoveProgress(QPointF newPos)
{
    ResizableRectItem::notifyMoveProgress(newPos);
/*
        # absolute pixel deltas
        pixelDeltaPos = newPos - self.moveOldPos

        deltaPos = None
        if self.useAbsoluteCoordsForMove():
            if self.useIntegersForAbsoluteMove():
                deltaPos = PyCEGUI.UVector2(PyCEGUI.UDim(0, math.floor(pixelDeltaPos.x())), PyCEGUI.UDim(0, math.floor(pixelDeltaPos.y())))
            else:
                deltaPos = PyCEGUI.UVector2(PyCEGUI.UDim(0, pixelDeltaPos.x()), PyCEGUI.UDim(0, pixelDeltaPos.y()))

        else:
            baseSize = self.getBaseSize()

            deltaPos = PyCEGUI.UVector2(PyCEGUI.UDim(pixelDeltaPos.x() / baseSize.d_width, 0), PyCEGUI.UDim(pixelDeltaPos.y() / baseSize.d_height, 0))

        self.widget.setPosition(self.preMovePos + deltaPos)

        self.lastMoveNewPos = newPos
*/
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
    return _widget ? CEGUIProjectManager::ceguiStringToQString(_widget->getName()) : "<Unknown>";
}

QString CEGUIManipulator::getWidgetType() const
{
    return _widget ? CEGUIProjectManager::ceguiStringToQString(_widget->getType()) : "<Unknown>";
}

QString CEGUIManipulator::getWidgetPath() const
{
    return _widget ? CEGUIProjectManager::ceguiStringToQString(_widget->getNamePath()) : "<Unknown>";
}

// Creates a child manipulator suitable for a child widget of manipulated widget
// This is there to allow overriding (if user subclasses the Manipulator, child manipulators are likely to be also subclassed)
CEGUIManipulator* CEGUIManipulator::createChildManipulator(CEGUI::Window* childWidget, bool recursive, bool skipAutoWidgets)
{
    auto ret = new CEGUIManipulator(this, childWidget, recursive, skipAutoWidgets);
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
/*
        // Throws LookupError on failure.
        if isinstance(self.widget, PyCEGUI.TabControl) or isinstance(self.widget, PyCEGUI.ScrollablePane):
            manipulator = self.getManipulatorFromChildContainerByPath(widgetPath)
            if manipulator is not None:
                return manipulator

                //QString::section
        path = widgetPath.split("/", 1)
        assert(len(path) >= 1)

        baseName = path[0]
        remainder = ""
        if len(path) == 2:
            remainder = path[1]

        for item in self.childItems():
            if isinstance(item, Manipulator):
                if item.widget.getName() == baseName:
                    if remainder == "":
                        return item

                    else:
                        return item.getManipulatorByPath(remainder)

        raise LookupError("Can't find widget manipulator of path '" + widgetPath + "'")
*/
    return nullptr;
}

// Retrieves a manipulator relative to this manipulator by given widget path for widgets that use
// autoWindow containers, such as ScrollablePanes and TabControl. The children in these case should
// be treated as if they were attached to the window directly, whereas in reality they use a container
// widget, which forces us to handle these cases using this function.
CEGUIManipulator* CEGUIManipulator::getManipulatorFromChildContainerByPath(const QString& widgetPath) const
{
    auto sepPos = widgetPath.indexOf('/');
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
        const size_t count = tabControl->getChildCount();
        for (size_t i = 0; i < count; ++i)
            callback(tabControl->getChildAtIdx(i));
    }
}

// Goes through child widgets of the manipulated widget and creates manipulator for each missing one.
// recursive - recurse into children?
// skipAutoWidgets - if true, auto widgets will be skipped over
void CEGUIManipulator::createMissingChildManipulators(bool recursive, bool skipAutoWidgets)
{
    forEachChildWidget([this, skipAutoWidgets, recursive](CEGUI::Window* childWidget)
    {
        if (getManipulatorByPath(CEGUIProjectManager::ceguiStringToQString(childWidget->getName())))
            return;

        if (!skipAutoWidgets || !childWidget->isAutoWindow())
        {
            // NB: we don't have to assign or attach the child manipulator here
            //     just passing parent to the constructor is enough
            auto childManipulator = createChildManipulator(childWidget, recursive, skipAutoWidgets);
            if (recursive)
                childManipulator->createMissingChildManipulators(true, skipAutoWidgets);
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

/*

    def getBaseSize(self):
        if self.widget.getParent() is not None and not self.widget.isNonClient():
            return self.widget.getParent().getUnclippedInnerRect().get().getSize()

        else:
            return self.widget.getParentPixelSize()

*/
