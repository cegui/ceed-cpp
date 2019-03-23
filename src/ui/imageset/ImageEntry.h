#ifndef IMAGEENTRY_H
#define IMAGEENTRY_H

//!!!DBG TMP!
#include "qgraphicsitem.h"

// Represents the image of the imageset, can be drag moved, selected, resized, ...

class QDomElement;

//!!!ResizableRectItem subclass!
class ImageEntry : public QGraphicsRectItem
{
public:

    ImageEntry(QGraphicsItem* parent = nullptr);

    void loadFromElement(const QDomElement& xml);
    void saveToElement(QDomElement& xml);

    void updateDockWidget();
    void updateListItem();

    QString name() const;
    void setName(const QString& newName);

protected:

    QPixmap getPixmap();

    QString autoScaled = "";
    int nativeHorzRes = 0;
    int nativeVertRes = 0;
    bool isHovered = false;

    // Used for undo
    bool potentialMove = false;
    QPoint oldPosition;
    bool resized = false;
};

#endif // IMAGEENTRY_H
