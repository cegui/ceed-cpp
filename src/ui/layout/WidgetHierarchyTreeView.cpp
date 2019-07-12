#include "src/ui/layout/WidgetHierarchyTreeView.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/LayoutContainerHandle.h"
#include "src/ui/ResizingHandle.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/Application.h"
#include "qmenu.h"
#include "qevent.h"
#include "qstandarditemmodel.h"
#include "qclipboard.h"
#include <unordered_set>

WidgetHierarchyTreeView::WidgetHierarchyTreeView(QWidget* parent)
    : QTreeView(parent)
{
}

void WidgetHierarchyTreeView::setupContextMenu()
{
    setContextMenuPolicy(Qt::DefaultContextMenu);

    Application* app = qobject_cast<Application*>(qApp);

    actionCopyWidgetPath = app->getAction("layout/copy_widget_path");
    connect(actionCopyWidgetPath, &QAction::triggered, this, &WidgetHierarchyTreeView::copySelectedWidgetPaths);

    actionRename = app->getAction("layout/rename");
    connect(actionRename, &QAction::triggered, this, &WidgetHierarchyTreeView::editSelectedWidgetName);

    actionLockWidget = app->getAction("layout/lock_widget");
    connect(actionLockWidget, &QAction::triggered, [this]() { setSelectedWidgetsLocked(true, false); });

    actionUnlockWidget = app->getAction("layout/unlock_widget");
    connect(actionUnlockWidget, &QAction::triggered, this, [this]() { setSelectedWidgetsLocked(false, false); });

    actionLockWidgetRecursively = app->getAction("layout/recursively_lock_widget");
    connect(actionLockWidgetRecursively, &QAction::triggered, this, [this]() { setSelectedWidgetsLocked(true, true); });

    actionUnlockWidgetRecursively = app->getAction("layout/recursively_unlock_widget");
    connect(actionUnlockWidgetRecursively, &QAction::triggered, this, [this]() { setSelectedWidgetsLocked(false, true); });

    contextMenu = new QMenu(this);
    contextMenu->addAction(actionRename);
    contextMenu->addSeparator();
    contextMenu->addAction(actionLockWidget);
    contextMenu->addAction(actionUnlockWidget);
    contextMenu->addAction(actionLockWidgetRecursively);
    contextMenu->addAction(actionUnlockWidgetRecursively);
    contextMenu->addSeparator();
    contextMenu->addAction(app->getMainWindow()->getActionCut());
    contextMenu->addAction(app->getMainWindow()->getActionCopy());
    contextMenu->addAction(app->getMainWindow()->getActionPaste());
    contextMenu->addAction(app->getMainWindow()->getActionDeleteSelected());
    contextMenu->addSeparator();
    contextMenu->addAction(actionCopyWidgetPath);
}

void WidgetHierarchyTreeView::copySelectedWidgetPaths()
{
    auto selection = selectedIndexes();
    if (selection.empty()) return;

    QStringList paths;
    for (const auto& index : selection)
    {
        auto item = static_cast<WidgetHierarchyItem*>(static_cast<QStandardItemModel*>(model())->itemFromIndex(index));
        if (item->getManipulator())
            paths.append(item->getManipulator()->getWidgetPath());
    }

    if (paths.empty()) return;

    // Sort (otherwise the order is the item selection order)
    paths.sort();

    // FIXME: was OS line separator instead of \n
    QApplication::clipboard()->setText(paths.join("\n"));
}

void WidgetHierarchyTreeView::editSelectedWidgetName()
{
    auto selection = selectedIndexes();
    if (selection.empty()) return;

    setCurrentIndex(selection[0]);
    edit(selection[0]);
}

void WidgetHierarchyTreeView::setSelectedWidgetsLocked(bool locked, bool recursive)
{
    auto selection = selectedIndexes();
    if (selection.empty()) return;

    // It is possible that we will make superfluous lock actions if user selects widgets
    // in a hierarchy (parent & child) and then does a recursive lock. This doesn't do anything
    // harmful so we don't have any logic to prevent that.
    for (const auto& index : selection)
    {
        auto item = static_cast<WidgetHierarchyItem*>(static_cast<QStandardItemModel*>(model())->itemFromIndex(index));
        if (item->getManipulator())
            item->setLocked(locked, recursive);
    }
}

LayoutManipulator* WidgetHierarchyTreeView::getManipulatorFromIndex(const QModelIndex& index) const
{
    auto item = static_cast<QStandardItemModel*>(model())->itemFromIndex(index);

    auto widgetItem = dynamic_cast<WidgetHierarchyItem*>(item);
    if (!widgetItem) return nullptr;

    QString manipulatorPath = widgetItem->data(Qt::UserRole).toString();
    if (manipulatorPath.isEmpty()) return nullptr;

    WidgetHierarchyDockWidget* widget = static_cast<WidgetHierarchyDockWidget*>(parentWidget()->parentWidget());
    return widget->getVisualMode().getScene()->getManipulatorByPath(manipulatorPath);
}

// Synchronizes tree selection with scene selection
void WidgetHierarchyTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    // We are running synchronization the other way, this prevents infinite loops and recursion
    WidgetHierarchyDockWidget* widget = dynamic_cast<WidgetHierarchyDockWidget*>(parentWidget()->parentWidget());
    if (!widget || widget->isIgnoringSelectionChanges()) return;

    widget->ignoreSelectionChangesInScene(true);

    // We can't use incremental selected & deselected data because some manipulators may be
    // selected via their handles. We must explicitly deselect them, since reeView knows nothing
    // about handles and will not add their manipulators to 'deselected'.
    std::unordered_set<LayoutManipulator*> selectedManipulators;
    for (auto& index : selectionModel()->selectedIndexes())
    {
        auto manipulator = getManipulatorFromIndex(index);
        if (manipulator)
        {
            manipulator->setSelected(true);
            selectedManipulators.insert(manipulator);
        }
    }

    auto selection = widget->getVisualMode().getScene()->selectedItems();
    for (QGraphicsItem* item : selection)
    {
        auto manipulator = dynamic_cast<LayoutManipulator*>(item);
        if (!manipulator && (dynamic_cast<ResizingHandle*>(item) || dynamic_cast<LayoutContainerHandle*>(item)))
            manipulator = dynamic_cast<LayoutManipulator*>(item->parentItem());

        if (selectedManipulators.find(manipulator) == selectedManipulators.cend())
        {
            manipulator->setSelected(false);
            manipulator->deselectAllHandles();
        }
    }

    widget->ignoreSelectionChangesInScene(false);
}

void WidgetHierarchyTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    if (!contextMenu) return;

    // TODO: since these actions enabled state depends on the selection,
    // move the enabling/disabling to a central "selection changed" handler.
    // The handler should be called on tab activations too because
    // activating a tab changes the selection, effectively.
    // We don't touch the cut/copy/paste/delete actions because they're shared
    // among all editors and disabling them here would disable them
    // for the other editors too.
    const bool hasSelection = !selectedIndexes().empty();
    actionCopyWidgetPath->setEnabled(hasSelection);
    actionRename->setEnabled(hasSelection);
    actionLockWidget->setEnabled(hasSelection);
    actionUnlockWidget->setEnabled(hasSelection);
    actionLockWidgetRecursively->setEnabled(hasSelection);
    actionUnlockWidgetRecursively->setEnabled(hasSelection);
    contextMenu->exec(event->globalPos());
}
