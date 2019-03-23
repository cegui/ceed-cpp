#ifndef IMAGESETVISUALMODE_H
#define IMAGESETVISUALMODE_H

#include "src/editors/MultiModeEditor.h"
#include "src/ui/ResizableGraphicsView.h"

// This is the "Visual" tab for imageset editing

class ImagesetVisualMode : public ResizableGraphicsView, public IEditMode
{
public:

    ImagesetVisualMode(MultiModeEditor& editor);

protected:

    QPoint lastMousePosition;
};

#endif // IMAGESETVISUALMODE_H
