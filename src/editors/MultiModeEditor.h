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

class MultiModeEditor : public EditorBase
{
public:

    MultiModeEditor(/*compatibilityManager, */ const QString& filePath);

protected:

    QTabWidget tabs;
};

#endif // MULTIMODEEDITOR_H
