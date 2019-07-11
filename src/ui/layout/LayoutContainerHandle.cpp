#include "src/ui/layout/LayoutContainerHandle.h"
#include "src/ui/layout/LayoutManipulator.h"

LayoutContainerHandle::LayoutContainerHandle(LayoutManipulator& host)
    : QGraphicsPixmapItem(&host)
{
    assert(host.isLayoutContainer());

    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    //setAcceptHoverEvents(true); //???transparency?

    QPixmap pixmap(":/icons/actions/help_about.png");
    setPixmap(pixmap);
    setPos(-pixmap.size().width(), -pixmap.size().height());
}
