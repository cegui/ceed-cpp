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

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    QPen _normalPen;
    QColor _hoverColor;
};

#endif // ANCHORCORNERHANDLE_H
