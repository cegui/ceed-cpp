#ifndef LOOKNFEELVISUALMODE_H
#define LOOKNFEELVISUALMODE_H

#include "src/editors/MultiModeEditor.h"
#include "qwidget.h"

// This is the default visual editing mode for look'n'feel

class LookNFeelEditor;

class LookNFeelVisualMode : public QWidget, public IEditMode
{
public:

    LookNFeelVisualMode(LookNFeelEditor& editor);

    void initialize();

protected:

    void setupActions();
};

#endif // LOOKNFEELVISUALMODE_H
