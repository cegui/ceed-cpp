#ifndef CEGUIWIDGET_H
#define CEGUIWIDGET_H

#include <QWidget>

// CEGUI context container widget with size control UI

namespace Ui {
class CEGUIWidget;
}

class CEGUIWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CEGUIWidget(QWidget *parent = nullptr);
    ~CEGUIWidget();

    void makeOpenGLContextCurrent();
    void setInputEnabled(bool enable);

private slots:
    void on_debugInfoButton_clicked();

private:
    Ui::CEGUIWidget *ui;
};

#endif // CEGUIWIDGET_H
