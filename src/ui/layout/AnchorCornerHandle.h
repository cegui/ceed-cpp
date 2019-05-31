#ifndef ANCHORCORNERHANDLE_H
#define ANCHORCORNERHANDLE_H

#include <qgraphicsitem.h>
#include <qpen.h>

class AnchorCornerHandle : public QGraphicsPolygonItem
{
public:

    AnchorCornerHandle(bool left, bool top, QGraphicsItem* parent = nullptr, qreal size = 12.0, const QPen& pen = QPen(Qt::white), QColor hoverColor = Qt::white);

    void setPosSilent(const QPointF& newPos);
    void setPosSilent(qreal x, qreal y);

protected:

    void updatePen(bool hovered);
    void updateBrush();

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

    QPen _normalPen;
    QColor _hoverColor;
    qreal _moveOppositeThreshold;
    bool _moveOpposite = false; // When overlap with opposite items, move them or stop moving self?
};

#endif // ANCHORCORNERHANDLE_H
