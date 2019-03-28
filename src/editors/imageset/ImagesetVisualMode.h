#ifndef IMAGESETVISUALMODE_H
#define IMAGESETVISUALMODE_H

#include "src/editors/MultiModeEditor.h"
#include "src/ui/ResizableGraphicsView.h"

// This is the "Visual" tab for imageset editing

class ImageEntry;
class ImagesetEntry;
class ImagesetEditorDockWidget;
class QDomElement;

class ImagesetVisualMode : public ResizableGraphicsView, public IEditMode
{
public:

    ImagesetVisualMode(MultiModeEditor& editor);

    void loadImagesetEntryFromElement(const QDomElement& xmlRoot);

    void refreshSceneRect();

    void createImageEntry(QPointF pos);
    bool moveImageEntries(const std::vector<ImageEntry*>& imageEntries, QPointF delta);
    bool resizeImageEntries(const std::vector<ImageEntry*>& imageEntries, QPointF topLeftDelta, QPointF bottomRightDelta);
    bool deleteImageEntries(const std::vector<ImageEntry*>& imageEntries);
    bool deleteSelectedImageEntries();

    ImagesetEntry* getImagesetEntry() const { return imagesetEntry; }
    ImagesetEditorDockWidget* getDockWidget() const { return dockWidget; }

protected slots:

    bool cycleOverlappingImages();
    void slot_selectionChanged();

protected:

    void setupActions();

    QPoint lastMousePosition;
    ImagesetEntry* imagesetEntry = nullptr;
    ImagesetEditorDockWidget* dockWidget = nullptr;
};

#endif // IMAGESETVISUALMODE_H
