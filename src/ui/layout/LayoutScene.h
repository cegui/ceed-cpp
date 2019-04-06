#ifndef LAYOUTSCENE_H
#define LAYOUTSCENE_H

#include "src/ui/CEGUIGraphicsScene.h"

// This scene contains all the manipulators users want to interact it. You can visualise it as the
// visual editing centre screen where CEGUI is rendered.
// It renders CEGUI on it's background and outlines (via Manipulators) in front of it.

class LayoutVisualMode;
class LayoutManipulator;

class LayoutScene : public CEGUIGraphicsScene
{
public:

    LayoutScene(LayoutVisualMode& visualMode);

    void updateFromWidgets();
    virtual void setCEGUIDisplaySize(float width, float height, bool lazyUpdate = true) override;

    void setRootWidgetManipulator(LayoutManipulator* manipulator);
    LayoutManipulator* getRootWidgetManipulator() const { return rootManipulator; }
    LayoutManipulator* getManipulatorByPath(const QString& widgetPath) const;

    void normalisePositionOfSelectedWidgets();
    void normaliseSizeOfSelectedWidgets();
    void roundPositionOfSelectedWidgets();
    void roundSizeOfSelectedWidgets();
    bool deleteSelectedWidgets();

public slots:

    void slot_selectionChanged();

protected:

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void dropEvent(QGraphicsSceneDragDropEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    LayoutVisualMode& _visualMode;
    LayoutManipulator* rootManipulator = nullptr;

    bool ignoreSelectionChanges = false;
};

#endif // LAYOUTSCENE_H
