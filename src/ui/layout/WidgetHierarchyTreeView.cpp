#include "src/ui/layout/WidgetHierarchyTreeView.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/util/ConfigurableAction.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/Application.h"
#include "qmenu.h"
#include "qevent.h"
#include "qstandarditemmodel.h"
#include "qclipboard.h"

WidgetHierarchyTreeView::WidgetHierarchyTreeView(QWidget* parent)
    : QTreeView(parent)
{
}

void WidgetHierarchyTreeView::setupContextMenu()
{
    setContextMenuPolicy(Qt::DefaultContextMenu);

    Application* app = qobject_cast<Application*>(qApp);

    auto&& settings = app->getSettings();
    auto category = settings->getCategory("shortcuts");
    if (!category) category = settings->createCategory("shortcuts", "Shortcuts");
    auto section = category->createSection("layout", "Layout Editor");

    actionCopyWidgetPath = new ConfigurableAction(this,
                                         *section, "copy_widget_path", "C&opy Widget Paths",
                                         "Copies the 'NamePath' properties of the selected widgets to the clipboard.",
                                         QIcon(":/icons/actions/copy.png"));
    connect(actionCopyWidgetPath, &ConfigurableAction::triggered, this, &WidgetHierarchyTreeView::copySelectedWidgetPaths);

    actionRename = new ConfigurableAction(this,
                                         *section, "rename", "&Rename Widget",
                                         "Edits the selected widget's name.",
                                         QIcon(":/icons/layout_editing/rename.png"));
    connect(actionRename, &ConfigurableAction::triggered, this, &WidgetHierarchyTreeView::editSelectedWidgetName);

    actionLockWidget = new ConfigurableAction(this,
                                         *section, "lock_widget", "&Lock Widget",
                                         "Locks the widget for moving and resizing in the visual editing mode.",
                                         QIcon(":/icons/layout_editing/lock_widget.png"));
    connect(actionLockWidget, &ConfigurableAction::triggered, [this]() { setSelectedWidgetsLocked(true, false); });

    actionUnlockWidget = new ConfigurableAction(this,
                                         *section, "unlock_widget", "&Unlock Widget",
                                         "Unlocks the widget for moving and resizing in the visual editing mode.",
                                         QIcon(":/icons/layout_editing/unlock_widget.png"));
    connect(actionUnlockWidget, &ConfigurableAction::triggered, this, [this]() { setSelectedWidgetsLocked(false, false); });

    actionLockWidgetRecursively = new ConfigurableAction(this,
                                         *section, "recursively_lock_widget", "&Lock Widget (recursively)",
                                         "Locks the widget and all its child widgets for moving and resizing in the visual editing mode.",
                                         QIcon(":/icons/layout_editing/lock_widget_recursively.png"));
    connect(actionLockWidgetRecursively, &ConfigurableAction::triggered, this, [this]() { setSelectedWidgetsLocked(true, true); });

    actionUnlockWidgetRecursively = new ConfigurableAction(this,
                                         *section, "recursively_unlock_widget", "&Unlock Widget (recursively)",
                                         "Unlocks the widget and all its child widgets for moving and resizing in the visual editing mode.",
                                         QIcon(":/icons/layout_editing/unlock_widget_recursively.png"));
    connect(actionUnlockWidgetRecursively, &ConfigurableAction::triggered, this, [this]() { setSelectedWidgetsLocked(false, true); });

    contextMenu = new QMenu(this);
    contextMenu->addAction(actionRename);
    contextMenu->addSeparator();
    contextMenu->addAction(actionLockWidget);
    contextMenu->addAction(actionUnlockWidget);
    contextMenu->addAction(actionLockWidgetRecursively);
    contextMenu->addAction(actionUnlockWidgetRecursively);
    contextMenu->addSeparator();
/*
        self.cutAction = action.getAction("all_editors/cut")
        self.contextMenu.addAction(self.cutAction)
        self.copyAction = action.getAction("all_editors/copy")
        self.contextMenu.addAction(self.copyAction)
        self.pasteAction = action.getAction("all_editors/paste")
        self.contextMenu.addAction(self.pasteAction)
        self.deleteAction = action.getAction("all_editors/delete")
        self.contextMenu.addAction(self.deleteAction)
*/
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

// Synchronizes tree selection with scene selection
void WidgetHierarchyTreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    // We are running synchronization the other way, this prevents infinite loops and recursion
    WidgetHierarchyDockWidget* widget = dynamic_cast<WidgetHierarchyDockWidget*>(parentWidget()->parentWidget());
    if (!widget || widget->isIgnoringSelectionChanges()) return;

    widget->ignoreSelectionChangesInScene(true);

    for (auto& index : selected.indexes())
    {
        auto item = static_cast<QStandardItemModel*>(model())->itemFromIndex(index);

        auto widgetItem = dynamic_cast<WidgetHierarchyItem*>(item);
        if (!widgetItem) continue;

        QString manipulatorPath = widgetItem->data(Qt::UserRole).toString();
        LayoutManipulator* manipulator = nullptr;
        if (!manipulatorPath.isEmpty())
            manipulator = widget->getVisualMode()->getScene()->getManipulatorByPath(manipulatorPath);

        if (manipulator) manipulator->setSelected(true);
    }

    for (auto& index : deselected.indexes())
    {
        auto item = static_cast<QStandardItemModel*>(model())->itemFromIndex(index);

        auto widgetItem = dynamic_cast<WidgetHierarchyItem*>(item);
        if (!widgetItem) continue;

        QString manipulatorPath = widgetItem->data(Qt::UserRole).toString();
        LayoutManipulator* manipulator = nullptr;
        if (!manipulatorPath.isEmpty())
            manipulator = widget->getVisualMode()->getScene()->getManipulatorByPath(manipulatorPath);

        if (manipulator) manipulator->setSelected(false);
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
    // We don't touch the cut/copy/paste actions because they're shared
    // among all editors and disabling them here would disable them
    // for the other editors too.
    const bool hasSelection = !selectedIndexes().empty();
    actionCopyWidgetPath->setEnabled(hasSelection);
    actionRename->setEnabled(hasSelection);
    actionLockWidget->setEnabled(hasSelection);
    actionUnlockWidget->setEnabled(hasSelection);
    actionLockWidgetRecursively->setEnabled(hasSelection);
    actionUnlockWidgetRecursively->setEnabled(hasSelection);
/*
        self.deleteAction.setEnabled(haveSel)
*/
    contextMenu->exec(event->globalPos());
}
