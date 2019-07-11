#ifndef LAYOUTCONTAINERHANDLE_H
#define LAYOUTCONTAINERHANDLE_H

#include <qgraphicsitem.h>

class LayoutManipulator;

class LayoutContainerHandle : public QGraphicsPixmapItem
{
public:

    LayoutContainerHandle(LayoutManipulator& host);

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};

#endif // LAYOUTCONTAINERHANDLE_H
