#ifndef RESIZINGHANDLE_H
#define RESIZINGHANDLE_H

#include "qgraphicsitem.h"

// A rectangle that when moved resizes the parent resizable rect item.
// The reason to go with a child GraphicsRectItem instead of just overriding mousePressEvent et al
// is to easily support multi selection resizing (you can multi-select various edges in all imaginable
// combinations and resize many things at once).

class QMouseEvent;
class ResizableRectItem;

class ResizingHandle : public QGraphicsRectItem
{
public:

    enum class Type
    {
        Top,
        Bottom,
        Left,
        Right,
        TopRight,
        BottomRight,
        BottomLeft,
        TopLeft
    };

    ResizingHandle(Type type, ResizableRectItem* parent = nullptr);

    QPointF performResizing(QPointF value);
    void onScaleChanged(qreal scaleX, qreal scaleY);
    void mouseReleaseEventSelected(QMouseEvent* event);

    void showHandle(bool show);
    void ignoreGeometryChanges(bool ignore) { _ignoreGeometryChanges = ignore; }

    bool isEdge() const { return _type == Type::Top || _type == Type::Bottom || _type == Type::Left || _type == Type::Right; }
    bool isHorizontal() const { return _type == Type::Top || _type == Type::Bottom; }
    bool isVertical() const { return _type == Type::Left || _type == Type::Right; }
    bool isCorner() const { return !isEdge(); }

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

    Type _type = Type::Top;
    bool _mouseOver = false;
    bool _ignoreGeometryChanges = false;
};

#endif // RESIZINGHANDLE_H
