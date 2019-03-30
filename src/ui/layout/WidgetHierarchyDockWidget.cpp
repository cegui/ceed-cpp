#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "ui_WidgetHierarchyDockWidget.h"

WidgetHierarchyDockWidget::WidgetHierarchyDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::WidgetHierarchyDockWidget)
{
    ui->setupUi(this);
}

WidgetHierarchyDockWidget::~WidgetHierarchyDockWidget()
{
    delete ui;
}
