#ifndef IMAGESETEDITORDOCKWIDGET_H
#define IMAGESETEDITORDOCKWIDGET_H

#include <QDockWidget>

// Provides list of images, property editing of currently selected image and create/delete

class ImagesetEntry;

namespace Ui {
class ImagesetEditorDockWidget;
}

class ImagesetEditorDockWidget : public QDockWidget
{
    Q_OBJECT

public:

    explicit ImagesetEditorDockWidget(QWidget *parent = nullptr);
    ~ImagesetEditorDockWidget();

    void setImagesetEntry(ImagesetEntry* entry) { imagesetEntry = entry; }

    void refresh();

private:

    Ui::ImagesetEditorDockWidget *ui;
    ImagesetEntry* imagesetEntry = nullptr;
};

#endif // IMAGESETEDITORDOCKWIDGET_H
