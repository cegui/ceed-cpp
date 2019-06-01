#ifndef LAYOUTSCENE_H
#define LAYOUTSCENE_H

#include "src/ui/CEGUIGraphicsScene.h"
#include <CEGUI/HorizontalAlignment.h>
#include <CEGUI/VerticalAlignment.h>

// This scene contains all the manipulators users want to interact it. You can visualise it as the
// visual editing centre screen where CEGUI is rendered.
// It renders CEGUI on it's background and outlines (via Manipulators) in front of it.

class LayoutVisualMode;
class LayoutManipulator;
class AnchorEdgeHandle;
class AnchorCornerHandle;
class NumericValueItem;

class LayoutScene : public CEGUIGraphicsScene
{
public:

    LayoutScene(LayoutVisualMode& visualMode);

    void updateFromWidgets();
    virtual void setCEGUIDisplaySize(float width, float height) override;

    void setRootWidgetManipulator(LayoutManipulator* manipulator);
    LayoutManipulator* getRootWidgetManipulator() const { return rootManipulator; }
    LayoutManipulator* getManipulatorByPath(const QString& widgetPath) const;

    void normalizePositionOfSelectedWidgets();
    void normalizeSizeOfSelectedWidgets();
    void roundPositionOfSelectedWidgets();
    void roundSizeOfSelectedWidgets();
    void alignSelectionHorizontally(CEGUI::HorizontalAlignment alignment);
    void alignSelectionVertically(CEGUI::VerticalAlignment alignment);
    void moveSelectedWidgetsInParentWidgetLists(int delta);
    bool deleteSelectedWidgets();

    void ignoreSelectionChanges(bool ignore) { _ignoreSelectionChanges = ignore; }

    void anchorHandleMoved(QGraphicsItem* item, QPointF& delta, bool moveOpposite);
    void anchorHandleSelected(QGraphicsItem* item);

public slots:

    void slot_selectionChanged();

protected:

    void updateAnchorItems(QGraphicsItem* movedItem = nullptr);

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    LayoutVisualMode& _visualMode;
    LayoutManipulator* rootManipulator = nullptr;

    // Anchor editing items
    LayoutManipulator* _anchorTarget = nullptr;
    QGraphicsRectItem* _anchorParentRect = nullptr;
    AnchorEdgeHandle* _anchorMinX = nullptr;
    AnchorEdgeHandle* _anchorMinY = nullptr;
    AnchorEdgeHandle* _anchorMaxX = nullptr;
    AnchorEdgeHandle* _anchorMaxY = nullptr;
    AnchorCornerHandle* _anchorMinXMinY = nullptr;
    AnchorCornerHandle* _anchorMaxXMinY = nullptr;
    AnchorCornerHandle* _anchorMinXMaxY = nullptr;
    AnchorCornerHandle* _anchorMaxXMaxY = nullptr;
    NumericValueItem* _currItemText = nullptr;

    bool _ignoreSelectionChanges = false;
};

#endif // LAYOUTSCENE_H
