#ifndef IMAGEENTRY_H
#define IMAGEENTRY_H

#include "src/ui/ResizableRectItem.h"

// Represents the image of the imageset, can be drag moved, selected, resized, ...

class QDomElement;
class ImageLabel;
class ImageOffsetMark;

class ImageEntry : public ResizableRectItem
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

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    QPixmap getPixmap();
    void updateListItemSelection();

    ImageLabel* label = nullptr;
    ImageOffsetMark* offset = nullptr;

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
