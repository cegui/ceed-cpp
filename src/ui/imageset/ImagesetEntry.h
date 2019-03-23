#ifndef IMAGESETENTRY_H
#define IMAGESETENTRY_H

#include "qgraphicsitem.h"

// This is the whole imageset containing all the images (ImageEntries).
// The main reason for this is not to have multiple imagesets editing at once but rather
// to have the transparency background working properly.

class QDomElement;

class ImagesetEntry : public QGraphicsPixmapItem
{
public:

    ImagesetEntry(QGraphicsItem *parent = nullptr);

    void loadFromElement(const QDomElement& xml);

protected:

    QString _name = "Unknown";
    QString autoScaled = "false";
    int nativeHorzRes = 800;
    int nativeVertRes = 600;
};

#endif // IMAGESETENTRY_H
