#include "src/ui/layout/AnchorEdgeHandle.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/Application.h"

AnchorEdgeHandle::AnchorEdgeHandle(bool horizontal, QGraphicsItem* parent, int width, Qt::PenStyle style, QColor normalColor, QColor hoverColor, size_t mouseInteractionDistance)
    : GuideLine(horizontal, parent, width, style, normalColor, hoverColor, mouseInteractionDistance)
{
    // TODO: not working, intersection rect width depends on the scale
    setFlags(flags() | ItemIgnoresTransformations);
}

AnchorEdgeHandle::AnchorEdgeHandle(bool horizontal, QGraphicsItem* parent, const QPen& pen, QColor hoverColor, size_t mouseInteractionDistance)
    : GuideLine(horizontal, parent, pen, hoverColor, mouseInteractionDistance)
{
}

void AnchorEdgeHandle::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    GuideLine::hoverEnterEvent(event);

    auto layoutScene = static_cast<LayoutScene*>(scene());
    if (auto target = layoutScene->getAnchorTarget())
    {
        const bool ctrl = (QApplication::keyboardModifiers() & Qt::ControlModifier);
        QString helpMsg = "Drag to change anchors of <i>" + target->getWidgetPath(true) + "</i>, " +
                (ctrl ?
                     "release <b>Ctrl</b> to preserve current widget size." :
                     "hold <b>Ctrl</b> to resize the widget accordingly.");
        qobject_cast<Application*>(qApp)->getMainWindow()->setStatusMessage(helpMsg);
    }
    else
    {
        qobject_cast<Application*>(qApp)->getMainWindow()->setStatusMessage("");
    }
}

void AnchorEdgeHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    GuideLine::hoverLeaveEvent(event);

    qobject_cast<Application*>(qApp)->getMainWindow()->setStatusMessage("");
}

QVariant AnchorEdgeHandle::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedHasChanged)
    {
        if (value.toBool())
            static_cast<LayoutScene*>(scene())->anchorHandleSelected(this);
    }

    return GuideLine::itemChange(change, value);
}

void AnchorEdgeHandle::onMoving(QPointF& newPos)
{
    static_cast<LayoutScene*>(scene())->anchorHandleMoved(this, newPos, false);
}
