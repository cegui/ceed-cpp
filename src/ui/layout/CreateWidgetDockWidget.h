#ifndef CREATEWIDGETDOCKWIDGET_H
#define CREATEWIDGETDOCKWIDGET_H

#include <QDockWidget>

// This lists available widgets you can create and allows their creation (by drag N drop)

namespace Ui {
class CreateWidgetDockWidget;
}

class CreateWidgetDockWidget : public QDockWidget
{
    Q_OBJECT

public:

    explicit CreateWidgetDockWidget(QWidget *parent = nullptr);
    virtual ~CreateWidgetDockWidget() override;

    void populate();

private:

    Ui::CreateWidgetDockWidget *ui;
};

#endif // CREATEWIDGETDOCKWIDGET_H
