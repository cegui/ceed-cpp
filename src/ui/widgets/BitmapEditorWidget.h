#ifndef BITMAPEDITORWIDGET_H
#define BITMAPEDITORWIDGET_H

#include <QWidget>

namespace Ui {
class BitmapEditorWidget;
}

class BitmapEditorWidget : public QWidget
{
    Q_OBJECT

public:

    explicit BitmapEditorWidget(QWidget *parent = nullptr);
    ~BitmapEditorWidget();

    void openFile(const QString& filePath);

private:

    Ui::BitmapEditorWidget *ui;
};

#endif // BITMAPEDITORWIDGET_H
