#ifndef IMAGESETVISUALMODE_H
#define IMAGESETVISUALMODE_H

#include "src/editors/MultiModeEditor.h"
#include "src/ui/ResizableGraphicsView.h"

// This is the "Visual" tab for imageset editing

class ImagesetEntry;
class QDomElement;

class ImagesetVisualMode : public ResizableGraphicsView, public IEditMode
{
public:

    ImagesetVisualMode(MultiModeEditor& editor);

    void loadImagesetEntryFromElement(const QDomElement& xmlRoot);

protected slots:

    void slot_selectionChanged();

protected:

    void setupActions();

    QPoint lastMousePosition;
    ImagesetEntry* imagesetEntry = nullptr;
};

#endif // IMAGESETVISUALMODE_H
