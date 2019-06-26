#ifndef LAYOUTMANIPULATOR_H
#define LAYOUTMANIPULATOR_H

#include "src/cegui/CEGUIManipulator.h"

// Layout editing specific widget manipulator

class WidgetHierarchyItem;
class LayoutVisualMode;

class LayoutManipulator : public CEGUIManipulator
{
public:

    LayoutManipulator(LayoutVisualMode& visualMode, QGraphicsItem* parent, CEGUI::Window* widget);
    virtual ~LayoutManipulator() override;

    virtual LayoutManipulator* createChildManipulator(CEGUI::Window* childWidget, bool recursive = true, bool skipAutoWidgets = false) override;
    void getChildLayoutManipulators(std::vector<LayoutManipulator*>& outList, bool recursive);

    virtual QPointF constrainMovePoint(QPointF value) override;
    virtual QRectF constrainResizeRect(QRectF rect, QRectF oldRect) override;

    virtual void notifyResizeStarted() override;
    virtual void notifyResizeProgress(QPointF newPos, QRectF newRect) override;
    virtual void notifyResizeFinished(QPointF newPos, QRectF newRect) override;
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

    void setLocked(bool locked);
    void setTreeItem(WidgetHierarchyItem* treeItem) { _treeItem = treeItem; }
    WidgetHierarchyItem* getTreeItem() const { return _treeItem; }

    void resetPen();

protected:

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;

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

    QPointF _lastNewPos;
    QRectF _lastNewRect;

    bool _showOutline = true;
    bool _drawSnapGrid = false;
    bool _snapGridNonClientArea = false;
    bool _ignoreSnapGrid = false;
};

#endif // LAYOUTMANIPULATOR_H
