#include "src/ui/imageset/ImageLabel.h"
#include "qapplication.h"
#include "qpalette.h"
#include "qpainter.h"

ImageLabel::ImageLabel(QGraphicsItem* parent)
    : QGraphicsTextItem(parent)
{
    setOpacity(0.8);
    setVisible(false);
    setPlainText("Unknown");

    // We make the label a lot more transparent when mouse is over it to make it easier
    // to work around the top edge of the image
    setAcceptHoverEvents(true);

    setY(-boundingRect().height() - 2.0);
}

void ImageLabel::onScaleChanged(qreal scaleX, qreal scaleY)
{
    auto tfm = transform();

    const qreal counterScaleX = (1.0 / scaleX);
    const qreal counterScaleY = (1.0 / scaleY);

    tfm = QTransform(counterScaleX, tfm.m12(), tfm.m13(),
                     tfm.m21(), counterScaleY, tfm.m23(),
                     tfm.m31(), tfm.m32(), tfm.m33());

    setTransform(tfm);
    setY((-boundingRect().height() - 2.0) * counterScaleY);
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
