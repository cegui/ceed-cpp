#ifndef IMAGEENTRY_H
#define IMAGEENTRY_H

#include "src/ui/ResizableRectItem.h"

// Represents the image of the imageset, can be drag moved, selected, resized, ...

class QDomElement;
class QListWidgetItem;
class ImageLabel;
class ImageOffsetMark;

class ImageEntry : public ResizableRectItem
{
public:

    ImageEntry(QGraphicsItem* parent = nullptr);

    virtual QRectF constrainResizeRect(QRectF rect, QRectF oldRect) override;
    virtual void notifyResizeStarted() override;
    virtual void notifyResizeFinished(QPointF newPos, QRectF newRect) override;

    void loadFromElement(const QDomElement& xml);
    void saveToElement(QDomElement& xml);

    void updateDockWidget();
    void updateListItem();
    void setListItem(QListWidgetItem* newItem) { listItem = newItem; }
    QListWidgetItem* getListItem() const { return listItem; }
    ImageOffsetMark* getOffsetMark() const { return offset; }

    QString name() const;
    void setName(const QString& newName);
    int offsetX() const;
    void setOffsetX(int value);
    int offsetY() const;
    void setOffsetY(int value);
    QString getAutoScaled() const { return autoScaled; }
    int getNativeHorzRes() const { return nativeHorzRes; }
    int getNativeVertRes() const { return nativeVertRes; }
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name);

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

protected:

    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    QPixmap getPixmap();
    void updateListItemSelection();

    ImageLabel* label = nullptr;
    ImageOffsetMark* offset = nullptr;
    QListWidgetItem* listItem = nullptr; // This allows fast updates of the list item without looking it up

    QString autoScaled = "";
    int nativeHorzRes = 0;
    int nativeVertRes = 0;
    bool isHovered = false;

    // Used for undo
    bool potentialMove = false;
    QPointF oldPosition;
    bool resized = false;
};

Q_DECLARE_METATYPE(ImageEntry*); // For storing in list items of the ImagesetEditorDockWidget

#endif // IMAGEENTRY_H
