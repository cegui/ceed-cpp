#ifndef IMAGESETEDITORDOCKWIDGET_H
#define IMAGESETEDITORDOCKWIDGET_H

#include <QDockWidget>

namespace Ui {
class ImagesetEditorDockWidget;
}

class ImagesetEditorDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ImagesetEditorDockWidget(QWidget *parent = nullptr);
    ~ImagesetEditorDockWidget();

private:
    Ui::ImagesetEditorDockWidget *ui;
};

#endif // IMAGESETEDITORDOCKWIDGET_H
