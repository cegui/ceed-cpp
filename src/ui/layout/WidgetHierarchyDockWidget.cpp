#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "ui_WidgetHierarchyDockWidget.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/ui/layout/WidgetHierarchyTreeModel.h"
#include "src/ui/layout/LayoutScene.h"
#include "qevent.h"

WidgetHierarchyDockWidget::WidgetHierarchyDockWidget(LayoutVisualMode& visualMode)
    : QDockWidget(&visualMode)
    , ui(new Ui::WidgetHierarchyDockWidget)
    , _visualMode(visualMode) // Parent may change so we store the mode pointer explicitly
{
    ui->setupUi(this);
    ui->treeView->setModel(new WidgetHierarchyTreeModel(visualMode));
}

WidgetHierarchyDockWidget::~WidgetHierarchyDockWidget()
{
    delete ui;
}

void WidgetHierarchyDockWidget::setupContextMenu()
{
    ui->treeView->setupContextMenu();
}

// Sets the widget manipulator that is at the root of our observed hierarchy.
void WidgetHierarchyDockWidget::setRootWidgetManipulator(LayoutManipulator* root)
{
    _rootWidgetManipulator = root;
    refresh();
    ui->treeView->expandToDepth(0);
}

// Refreshes the entire hierarchy completely from scratch. Also synchronize selection.
void WidgetHierarchyDockWidget::refresh()
{
    static_cast<WidgetHierarchyTreeModel*>(ui->treeView->model())->setRootManipulator(_rootWidgetManipulator);
    _visualMode.getScene()->onSelectionChanged();
}

QTreeView*WidgetHierarchyDockWidget::getTreeView() const
{
    return ui->treeView;
}

void WidgetHierarchyDockWidget::ignoreSelectionChangesInScene(bool ignore)
{
    _visualMode.getScene()->ignoreSelectionChanges(ignore);
}

void WidgetHierarchyDockWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete)
    {
        emit deleteRequested();
        return;
    }

    return QDockWidget::keyReleaseEvent(event);
}
