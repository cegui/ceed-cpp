#ifndef IMAGESETEDITOR_H
#define IMAGESETEDITOR_H

#include "src/editors/MultiModeEditor.h"

// Binds all imageset editing functionality together
// Special words used in imageset editing code:
// Imageset - definition of image rectangles on specified underlying image (~texture atlas)
// Image Entry - one rectangle of the imageset
// Image Offset - allows you to change the pivot point of the image entry which by default is at the
//                top left corner of the image. To make the pivot point be at the centre of the image
//                that is 25x25 pixels, set the offset to -12, -12
// Underlying image - the image that lies under the image entries/rectangles (bitmap image)

class ImagesetEditor : public MultiModeEditor
{
public:

    ImagesetEditor(const QString& filePath);

    virtual void initialize() override;
    virtual void finalize() override;
    virtual void activate(QMenu* editorMenu) override;
    virtual void deactivate() override;

    // Application commands implementation
//    virtual void copy() {}
//    virtual void cut() {}
//    virtual void paste() {}
//    virtual void deleteSelected() {}
//    virtual void undo();
//    virtual void redo();
//    virtual void revert();
//    virtual void zoomIn() {}
//    virtual void zoomOut() {}
//    virtual void zoomReset() {}
//    //virtual void zoomFit() {}

    virtual QWidget* getWidget() override { return &tabs; }
    //virtual bool hasChanges() const;
    //virtual bool requiresProject() const { return false; }

protected:

    virtual void setupEditorMenu(QMenu* editorMenu) override;
    //virtual void getRawData(QByteArray& outRawData) {}
    //virtual void markAsUnchanged();
};

class ImagesetEditorFactory : public EditorFactoryBase
{
public:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // IMAGESETEDITOR_H
