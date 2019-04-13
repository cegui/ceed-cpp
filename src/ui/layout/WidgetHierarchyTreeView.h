#ifndef WIDGETHIERARCHYTREEVIEW_H
#define WIDGETHIERARCHYTREEVIEW_H

#include "qtreeview.h"

// The actual widget hierarchy tree widget - what a horrible name
// This is a Qt widget that does exactly the same as QTreeWidget for now,
// it is a placeholder that will be put to use once the need arises - and it will.

class WidgetHierarchyTreeView : public QTreeView
{
public:

    WidgetHierarchyTreeView(QWidget* parent = nullptr);

    void setupContextMenu();

protected:

    virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;

    QMenu* contextMenu = nullptr;
};

#endif // WIDGETHIERARCHYTREEVIEW_H
