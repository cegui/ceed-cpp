#ifndef LAYOUTCONTAINERHANDLE_H
#define LAYOUTCONTAINERHANDLE_H

#include <qgraphicsitem.h>

class LayoutManipulator;

class LayoutContainerHandle : public QGraphicsPixmapItem
{
public:

    LayoutContainerHandle(LayoutManipulator& host);

protected:

    void updateLook();

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    bool _hovered = false;
};

#endif // LAYOUTCONTAINERHANDLE_H
