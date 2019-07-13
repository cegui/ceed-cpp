#include "src/ui/layout/WidgetHierarchyTreeModel.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/cegui/CEGUIUtils.h"
#include <CEGUI/widgets/GridLayoutContainer.h>
#include "qmimedata.h"
#include "qinputdialog.h"
#include <qmessagebox.h>
#include <unordered_set>

WidgetHierarchyTreeModel::WidgetHierarchyTreeModel(LayoutVisualMode& visualMode)
    : _visualMode(visualMode)
{
    setSortRole(Qt::UserRole + 1);
    setItemPrototype(new WidgetHierarchyItem(nullptr));
}

bool WidgetHierarchyTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    // Handle widget renaming
    if (role == Qt::EditRole)
    {
        QString newName = value.toString();
        static_cast<WidgetHierarchyItem*>(itemFromIndex(index))->getManipulator()->renameWidget(newName);

        // Return false because the undo command inside renameWidget() has changed the text
        // of the item already (if it was possible)
        return false;
    }

    return QStandardItemModel::setData(index, value, role);
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

bool WidgetHierarchyTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int /*row*/, int /*column*/, const QModelIndex& parent)
{
    if (data->hasFormat("application/x-ceed-widget-paths"))
    {
        QStringList widgetPaths;
        QByteArray bytes = data->data("application/x-ceed-widget-paths");
        QDataStream stream(&bytes, QIODevice::ReadOnly);
        while (!stream.atEnd())
        {
            QString name;
            stream >> name;
            widgetPaths.append(name);
        }

        if (widgetPaths.empty()) return false;

        auto newParent = itemFromIndex(parent);
        if (!newParent) return false;

        QStringList targetWidgetPaths;
        std::unordered_set<QString> usedNames;

        const QString newParentPath = newParent->data(Qt::UserRole).toString();
        auto newParentManipulator = _visualMode.getScene()->getManipulatorByPath(newParentPath);
        if (!newParentManipulator)
        {
            assert(false);
            return false;
        }

        auto glc = dynamic_cast<CEGUI::GridLayoutContainer*>(newParentManipulator->getWidget());
        if (glc && (!glc->getGridWidth() || !glc->getGridHeight()))
        {
            QMessageBox::warning(nullptr, "Can't reparent", "Grid layout container must have non-zero dimensions to accept children");
            return false;
        }

        std::vector<LayoutReparentCommand::Record> records;

        for (const QString& widgetPath : widgetPaths)
        {
            int sepPos = widgetPath.lastIndexOf('/');
            const QString oldWidgetName = widgetPath.mid(sepPos + 1);
            const QString oldWidgetParentPath = widgetPath.left(sepPos);

            // Prevent name clashes at the new parent
            // When a name clash occurs, we suggest a new name to the user and
            // ask them to confirm it/enter their own.
            // The tricky part is that we have to consider the other widget renames
            // too (in case we're reparenting and renaming more than one widget)
            // and we must prevent invalid names (i.e. containing "/")
            QString suggestedName = oldWidgetName;
            QString error;
            while (true)
            {
                // Get a name that's not used in the new parent, trying to keep
                // the suggested name (which is the same as the old widget name at
                // the beginning)
                QString tempName = CEGUIUtils::getUniqueChildWidgetName(*newParentManipulator->getWidget(), suggestedName);

                // If the name we got is the same as the one we wanted...
                if (tempName == suggestedName)
                {
                    // ...we need to check our own usedNames list too, in case
                    // another widget we're reparenting has got this name.
                    int counter = 2;
                    while (usedNames.find(suggestedName) != usedNames.end())
                    {
                        // When this happens, we simply add a numeric suffix to
                        // the suggested name. The result could theoretically
                        // collide in the new parent but it's OK because this
                        // is just a suggestion and will be checked again when
                        // the 'while' loops.
                        suggestedName = tempName + QString::number(counter);
                        ++counter;
                        error = QString("Widget name is in use by another widget being ") + ((action == Qt::CopyAction) ? "copied" : "moved");
                    }

                    // If we had no collision, we can keep this name!
                    if (counter == 2) break;
                 }
                 else
                 {
                     // The new parent had a child with that name already and so
                     // it gave us a new suggested name.
                     suggestedName = tempName;
                     error = "Widget name already exists in the new parent";
                 }

                 // Ask the user to confirm our suggested name or enter a new one.
                 // We do this in a loop because we validate the user input.
                 while (true)
                 {
                     bool ok = false;
                     suggestedName = QInputDialog::getText(
                                         &_visualMode,
                                         error,
                                         "New name for '" + oldWidgetName + "':",
                                         QLineEdit::Normal,
                                         suggestedName,
                                         &ok);

                     // Abort everything if the user cancels the dialog
                     if (!ok) return false;

                     // Validate the entered name
                     suggestedName = CEGUIUtils::getValidWidgetName(suggestedName);
                     if (!suggestedName.isEmpty()) break;
                     error = "Invalid name, please try again";
                 }
             }

            usedNames.insert(suggestedName);
            targetWidgetPaths.append(newParentPath + "/" + suggestedName);

            LayoutReparentCommand::Record rec;
            rec.oldName = oldWidgetName;
            rec.newName = suggestedName;
            rec.oldParentPath = oldWidgetParentPath;
            records.push_back(std::move(rec));
        }

        if (action == Qt::MoveAction)
        {
            _visualMode.getEditor().getUndoStack()->push(new LayoutReparentCommand(_visualMode, std::move(records), newParentPath));
            return true;
        }
        else if (action == Qt::CopyAction)
        {
            // FIXME: TODO
            assert(false && "NOT IMPLEMENTED!!!");
            return false;
        }
    }
    else if (data->hasFormat("application/x-ceed-widget-type"))
    {
        QString widgetType = data->data("application/x-ceed-widget-type").data();

        auto parentItem = itemFromIndex(parent);

        // If the drop was at empty space (parentItem is None) the parentItemPath
        // should be "" if no root item exists, otherwise the name of the root item
        auto rootManip = _visualMode.getScene()->getRootWidgetManipulator();
        const QString parentItemPath = parentItem ? parentItem->data(Qt::UserRole).toString() : (rootManip ? rootManip->getWidgetName() : "");
        LayoutManipulator* parentManipulator = parentItemPath.isEmpty() ? nullptr : _visualMode.getScene()->getManipulatorByPath(parentItemPath);

        auto glc = dynamic_cast<CEGUI::GridLayoutContainer*>(parentManipulator->getWidget());
        if (glc && (!glc->getGridWidth() || !glc->getGridHeight()))
        {
            QMessageBox::warning(nullptr, "Can't create child", "Grid layout container must have non-zero dimensions to accept children");
            return false;
        }

        QString uniqueName = widgetType.mid(widgetType.lastIndexOf('/') + 1);
        if (parentManipulator)
            uniqueName = CEGUIUtils::getUniqueChildWidgetName(*parentManipulator->getWidget(), uniqueName);

        _visualMode.getEditor().getUndoStack()->push(new LayoutCreateCommand(_visualMode, parentItemPath, widgetType, uniqueName, QPointF()));

        return true;
    }

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

WidgetHierarchyItem* WidgetHierarchyTreeModel::constructSubtree(LayoutManipulator* manipulator)
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

