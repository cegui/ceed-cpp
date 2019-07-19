#include "src/ui/layout/WidgetHierarchyTreeModel.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/cegui/CEGUIUtils.h"
#include <qmimedata.h>
#include <CEGUI/Window.h>

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

QMimeData* WidgetHierarchyTreeModel::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.empty()) return nullptr;

    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);

    for (const auto& index : indexes)
        stream << data(index, Qt::UserRole).toString();

    QMimeData* ret = new QMimeData();
    ret->setData("application/x-ceed-widget-paths", bytes);
    return ret;
}

QStringList WidgetHierarchyTreeModel::mimeTypes() const
{
    return { "application/x-ceed-widget-paths", "application/x-ceed-widget-type" };
}

bool WidgetHierarchyTreeModel::dropMimeData(const QMimeData* mimeData, Qt::DropAction action, int row, int /*column*/, const QModelIndex& parent)
{
    size_t childIndex = (row > 0) ? static_cast<size_t>(data(index(row - 1, 0, parent), Qt::UserRole + 1).toULongLong()) + 1 : 0;

    if (mimeData->hasFormat("application/x-ceed-widget-paths"))
    {
        QStringList widgetPaths;
        QByteArray bytes = mimeData->data("application/x-ceed-widget-paths");
        QDataStream stream(&bytes, QIODevice::ReadOnly);
        while (!stream.atEnd())
        {
            QString name;
            stream >> name;
            widgetPaths.append(name);
        }

        const QString newParentPath = data(parent, Qt::UserRole).toString();
        auto newParentManipulator = _visualMode.getScene()->getManipulatorByPath(newParentPath);

        if (action == Qt::MoveAction)
        {
            return _visualMode.moveWidgetsInHierarchy(std::move(widgetPaths), newParentManipulator, childIndex);
        }
        else if (action == Qt::CopyAction)
        {
            // FIXME: TODO, may need another sorting / fixing than MoveAction (LayoutMoveInHierarchyCommand)
            assert(false && "NOT IMPLEMENTED!!!");
            return false;
        }
    }
    else if (mimeData->hasFormat("application/x-ceed-widget-type"))
    {
        QString widgetType = mimeData->data("application/x-ceed-widget-type").data();

        auto parentItem = itemFromIndex(parent);

        // If the drop was at empty space (parentItem is None) the parentItemPath
        // should be "" if no root item exists, otherwise the name of the root item
        auto rootManip = _visualMode.getScene()->getRootWidgetManipulator();
        const QString parentItemPath = parentItem ? parentItem->data(Qt::UserRole).toString() : (rootManip ? rootManip->getWidgetName() : "");
        LayoutManipulator* parentManipulator = parentItemPath.isEmpty() ? nullptr : _visualMode.getScene()->getManipulatorByPath(parentItemPath);

        if (!parentManipulator->canAcceptChildren(1, true)) return false;

        QString uniqueName = widgetType.mid(widgetType.lastIndexOf('/') + 1);
        if (parentManipulator)
            uniqueName = CEGUIUtils::getUniqueChildWidgetName(*parentManipulator->getWidget(), uniqueName);

        _visualMode.getEditor().getUndoStack()->push(new LayoutCreateCommand(_visualMode, parentItemPath, widgetType, uniqueName, QPointF(), childIndex));

        return true;
    }

    return false;
}

void WidgetHierarchyTreeModel::setRootManipulator(LayoutManipulator* rootManipulator)
{
    if (!rowCount() || !synchroniseSubtree(static_cast<WidgetHierarchyItem*>(item(0)), rootManipulator))
    {
        clear();
        if (rootManipulator) appendRow(constructSubtree(rootManipulator));
    }
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

