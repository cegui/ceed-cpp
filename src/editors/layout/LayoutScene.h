#ifndef LAYOUTSCENE_H
#define LAYOUTSCENE_H

#include "src/ui/CEGUIGraphicsScene.h"

// This scene contains all the manipulators users want to interact it. You can visualise it as the
// visual editing centre screen where CEGUI is rendered.
// It renders CEGUI on it's background and outlines (via Manipulators) in front of it.

class LayoutVisualMode;

class LayoutScene : public CEGUIGraphicsScene
{
public:

    LayoutScene(LayoutVisualMode& visualMode);

    bool deleteSelectedWidgets();

public slots:

    void slot_selectionChanged();

protected:

    LayoutVisualMode& _visualMode;

    bool ignoreSelectionChanges = false;
};

#endif // LAYOUTSCENE_H
