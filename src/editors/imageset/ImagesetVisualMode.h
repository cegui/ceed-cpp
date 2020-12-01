#ifndef IMAGESETVISUALMODE_H
#define IMAGESETVISUALMODE_H

#include "src/editors/MultiModeEditor.h"
#include "src/ui/ResizableGraphicsView.h"

// This is the "Visual" tab for imageset editing

class ImageEntry;
class ImagesetEntry;
class ImagesetEditorDockWidget;
class QDomElement;
class QMenu;

class ImagesetVisualMode : public ResizableGraphicsView, public IEditMode
{
public:

    ImagesetVisualMode(MultiModeEditor& editor);
    virtual ~ImagesetVisualMode() override;

    virtual void activate(MainWindow& mainWindow) override;
    virtual bool deactivate(MainWindow& mainWindow, bool editorDeactivated) override;

    void loadImagesetEntryFromElement(const QDomElement& xmlRoot);
    void rebuildEditorMenu(QMenu* editorMenu);

    void refreshSceneRect();

    void createImageEntry(QPointF pos);
    void createImageEntryAtCursor() { createImageEntry(lastCursorPosition); }
    bool moveImageEntries(const std::vector<ImageEntry*>& imageEntries, QPointF delta);
    bool resizeImageEntries(const std::vector<ImageEntry*>& imageEntries, QPointF topLeftDelta, QPointF bottomRightDelta);
    bool deleteImageEntries(const std::vector<ImageEntry*>& imageEntries);
    bool deleteSelectedImageEntries();
    bool duplicateImageEntries(const std::vector<ImageEntry*>& imageEntries);
    bool duplicateSelectedImageEntries();

    bool cut();
    bool copy();
    bool paste();

    ImagesetEntry* getImagesetEntry() const { return imagesetEntry; }
    ImagesetEditorDockWidget* getDockWidget() const { return dockWidget; }

protected slots:

    bool cycleOverlappingImages();
    void slot_selectionChanged();
    void slot_toggleEditOffsets(bool enabled);
    void slot_customContextMenu(QPoint point);

protected:

    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;

    void createActiveStateConnections();
    QString getNewImageName(const QString& desiredName, QString copyPrefix = "", QString copySuffix = "_copy");

    QPointF lastCursorPosition;
    ImagesetEntry* imagesetEntry = nullptr;
    ImagesetEditorDockWidget* dockWidget = nullptr;
    QMenu* contextMenu = nullptr;

    QAction* editOffsetsAction = nullptr;
    QAction* cycleOverlappingAction = nullptr;
    QAction* createImageAction = nullptr;
    QAction* duplicateSelectedImagesAction = nullptr;
    QAction* focusImageListFilterBoxAction = nullptr;
};

#endif // IMAGESETVISUALMODE_H
