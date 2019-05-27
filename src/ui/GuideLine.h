#ifndef GUIDELINE_H
#define GUIDELINE_H

#include <qgraphicsitem.h>
#include <qpen.h>

// Cosmetic line that crosses the whole scene or parent rect vertically or horizontally.
// May be dragged. Use it as a guide or as x/y editing handle.

class GuideLine : public QGraphicsLineItem
{
public:

    GuideLine(bool horizontal, QGraphicsItem* parent = nullptr, int width = 1, Qt::PenStyle style = Qt::PenStyle::SolidLine, QColor normalColor = Qt::white, QColor hoverColor = Qt::white, size_t mouseInteractionDistance = 5);
    GuideLine(bool horizontal, QGraphicsItem* parent = nullptr, const QPen& pen = QPen(Qt::white), QColor hoverColor = Qt::white, size_t mouseInteractionDistance = 5);

    virtual QPainterPath shape() const override;

    void setPosSilent(const QPointF& newPos);
    void setPosSilent(qreal x, qreal y);

protected:

    void updateLine();
    void updatePen(bool hovered);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    QPen _normalPen;
    QColor _hoverColor;

    size_t _mouseInteractionDistance = 5; // Distance from line in pixels where the mouse interaction starts
    bool _horizontal = false;
};

#endif // GUIDELINE_H
