#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "ui_WidgetHierarchyDockWidget.h"

WidgetHierarchyDockWidget::WidgetHierarchyDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::WidgetHierarchyDockWidget)
{
    ui->setupUi(this);

/*
        self.visual = visual

        self.ignoreSelectionChanges = False

        self.model = WidgetHierarchyTreeModel(self)
        self.treeView = self.findChild(WidgetHierarchyTreeView, "treeView")
        self.treeView.setModel(self.model)

        self.rootWidgetManipulator = None
*/
}

WidgetHierarchyDockWidget::~WidgetHierarchyDockWidget()
{
    delete ui;
}

/*

    def setRootWidgetManipulator(self, root):
        """Sets the widget manipulator that is at the root of our observed hierarchy.
        Uses getTreeItemForManipulator to recursively populate the tree.
        """

        self.rootWidgetManipulator = root
        self.model.setRootManipulator(root)
        self.treeView.expandToDepth(0)

    def refresh(self):
        """Refreshes the entire hierarchy completely from scratch"""

        # this will resynchronise the entire model
        self.model.setRootManipulator(self.rootWidgetManipulator)

    def keyReleaseEvent(self, event):
        if event.key() == QtCore.Qt.Key_Delete:
            handled = self.visual.scene.deleteSelectedWidgets()

            if handled:
                return True

        return super(HierarchyDockWidget, self).keyReleaseEvent(event)
*/
