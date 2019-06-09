#ifndef ANCHOREDGEHANDLE_H
#define ANCHOREDGEHANDLE_H

#include "src/ui/GuideLine.h"

class AnchorEdgeHandle : public GuideLine
{
public:

    AnchorEdgeHandle(bool horizontal, QGraphicsItem* parent = nullptr, int width = 1, Qt::PenStyle style = Qt::PenStyle::SolidLine, QColor normalColor = Qt::white, QColor hoverColor = Qt::white, size_t mouseInteractionDistance = 5);
    AnchorEdgeHandle(bool horizontal, QGraphicsItem* parent = nullptr, const QPen& pen = QPen(Qt::white), QColor hoverColor = Qt::white, size_t mouseInteractionDistance = 5);

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    virtual void onMoving(QPointF& newPos) override;
};

#endif // ANCHOREDGEHANDLE_H
