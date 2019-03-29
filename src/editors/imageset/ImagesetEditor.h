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

class ImagesetVisualMode;
class ImagesetCodeMode;
class Settings;

class ImagesetEditor : public MultiModeEditor
{
public:

    static void createSettings(Settings& mgr);

    ImagesetEditor(const QString& filePath);

    virtual void initialize() override;
    virtual void finalize() override;
    virtual void activate(MainWindow& mainWindow) override;
    virtual void deactivate(MainWindow& mainWindow) override;

    // Application commands implementation
    virtual void copy() override;
    virtual void cut() override;
    virtual void paste() override;
    virtual void deleteSelected() override;
    virtual void zoomIn() override;
    virtual void zoomOut() override;
    virtual void zoomReset() override;
    //virtual void zoomFit() {}

    virtual QWidget* getWidget() override { return &tabs; }

    ImagesetVisualMode* getVisualMode() const { return visualMode; }
    QString getSourceCode() const;

protected:

    virtual void getRawData(QByteArray& outRawData) override;

    ImagesetVisualMode* visualMode = nullptr;
    ImagesetCodeMode* codeMode = nullptr;
};

class ImagesetEditorFactory : public EditorFactoryBase
{
public:

    virtual QString getFileTypesDescription() const override;
    virtual QStringList getFileExtensions() const override;
    virtual EditorBasePtr create(const QString& filePath) const override;
};

#endif // IMAGESETEDITOR_H
