#ifndef IMAGESETVISUALMODE_H
#define IMAGESETVISUALMODE_H

#include "src/editors/MultiModeEditor.h"

//!!!DBG TMP!
#include "qgraphicsview.h"

// This is the "Visual" tab for imageset editing

//!!!resizable.GraphicsView!
class ImagesetVisualMode : public QGraphicsView, public IEditMode
{
public:

    ImagesetVisualMode(MultiModeEditor& editor);
};

#endif // IMAGESETVISUALMODE_H
