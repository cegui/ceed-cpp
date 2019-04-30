#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/ui/layout/LayoutManipulator.h"
#include <CEGUI/Window.h>

WidgetHierarchyItem::WidgetHierarchyItem(LayoutManipulator* manipulator)
    : QStandardItem(manipulator ? manipulator->getWidgetName() : "<No widget>")
    , _manipulator(manipulator)
{
    if (manipulator)
    {
        setToolTip("type: " + manipulator->getWidgetType());

        // Interlink them so we can react on selection changes
        manipulator->setTreeItem(this);
    }

    refreshPathData(false);
    refreshOrderingData(false, false);

    setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable |
             Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable);

    setData(Qt::Unchecked, Qt::CheckStateRole);
}

QStandardItem* WidgetHierarchyItem::clone() const
{
    auto ret = new WidgetHierarchyItem(_manipulator);
    ret->setData(data(Qt::CheckStateRole), Qt::CheckStateRole);
    return ret;
}

void WidgetHierarchyItem::setData(const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole && _manipulator)
    {
        // Synchronise the manipulator with the lock state
        _manipulator->setLocked(value == Qt::Checked);
    }

    return QStandardItem::setData(value, role);
}

//  TODO: Move this to CEGUI::Window
int WidgetHierarchyItem::getWidgetIdxInParent() const
{
    auto widget = _manipulator->getWidget();
    if (!widget) return -1;

    auto parent = widget->getParent();
    if (!parent) return 0;

    for (size_t i = 0; i < parent->getChildCount(); ++i)
        if (parent->getChildAtIdx(i) == widget)
            return static_cast<int>(i);

    return -1;
}

// Updates the stored path data for the item and its children
// NOTE: We use widget path here because that's what QVariant can serialise and pass forth
//       I have had weird segfaults when storing manipulator directly here, perhaps they
//       are related to PySide, perhaps they were caused by my stupidity, we will never know!
void WidgetHierarchyItem::refreshPathData(bool recursive)
{
    if (!_manipulator) return;

    setText(_manipulator->getWidgetName());
    setData(_manipulator->getWidgetPath(), Qt::UserRole);

    if (recursive)
    {
        for (int i = 0; i < rowCount(); ++i)
        {
            static_cast<WidgetHierarchyItem*>(child(i))->refreshPathData(true);
        }
    }
}

// Updates the stored ordering data for the item and its children
// if resort is True the children are sorted according to their order in CEGUI::Window
void WidgetHierarchyItem::refreshOrderingData(bool resort, bool recursive)
{
    // resort=True with recursive=False makes no sense and is a bug
    assert(!resort || recursive);

    if (!_manipulator) return;

    setData(getWidgetIdxInParent(), Qt::UserRole + 1);

    if (recursive)
    {
        for (int i = 0; i < rowCount(); ++i)
        {
            // We pass resort=False because sortChildren is recursive itself
            static_cast<WidgetHierarchyItem*>(child(i))->refreshOrderingData(false, true);
        }
    }

    if (resort) sortChildren(0);
}

// Locks or unlocks this item.
// locked - if True this item gets locked = user won't be able to move it
//          in the visual editing mode.
// recursive - if True, all children of this item will also get affected
//             They will get locked or unlocked depending on the "locked"
//             argument, independent of their previous lock state.
void WidgetHierarchyItem::setLocked(bool locked, bool recursive)
{
    // We do it this way around to make sure the checkbox's check state is always up to date
    setData(locked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);

    if (recursive)
    {
        for (int i = 0; i < rowCount(); ++i)
        {
            static_cast<WidgetHierarchyItem*>(child(i))->setLocked(locked, true);
        }
    }
}
