#include "src/editors/layout/AnchorsManipulator.h"
#include "src/ui/layout/LayoutScene.h"
#include <qcursor.h>

AnchorsManipulator::AnchorsManipulator(QGraphicsItem* parent)
    : QGraphicsItemGroup(parent)
{
    //!!!subclass Guide - args horz/vert, pen! Always sized to the whole scene, cursor is automatic.
    QPen guidePen(Qt::GlobalColor::white);
    guidePen.setWidth(2);
    guidePen.setCosmetic(true);
    guidePen.setStyle(Qt::PenStyle::DashLine);

    _minX = new QGraphicsLineItem(this);
    _minX->setPen(guidePen);
    _minX->setCursor(Qt::SizeHorCursor);

    _maxX = new QGraphicsLineItem(this);
    _maxX->setPen(guidePen);
    _maxX->setCursor(Qt::SizeHorCursor);

    _minY = new QGraphicsLineItem(this);
    _minY->setPen(guidePen);
    _minY->setCursor(Qt::SizeVerCursor);

    _maxY = new QGraphicsLineItem(this);
    _maxY->setPen(guidePen);
    _maxY->setCursor(Qt::SizeVerCursor);
}

void AnchorsManipulator::setAnchors(float minX, float minY, float sizeX, float sizeY)
{
    const QPointF parentPos = parentItem() ? parentItem()->pos() : QPointF();

    const float ctxWidth = static_cast<LayoutScene*>(scene())->getContextWidth();
    const float ctxHeight = static_cast<LayoutScene*>(scene())->getContextHeight();
    const qreal absMinX = static_cast<qreal>(minX * ctxWidth) - parentPos.x();
    const qreal absMinY = static_cast<qreal>(minY * ctxHeight) - parentPos.y();
    const qreal absMaxX = absMinX + static_cast<qreal>(sizeX * ctxWidth);
    const qreal absMaxY = absMinY + static_cast<qreal>(sizeY * ctxHeight);

    //???or change position here and change line only when scene size changes?
    _minX->setLine(absMinX, -parentPos.y(), absMinX, static_cast<qreal>(ctxHeight));
    _maxX->setLine(absMaxX, -parentPos.y(), absMaxX, static_cast<qreal>(ctxHeight));
    _minY->setLine(-parentPos.x(), absMinY, static_cast<qreal>(ctxWidth), absMinY);
    _maxY->setLine(-parentPos.x(), absMaxY, static_cast<qreal>(ctxWidth), absMaxY);
}
