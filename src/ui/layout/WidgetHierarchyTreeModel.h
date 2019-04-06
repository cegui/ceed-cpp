#ifndef WIDGETHIERARCHYTREEMODEL_H
#define WIDGETHIERARCHYTREEMODEL_H

#include "qstandarditemmodel.h"

class WidgetHierarchyDockWidget;
class WidgetHierarchyItem;
class LayoutManipulator;

class WidgetHierarchyTreeModel : public QStandardItemModel
{
public:

    WidgetHierarchyTreeModel(WidgetHierarchyDockWidget* dockWidget);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const override;
    virtual QStringList mimeTypes() const override;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

    void setRootManipulator(LayoutManipulator* rootManipulator);
    WidgetHierarchyItem* getRootHierarchyItem() const;

protected:

    bool synchroniseSubtree(WidgetHierarchyItem* item, LayoutManipulator* manipulator, bool recursive = true);
    WidgetHierarchyItem* constructSubtree(LayoutManipulator* manipulator);

    WidgetHierarchyDockWidget* _dockWidget = nullptr;
};

#endif // WIDGETHIERARCHYTREEMODEL_H
