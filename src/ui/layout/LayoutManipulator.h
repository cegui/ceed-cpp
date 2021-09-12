#ifndef LAYOUTMANIPULATOR_H
#define LAYOUTMANIPULATOR_H

#include "src/cegui/CEGUIManipulator.h"
#include <set>

// Layout editing specific widget manipulator

class WidgetHierarchyItem;
class LayoutVisualMode;
class LayoutContainerHandle;

class LayoutManipulator : public CEGUIManipulator
{
public:

    static void removeNestedManipulators(std::set<LayoutManipulator*>& manipulators);

    LayoutManipulator(LayoutVisualMode& visualMode, QGraphicsItem* parent, CEGUI::Window* widget);
    virtual ~LayoutManipulator() override;

    virtual LayoutManipulator* createChildManipulator(CEGUI::Window* childWidget) override;
    void getChildLayoutManipulators(std::vector<LayoutManipulator*>& outList, bool recursive);

    virtual QPointF constrainMovePoint(QPointF value) override;
    virtual QRectF constrainResizeRect(QRectF rect, QRectF oldRect) override;

    virtual void notifyResizeStarted() override;
    virtual void notifyResizeProgress(QPointF newPos, QSizeF newSize) override;
    virtual void notifyResizeFinished(QPointF newPos, QSizeF newSize) override;
    virtual void notifyMoveStarted() override;
    virtual void notifyMoveProgress(QPointF newPos) override;
    virtual void notifyMoveFinished(QPointF newPos) override;

    virtual void updateFromWidget(bool callUpdate = false, bool updateAncestorLCs = false) override;
    virtual void detach(bool detachWidget = true, bool destroyWidget = true, bool recursive = true) override;

    virtual bool preventManipulatorOverlap() const override;
    virtual bool useAbsoluteCoordsForMove() const override;
    virtual bool useAbsoluteCoordsForResize() const override;
    virtual bool useIntegersForAbsoluteMove() const override;
    virtual bool useIntegersForAbsoluteResize() const override;

    bool renameWidget(QString& newName);

    void setLocked(bool locked);
    void setTreeItem(WidgetHierarchyItem* treeItem) { _treeItem = treeItem; }
    WidgetHierarchyItem* getTreeItem() const { return _treeItem; }

    void resetPen();

    void showLayoutContainerHandles(bool show);
    virtual void updateHandles() override;
    virtual bool isAnyHandleSelected() const override;
    virtual void deselectAllHandles() override;

    // For redirection from children
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

protected:

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void onPropertyChanged(const QtnPropertyBase* changedProperty, CEGUI::Property* ceguiProperty) override;
    virtual void onWidgetNameChanged() override;

    virtual QPen getNormalPen() const override;
    virtual QPen getHoverPen() const override;
    virtual QPen getPenWhileResizing() const override;
    virtual QPen getPenWhileMoving() const override;

    virtual void impl_paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

    qreal snapXCoordToGrid(qreal x);
    qreal snapYCoordToGrid(qreal y);

    LayoutVisualMode& _visualMode;
    WidgetHierarchyItem* _treeItem = nullptr;
    LayoutContainerHandle* _lcHandle = nullptr;

    QPointF _lastNewPos;
    QSizeF _lastNewSize;

    bool _showOutline = true;
    bool _resizeable = true;
    bool _drawSnapGrid = false;
    bool _snapGridNonClientArea = false;
    bool _ignoreSnapGrid = false;
};

#endif // LAYOUTMANIPULATOR_H
