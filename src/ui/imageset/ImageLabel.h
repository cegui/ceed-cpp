#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include "qgraphicsitem.h"

// Text item showing image's label when the image is hovered or selected.
// You should not use this directly! Use ImageEntry.name instead to get the name.

class ImageLabel : public QGraphicsTextItem
{
public:

    ImageLabel(QGraphicsItem* parent = nullptr);

protected:

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};

#endif // IMAGELABEL_H
