#include "src/ui/layout/LayoutContainerHandle.h"
#include "src/ui/layout/LayoutManipulator.h"
#include <CEGUI/widgets/GridLayoutContainer.h>
#include <CEGUI/widgets/HorizontalLayoutContainer.h>
#include <CEGUI/widgets/VerticalLayoutContainer.h>
#include <qgraphicssceneevent.h>
#include <qmimedata.h>

LayoutContainerHandle::LayoutContainerHandle(LayoutManipulator& host)
    : QGraphicsPixmapItem(&host)
{
    assert(host.isLayoutContainer());

    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setAcceptDrops(true);

    QString iconName;
    if (dynamic_cast<CEGUI::GridLayoutContainer*>(host.getWidget()))
    {
        iconName = ":/icons/widgets/GLC.png";
        setToolTip(host.getWidgetName() + ", grid layout");
    }
    else if (dynamic_cast<CEGUI::VerticalLayoutContainer*>(host.getWidget()))
    {
        iconName = ":/icons/widgets/VLC.png";
        setToolTip(host.getWidgetName() + ", vertical layout");
    }
    else if (dynamic_cast<CEGUI::HorizontalLayoutContainer*>(host.getWidget()))
    {
        iconName = ":/icons/widgets/HLC.png";
        setToolTip(host.getWidgetName() + ", horizontal layout");
    }
    else
    {
        assert(false && "Unknown CEGUI layout container subtype");
        iconName = ":/icons/widgets/HLC.png";
    }

    QPixmap pixmap(iconName);
    setPixmap(pixmap);
    setPos(-pixmap.size().width(), -pixmap.size().height());
    updateLook();
}

void LayoutContainerHandle::updateLook()
{
    setOpacity(_hovered || isSelected() ? 1.0 : 0.5);
}

void LayoutContainerHandle::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    _hovered = true;
    if (event->mimeData()->hasFormat("application/x-ceed-widget-type"))
    {
        updateLook();
        return static_cast<LayoutManipulator*>(parentItem())->dragEnterEvent(event);
    }
}

void LayoutContainerHandle::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    _hovered = false;
    updateLook();
    return static_cast<LayoutManipulator*>(parentItem())->dragLeaveEvent(event);
}

void LayoutContainerHandle::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    updateLook();
    return static_cast<LayoutManipulator*>(parentItem())->dropEvent(event);
}

QVariant LayoutContainerHandle::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        updateLook();
    }

    return QGraphicsPixmapItem::itemChange(change, value);
}

void LayoutContainerHandle::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsPixmapItem::hoverEnterEvent(event);
    _hovered = true;
    updateLook();
}

void LayoutContainerHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    _hovered = false;
    updateLook();
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
