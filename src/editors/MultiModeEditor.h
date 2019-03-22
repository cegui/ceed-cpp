#ifndef MULTIMODEEDITOR_H
#define MULTIMODEEDITOR_H

#include "src/editors/EditorBase.h"
#include "qtabwidget.h"

// This class represents tabbed editor that has little tabs on the bottom
// allowing you to switch editing "modes" - visual, code, ...
// Not all modes have to be able to edit! Switching modes pushes undo actions
// onto the UndoStack to avoid confusion when undoing. These actions never merge
// together.
// You yourself are responsible for putting new tabs into this widget!
// You should not add/remove modes after the construction!

// Base interface for multi-mode editor tabs
class IEditMode
{
public:

    virtual ~IEditMode() {}

    virtual void activate() {}
    virtual bool deactivate() { return true; } // If this returns false, the action is terminated and the mode stays in place
};

class MultiModeEditor : public EditorBase
{
public:

    MultiModeEditor(/*compatibilityManager, */ const QString& filePath);

    virtual void initialize() override;

protected:

    void slot_currentChanged(int newTabIndex);

    QTabWidget tabs;
    int currentTabIndex = -1;

    // When canceling tab transfer we have to switch back and avoid unnecessary deactivate/activate cycle
    bool ignoreCurrentChanged = false;

    // To avoid unnecessary undo command pushes we ignore currentChanged if we are inside ModeChangeCommand.undo or redo
    bool ignoreCurrentChangedForUndo = false;
};

#endif // MULTIMODEEDITOR_H
