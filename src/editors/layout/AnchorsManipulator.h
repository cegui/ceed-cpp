#ifndef ANCHORSMANIPULATOR_H
#define ANCHORSMANIPULATOR_H

// TODO: NOT a resizable rect item! It must be handles only - 2 horz + 2 vert guides + 4 corner draggable items
// Guide is a cosmetic pen line screen-wide (or high) with mouse reaction rect wider than the line itself.
// Triangular corner handles must react on mouse only inside the polygon.
// This class could be a QGraphicsItemGroup containing 4 QGraphicsLineItem + 4 QGraphicsPolygonItem +
// on-demand QGraphicsTextItem / QGraphicsSimpleTextItem for displaying values edited by dragging at the time they change
// Guide will reimplement shape() or contains() to simplify interaction via mouse

#include <qgraphicsitem.h>

// Allows user to manipulate CEGUI widget anchors like in Unity3D UI editor

class AnchorsManipulator : public QGraphicsItemGroup
{
public:

    AnchorsManipulator(QGraphicsItem* parent);
    //virtual ~AnchorsManipulator() override;

    void setAnchors(float minX, float minY, float maxX, float maxY);

    //virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    //virtual QPointF constrainMovePoint(QPointF value) override { return value; }
    //virtual QRectF constrainResizeRect(QRectF rect, QRectF oldRect) override;
    //virtual QSizeF getMinSize() const override { return QSizeF(0.0, 0.0); }
    //virtual QSizeF getMaxSize() const override { return QSizeF(std::numeric_limits<qreal>().max(), std::numeric_limits<qreal>().max()); }

    //virtual void notifyHandleSelected(ResizingHandle* /*handle*/) override {}
    //virtual void notifyResizeStarted() override {}
    //virtual void notifyResizeProgress(QPointF /*newPos*/, QRectF /*newRect*/) override {}
    //virtual void notifyResizeFinished(QPointF /*newPos*/, QRectF /*newRect*/) override {}
    //virtual void notifyMoveStarted() override {}
    //virtual void notifyMoveProgress(QPointF /*newPos*/) override {}
    //virtual void notifyMoveFinished(QPointF /*newPos*/) override {}

protected:

    //virtual QPen getNormalPen() const override;
    //virtual QPen getHoverPen() const override;
    //virtual QPen getPenWhileResizing() const override;
    //virtual QPen getPenWhileMoving() const override;

    //virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    //virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    //virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    QGraphicsLineItem* _minX = nullptr;
    QGraphicsLineItem* _maxX = nullptr;
    QGraphicsLineItem* _minY = nullptr;
    QGraphicsLineItem* _maxY = nullptr;
};

#endif // ANCHORSMANIPULATOR_H
