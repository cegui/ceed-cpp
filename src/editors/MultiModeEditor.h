#ifndef MULTIMODEEDITOR_H
#define MULTIMODEEDITOR_H

#include "src/editors/EditorBase.h"
#include "qtabwidget.h"
#include "qundostack.h"

// Base interface for multi-mode editor tabs
class IEditMode
{
public:

    virtual ~IEditMode() {}

    virtual void activate() {}
    virtual bool deactivate() { return true; } // If this returns false, the action is terminated and the mode stays in place
};

// This class represents tabbed editor that has little tabs on the bottom
// allowing you to switch editing "modes" - visual, code, ...
// Not all modes have to be able to edit! Switching modes pushes undo actions
// onto the UndoStack to avoid confusion when undoing. These actions never merge
// together.
// You yourself are responsible for putting new tabs into this widget!
// You should not add/remove modes after the construction!
class MultiModeEditor : public EditorBase
{
public:

    MultiModeEditor(/*compatibilityManager, */ const QString& filePath);

    virtual void initialize() override;

    QString getTabText(int tabIndex) const { return tabs.tabText(tabIndex); }
    void setTabWithoutUndoHistory(int tabIndex);

protected:

    void slot_currentChanged(int newTabIndex);

    QTabWidget tabs;
    int currentTabIndex = -1;

    // When canceling tab transfer we have to switch back and avoid unnecessary deactivate/activate cycle
    bool ignoreCurrentChanged = false;

    // To avoid unnecessary undo command pushes we ignore currentChanged if we are inside ModeChangeCommand.undo or redo
    bool ignoreCurrentChangedForUndo = false;
};

// Undo command that is pushed to the undo stack whenever user switches edit modes
// Switching edit mode has to be an undoable command because the other commands might
// or might not make sense if user is not in the right mode.
// This has a drawback that switching to Live Preview (layout editing) and back is undoable
// even though you can't affect the document in any way whilst in Live Preview mode.
class ModeSwitchCommand : public QUndoCommand
{
public:

    ModeSwitchCommand(MultiModeEditor& editor, int oldTabIndex, int newTabIndex);

    virtual void undo() override;
    virtual void redo() override;

protected:

    MultiModeEditor& _editor;
    int _oldTabIndex = -1;
    int _newTabIndex = -1;
};

#endif // MULTIMODEEDITOR_H
