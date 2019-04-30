#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "ui_WidgetHierarchyDockWidget.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/ui/layout/WidgetHierarchyTreeModel.h"
#include "src/ui/layout/LayoutScene.h"
#include "qevent.h"

WidgetHierarchyDockWidget::WidgetHierarchyDockWidget(LayoutVisualMode& visualMode) :
    QDockWidget(&visualMode),
    ui(new Ui::WidgetHierarchyDockWidget)
{
    ui->setupUi(this);

    auto model = new WidgetHierarchyTreeModel(visualMode);
    ui->treeView->setModel(model);
}

WidgetHierarchyDockWidget::~WidgetHierarchyDockWidget()
{
    delete ui;
}

void WidgetHierarchyDockWidget::setupContextMenu()
{
    ui->treeView->setupContextMenu();
}

LayoutVisualMode* WidgetHierarchyDockWidget::getVisualMode() const
{
    return static_cast<LayoutVisualMode*>(parentWidget());
}

// Sets the widget manipulator that is at the root of our observed hierarchy.
// Uses getTreeItemForManipulator to recursively populate the tree.
void WidgetHierarchyDockWidget::setRootWidgetManipulator(LayoutManipulator* root)
{
    _rootWidgetManipulator = root;
    static_cast<WidgetHierarchyTreeModel*>(ui->treeView->model())->setRootManipulator(root);
    ui->treeView->expandToDepth(0);
}

// Refreshes the entire hierarchy completely from scratch
void WidgetHierarchyDockWidget::refresh()
{
    // This will resynchronise the entire model
    static_cast<WidgetHierarchyTreeModel*>(ui->treeView->model())->setRootManipulator(_rootWidgetManipulator);
}

QTreeView*WidgetHierarchyDockWidget::getTreeView() const
{
    return ui->treeView;
}

void WidgetHierarchyDockWidget::ignoreSelectionChangesInScene(bool ignore)
{
    static_cast<LayoutVisualMode*>(parentWidget())->getScene()->ignoreSelectionChanges(ignore);
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
