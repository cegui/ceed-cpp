#ifndef WIDGETHIERARCHYTREEMODEL_H
#define WIDGETHIERARCHYTREEMODEL_H

#include "qstandarditemmodel.h"

class WidgetHierarchyDockWidget;

class WidgetHierarchyTreeModel : public QStandardItemModel
{
public:

    WidgetHierarchyTreeModel(WidgetHierarchyDockWidget* dockWidget);

protected:

    WidgetHierarchyDockWidget* _dockWidget = nullptr;
};

#endif // WIDGETHIERARCHYTREEMODEL_H
