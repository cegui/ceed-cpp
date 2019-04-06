#include "src/ui/layout/WidgetHierarchyTreeModel.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "qmessagebox.h"
#include "qmimedata.h"

WidgetHierarchyTreeModel::WidgetHierarchyTreeModel(WidgetHierarchyDockWidget* dockWidget)
    : _dockWidget(dockWidget)
{
    setSortRole(Qt::UserRole + 1);
    setItemPrototype(new WidgetHierarchyItem(nullptr));
}

bool WidgetHierarchyTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    // Handle widget renaming
    if (role == Qt::EditRole)
    {
        auto item = static_cast<WidgetHierarchyItem*>(itemFromIndex(index));
        QString newName = value.toString();

        if (newName == item->getManipulator()->getWidgetName()) return false;

        // Validate the new name, cancel if invalid
        newName = LayoutManipulator::getValidWidgetName(newName);
        if (newName.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("The name was not changed because the new name is invalid.");
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
            return false;
        }

        // Check if the new name is unique in the parent, cancel if not
        /*
            parentWidget = item.manipulator.widget.getParent()
            if parentWidget is not None and parentWidget.isChild(value):
                msgBox = QtGui.QMessageBox()
                msgBox.setText("The name was not changed because the new name is in use by a sibling widget.")
                msgBox.setIcon(QtGui.QMessageBox.Warning)
                msgBox.exec_()
                return False

            // The name is good, apply it
            cmd = undo.RenameCommand(self.dockWidget.visual, item.manipulator.widget.getNamePath(), value)
            self.dockWidget.visual.tabbedEditor.undoStack.push(cmd)
        */

        // Return false because the undo command has changed the text of the item already
        return false;
    }

    QStandardItemModel::setData(index, value, role);
}

// If the selection contains children of something that is also selected, we don't include that
// (it doesn't make sense to move it anyways, it will be moved with its parent)
// DFS, Qt doesn't have helper methods for this it seems to me :-/
static bool isChild(QStandardItem* parent, QStandardItem* potentialChild)
{
    int i = 0;
    while (i < parent->rowCount())
    {
        auto child = parent->child(i);
        if (child == potentialChild || isChild(child, potentialChild)) return true;
        ++i;
    }

    return false;
}

QMimeData* WidgetHierarchyTreeModel::mimeData(const QModelIndexList& indexes) const
{
    std::vector<QStandardItem*> topItems;
    for (const auto& index : indexes)
    {
        auto item = itemFromIndex(index);

        bool hasParent = false;
        for (const auto& parentIndex : indexes)
        {
            if (parentIndex == index) continue;

            auto potentialParent = itemFromIndex(parentIndex);

            //???potentialParent here, potentialChild in isChild? was in original CEED.
            if (isChild(item, potentialParent))
            {
                hasParent = true;
                break;
            }
        }

        if (!hasParent) topItems.push_back(item);
    }

    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    for (auto item : topItems)
    {
        stream << item->data(Qt::UserRole).toString();
    }

    QMimeData* ret = new QMimeData();
    ret->setData("application/x-ceed-widget-paths", bytes);
    return ret;
}

QStringList WidgetHierarchyTreeModel::mimeTypes() const
{
    return { "application/x-ceed-widget-paths", "application/x-ceed-widget-type" };
}

