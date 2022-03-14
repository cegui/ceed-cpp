#ifndef LAYOUTSCENE_H
#define LAYOUTSCENE_H

#include "src/ui/CEGUIGraphicsScene.h"
#include <CEGUI/HorizontalAlignment.h>
#include <CEGUI/VerticalAlignment.h>
#include <qmenu.h>
#include <set>

// This scene contains all the manipulators users want to interact it. You can visualise it as the
// visual editing centre screen where CEGUI is rendered.
// It renders CEGUI on it's background and outlines (via Manipulators) in front of it.

class LayoutVisualMode;
class LayoutManipulator;
class AnchorEdgeHandle;
class AnchorCornerHandle;
class NumericValueItem;
class QtnPropertySet;
class AnchorPopupMenu;

class LayoutScene : public CEGUIGraphicsScene
{
    Q_OBJECT

public:

    LayoutScene(LayoutVisualMode& visualMode);
    virtual ~LayoutScene() override;

    void setupContextMenu();
    void updateFromWidgets();
    virtual void setCEGUIDisplaySize(float width, float height) override;

    void setRootWidgetManipulator(LayoutManipulator* manipulator);
    LayoutManipulator* getRootWidgetManipulator() const { return _rootManipulator; }
    LayoutManipulator* getManipulatorByPath(const QString& widgetPath) const;
    bool deleteWidgetByPath(const QString& widgetPath);
    size_t getMultiSelectionChangeId() const;
    void updatePropertySet();
    void updatePropertySet(const std::set<LayoutManipulator*>& selectedWidgets);
    void updatePropertyWidgetTitle(const std::set<LayoutManipulator*>& selectedWidgets);

    void alignSelectionHorizontally(CEGUI::HorizontalAlignment alignment);
    void alignSelectionVertically(CEGUI::VerticalAlignment alignment);
    void moveSelectedWidgetsInParentWidgetLists(int delta);

    void onManipulatorRemoved(LayoutManipulator* manipulator);
    void onManipulatorUpdatedFromWidget(LayoutManipulator* manipulator);
    void onManipulatorDragEnter(LayoutManipulator* manipulator);
    void onManipulatorDragLeave(LayoutManipulator* manipulator);
    void anchorHandleMoved(QGraphicsItem* item, QPointF& newPos, bool moveOpposite);
    void anchorHandleSelected(QGraphicsItem* item);

    LayoutManipulator* getAnchorTarget() const { return _anchorTarget; }
    bool getAnchorValues(float& minX, float& maxX, float& minY, float& maxY) const;
    void setAnchorValues(float minX, float maxX, float minY, float maxY, bool preserveEffectiveSize);

    LayoutManipulator* getManipulatorFromItem(QGraphicsItem* item) const;
    void collectSelectedWidgets(std::set<LayoutManipulator*>& selectedWidgets);
    void selectWidgetsByPaths(const std::set<QString>& paths);
    void selectAllWidgets();
    void ignoreSelectionChanges(bool ignore) { _ignoreSelectionChanges = ignore; }

    bool isAnchorItem(QGraphicsItem* item) const;
    QGraphicsItem* getCurrentAnchorItem() const;
    void deselectAllAnchorItems();
    void updateAnchorItems(QGraphicsItem* movedItem = nullptr);
    void updateAnchorValueItems();

    void showAnchorPopupMenu(const QPoint& pos);

public slots:

    void normalizePositionOfSelectedWidgets();
    void normalizeSizeOfSelectedWidgets();
    void roundPositionOfSelectedWidgets();
    void roundSizeOfSelectedWidgets();
    void selectParent();
    bool deleteSelectedWidgets();
    void showLayoutContainerHandles(bool show);

    void onSelectionChanged();
    void onBeforePropertyEdited();

protected:

    void createAnchorItems();

    void setupActionsForTabControl();

    void updateStatusMessage();

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    LayoutVisualMode& _visualMode;
    LayoutManipulator* _rootManipulator = nullptr;

    QtnPropertySet* _multiSet = nullptr;
    size_t _multiChangeId = 0;

    AnchorPopupMenu* _anchorPopupMenu = nullptr;
    QMenu* _contextMenu = nullptr;
    std::map<QString, std::vector<std::pair<QAction*, std::function<bool()>>>> _widgetActions; // Widget type -> {action + condition}
    QPoint _contextMenuPos;
    LayoutManipulator* _contextMenuWidget = nullptr;
    LayoutManipulator* _dragDropTarget = nullptr;

    QPointF _lastCursorPos;

    // Anchor editing items
    LayoutManipulator* _anchorTarget = nullptr;
    LayoutManipulator* _anchorSnapTarget = nullptr;
    QGraphicsRectItem* _anchorParentRect = nullptr;
    AnchorEdgeHandle* _anchorMinX = nullptr;
    AnchorEdgeHandle* _anchorMinY = nullptr;
    AnchorEdgeHandle* _anchorMaxX = nullptr;
    AnchorEdgeHandle* _anchorMaxY = nullptr;
    AnchorCornerHandle* _anchorMinXMinY = nullptr;
    AnchorCornerHandle* _anchorMaxXMinY = nullptr;
    AnchorCornerHandle* _anchorMinXMaxY = nullptr;
    AnchorCornerHandle* _anchorMaxXMaxY = nullptr;
    NumericValueItem* _anchorTextX = nullptr;
    NumericValueItem* _anchorTextY = nullptr;

    bool _ignoreSelectionChanges = false;
    bool _batchSelection = false;
};

#endif // LAYOUTSCENE_H
