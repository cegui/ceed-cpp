#ifndef LOOKNFEELPREVIEWMODE_H
#define LOOKNFEELPREVIEWMODE_H

#include "src/editors/MultiModeEditor.h"
#include "qwidget.h"

//???unify with LayoutPreviewMode? rootWindow as input.

// Provides "Live Preview" which is basically interactive CEGUI rendering
// without any other outlines or what not over it.

class LookNFeelEditor;

class LookNFeelPreviewMode : public QWidget, public IEditMode
{
public:

    LookNFeelPreviewMode(LookNFeelEditor& editor);
};

#endif // LOOKNFEELPREVIEWMODE_H
