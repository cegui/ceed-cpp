#ifndef WIDGETHIERARCHYTREEVIEW_H
#define WIDGETHIERARCHYTREEVIEW_H

#include "qtreeview.h"

// The actual widget hierarchy tree widget - what a horrible name
// This is a Qt widget that does exactly the same as QTreeWidget for now,
// it is a placeholder that will be put to use once the need arises - and it will.

class LayoutManipulator;

class WidgetHierarchyTreeView : public QTreeView
{
    Q_OBJECT;

public:

    WidgetHierarchyTreeView(QWidget* parent = nullptr);
    virtual ~WidgetHierarchyTreeView() override;

    void setupContextMenu();

public slots:

    void copySelectedWidgetPaths();
    void editSelectedWidgetName();

protected:

    LayoutManipulator* getManipulatorFromIndex(const QModelIndex& index) const;
    void setSelectedWidgetsLocked(bool locked, bool recursive);

    virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;

    QMenu* contextMenu = nullptr;
    QAction* actionCopyWidgetPath = nullptr;
    QAction* actionRename = nullptr;
    QAction* actionLockWidget = nullptr;
    QAction* actionUnlockWidget = nullptr;
    QAction* actionLockWidgetRecursively = nullptr;
    QAction* actionUnlockWidgetRecursively = nullptr;
};

#endif // WIDGETHIERARCHYTREEVIEW_H
