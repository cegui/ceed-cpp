#include "src/ui/layout/LayoutContainerHandle.h"
#include "src/ui/layout/LayoutManipulator.h"
#include <CEGUI/widgets/GridLayoutContainer.h>
#include <CEGUI/widgets/HorizontalLayoutContainer.h>
#include <CEGUI/widgets/VerticalLayoutContainer.h>

LayoutContainerHandle::LayoutContainerHandle(LayoutManipulator& host)
    : QGraphicsPixmapItem(&host)
{
    assert(host.isLayoutContainer());

    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);

    QString iconName;
    if (dynamic_cast<CEGUI::GridLayoutContainer*>(host.getWidget()))
        iconName = ":/icons/widgets/GLC.png";
    else if (dynamic_cast<CEGUI::VerticalLayoutContainer*>(host.getWidget()))
        iconName = ":/icons/widgets/VLC.png";
    else if (dynamic_cast<CEGUI::HorizontalLayoutContainer*>(host.getWidget()))
        iconName = ":/icons/widgets/HLC.png";
    else
    {
        assert(false && "Unknown CEGUI layout container subtype");
        iconName = ":/icons/widgets/HLC.png";
    }

    QPixmap pixmap(iconName);
    setPixmap(pixmap);
    setPos(-pixmap.size().width(), -pixmap.size().height());
    setOpacity(0.5);
}

QVariant LayoutContainerHandle::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        const bool selected = value.toBool();
        setOpacity(selected ? 1.0 : 0.5);
    }

    return QGraphicsPixmapItem::itemChange(change, value);
}

void LayoutContainerHandle::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsPixmapItem::hoverEnterEvent(event);
    setOpacity(1.0);
}

void LayoutContainerHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if (!isSelected()) setOpacity(0.5);
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}
