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

private slots:

    void onNativeResolutionEdited();

    void on_name_textEdited(const QString &arg1);

    void on_imageLoad_clicked();

    void on_autoScaled_currentIndexChanged(int index);

    void on_filterBox_textChanged(const QString &arg1);

    void on_list_itemChanged(QListWidgetItem *item);

    void on_list_itemSelectionChanged();

private:

    Ui::ImagesetEditorDockWidget *ui;
    ImagesetEntry* imagesetEntry = nullptr;

    bool selectionUnderway = false;
    bool selectionSynchronizationUnderway = false;
};

#endif // IMAGESETEDITORDOCKWIDGET_H
