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

void AnchorsManipulator::setAnchors(float minX, float minY, float maxX, float maxY)
{
    const QPointF offset = mapFromScene(0.0, 0.0);

    const float ctxWidth = static_cast<LayoutScene*>(scene())->getContextWidth();
    const float ctxHeight = static_cast<LayoutScene*>(scene())->getContextHeight();
    const qreal absMinX = static_cast<qreal>(minX * ctxWidth) + offset.x();
    const qreal absMinY = static_cast<qreal>(minY * ctxHeight) + offset.y();
    const qreal absMaxX = static_cast<qreal>(maxX * ctxWidth) + offset.x();
    const qreal absMaxY = static_cast<qreal>(maxY * ctxHeight) + offset.y();

    //???or change position here and change line only when scene size changes?
    _minX->setLine(absMinX, offset.y(), absMinX, static_cast<qreal>(ctxHeight));
    _maxX->setLine(absMaxX, offset.y(), absMaxX, static_cast<qreal>(ctxHeight));
    _minY->setLine(offset.x(), absMinY, static_cast<qreal>(ctxWidth), absMinY);
    _maxY->setLine(offset.x(), absMaxY, static_cast<qreal>(ctxWidth), absMaxY);
}