bool WidgetHierarchyTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
/*

    def dropMimeData(self, data, action, row, column, parent):
        if data.hasFormat("application/x-ceed-widget-paths"):
            # data.data(..).data() looks weird but is the correct thing!
            widgetPaths = cPickle.loads(data.data("application/x-ceed-widget-paths").data())
            targetWidgetPaths = []

            newParent = self.itemFromIndex(parent)
            if newParent is None:
                return False

            newParentManipulator = self.dockWidget.visual.scene.getManipulatorByPath(newParent.data(QtCore.Qt.UserRole))

            usedNames = set()
            for widgetPath in widgetPaths:
                oldWidgetName = widgetPath[widgetPath.rfind("/") + 1:]
                # Prevent name clashes at the new parent
                # When a name clash occurs, we suggest a new name to the user and
                # ask them to confirm it/enter their own.
                # The tricky part is that we have to consider the other widget renames
                # too (in case we're reparenting and renaming more than one widget)
                # and we must prevent invalid names (i.e. containing "/")
                suggestedName = oldWidgetName
                while True:
                    # get a name that's not used in the new parent, trying to keep
                    # the suggested name (which is the same as the old widget name at
                    # the beginning)
                    tempName = newParentManipulator.getUniqueChildWidgetName(suggestedName)
                    # if the name we got is the same as the one we wanted...
                    if tempName == suggestedName:
                        # ...we need to check our own usedNames list too, in case
                        # another widget we're reparenting has got this name.
                        counter = 2
                        while suggestedName in usedNames:
                            # When this happens, we simply add a numeric suffix to
                            # the suggested name. The result could theoretically
                            # collide in the new parent but it's OK because this
                            # is just a suggestion and will be checked again when
                            # the 'while' loops.
                            suggestedName = tempName + str(counter)
                            counter += 1
                            error = "Widget name is in use by another widget being " + ("copied" if action == QtCore.Qt.CopyAction else "moved")

                        # if we had no collision, we can keep this name!
                        if counter == 2:
                            break
                    else:
                        # The new parent had a child with that name already and so
                        # it gave us a new suggested name.
                        suggestedName = tempName
                        error = "Widget name already exists in the new parent"

                    # Ask the user to confirm our suggested name or enter a new one
                    # We do this in a loop because we validate the user input
                    while True:
                        suggestedName, ok = QtGui.QInputDialog.getText(
                                            self.dockWidget,
                                            error,
                                            "New name for '" + oldWidgetName + "':",
                                            QtGui.QLineEdit.Normal,
                                            suggestedName)
                        # Abort everything if the user cancels the dialog
                        if not ok:
                            return False
                        # Validate the entered name
                        suggestedName = widgethelpers.Manipulator.getValidWidgetName(suggestedName)
                        if suggestedName:
                            break
                        error = "Invalid name, please try again"

                usedNames.add(suggestedName)
                targetWidgetPaths.append(newParent.data(QtCore.Qt.UserRole) + "/" + suggestedName)

            if action == QtCore.Qt.MoveAction:
                cmd = undo.ReparentCommand(self.dockWidget.visual, widgetPaths, targetWidgetPaths)
                # FIXME: unreadable
                self.dockWidget.visual.tabbedEditor.undoStack.push(cmd)

                return True

            elif action == QtCore.Qt.CopyAction:
                # FIXME: TODO
                return False

        elif data.hasFormat("application/x-ceed-widget-type"):
            widgetType = data.data("application/x-ceed-widget-type").data()
            parentItem = self.itemFromIndex(parent)
            # if the drop was at empty space (parentItem is None) the parentItemPath
            # should be "" if no root item exists, otherwise the name of the root item
            parentItemPath = parentItem.data(QtCore.Qt.UserRole) if parentItem is not None else self.dockWidget.visual.scene.rootManipulator.widget.getName() if self.dockWidget.visual.scene.rootManipulator is not None else ""
            parentManipulator = self.dockWidget.visual.scene.getManipulatorByPath(parentItemPath) if parentItemPath else None
            uniqueName = parentManipulator.getUniqueChildWidgetName(widgetType.rsplit("/", 1)[-1]) if parentManipulator is not None else widgetType.rsplit("/", 1)[-1]

            cmd = undo.CreateCommand(self.dockWidget.visual, parentItemPath, widgetType, uniqueName)
            self.dockWidget.visual.tabbedEditor.undoStack.push(cmd)

            return True
*/
    return false;
}

void WidgetHierarchyTreeModel::setRootManipulator(LayoutManipulator* rootManipulator)
{
    if (!synchroniseSubtree(getRootHierarchyItem(), rootManipulator))
    {
        clear();
        if (rootManipulator) appendRow(constructSubtree(rootManipulator));
    }
}

WidgetHierarchyItem* WidgetHierarchyTreeModel::getRootHierarchyItem() const
{
    return rowCount() > 0 ? static_cast<WidgetHierarchyItem*>(item(0)) : nullptr;
}

// Attempts to synchronise subtree with given widget manipulator, returns false if impossible.
// recursive - If True the synchronisation will recurse, trying to unify child widget hierarchy
//             items with child manipulators (this is generally what you want to do).
bool WidgetHierarchyTreeModel::synchroniseSubtree(WidgetHierarchyItem* item, LayoutManipulator* manipulator, bool recursive)
{
    if (!item || !manipulator || item->getManipulator() != manipulator) return false;

    item->refreshPathData(false);

    if (recursive)
    {
        std::vector<LayoutManipulator*> manipulatorsToRecreate;
        manipulator->getChildLayoutManipulators(manipulatorsToRecreate, false);

        // We do NOT use range here because the rowCount might change while we are processing
        int i = 0;
        while (i < item->rowCount())
        {
            auto child = static_cast<WidgetHierarchyItem*>(item->child(i));

            auto it = std::find(manipulatorsToRecreate.begin(), manipulatorsToRecreate.end(), child->getManipulator());
            if (it != manipulatorsToRecreate.end() && synchroniseSubtree(child, child->getManipulator(), true))
            {
                manipulatorsToRecreate.erase(it);
                ++i;
            }
            else
            {
                item->removeRow(i);
            }
        }

        for (LayoutManipulator* childManipulator : manipulatorsToRecreate)
        {
            if (!childManipulator->shouldBeSkipped())
                item->appendRow(constructSubtree(childManipulator));
        }
    }

    item->refreshOrderingData(true, true);

    return true;
}

WidgetHierarchyItem*WidgetHierarchyTreeModel::constructSubtree(LayoutManipulator* manipulator)
{
    auto ret = new WidgetHierarchyItem(manipulator);

    for (QGraphicsItem* item : manipulator->childItems())
    {
        LayoutManipulator* childManipulator = dynamic_cast<LayoutManipulator*>(item);
        if (childManipulator && !childManipulator->shouldBeSkipped())
        {
            auto childSubtree = constructSubtree(childManipulator);
            ret->appendRow(childSubtree);
        }
    }

    return ret;
}

