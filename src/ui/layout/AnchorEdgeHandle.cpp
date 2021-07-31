#include "src/ui/layout/AnchorEdgeHandle.h"
#include "src/ui/layout/LayoutScene.h"

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
