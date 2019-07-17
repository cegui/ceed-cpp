#ifndef LAYOUTCONTAINERHANDLE_H
#define LAYOUTCONTAINERHANDLE_H

#include <qgraphicsitem.h>

class LayoutManipulator;

class LayoutContainerHandle : public QGraphicsPixmapItem
{
public:

    LayoutContainerHandle(LayoutManipulator& host);

    void updateLook();
    void updatePositionAndScale(qreal scaleX, qreal scaleY);
    void updateTooltip();

protected:

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    bool _mouseOver = false;
    bool _ignoreGeometryChanges = false;
};

#endif // LAYOUTCONTAINERHANDLE_H
