#include "src/ui/imageset/ImageLabel.h"
#include "qapplication.h"
#include "qpalette.h"
#include "qpainter.h"

ImageLabel::ImageLabel(QGraphicsItem* parent)
    : QGraphicsTextItem(parent)
{
    setFlags(ItemIgnoresTransformations);
    setOpacity(0.8);
    setVisible(false);
    setPlainText("Unknown");

    // We make the label a lot more transparent when mouse is over it to make it easier
    // to work around the top edge of the image
    setAcceptHoverEvents(true);
}

void ImageLabel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->fillRect(boundingRect(), qApp->palette().color(QPalette::Normal, QPalette::Base));
    painter->drawRect(boundingRect());
    QGraphicsTextItem::paint(painter, option, widget);
}

void ImageLabel::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    QGraphicsTextItem::hoverEnterEvent(event);
    setOpacity(0.2);
}

void ImageLabel::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    setOpacity(0.8);
    QGraphicsTextItem::hoverLeaveEvent(event);
}
