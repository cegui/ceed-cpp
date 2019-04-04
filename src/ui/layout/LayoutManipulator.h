#ifndef LAYOUTMANIPULATOR_H
#define LAYOUTMANIPULATOR_H

#include "src/cegui/CEGUIManipulator.h"

// Layout editing specific widget manipulator

class WidgetHierarchyItem;

class LayoutManipulator : public CEGUIManipulator
{
public:

    LayoutManipulator();
    virtual ~LayoutManipulator() override;

    //??? or in CEGUIManipulator?
    /*
    def shouldManipulatorBeSkipped(self, manipulator):
        return \
           manipulator.widget.isAutoWindow() and \
           settings.getEntry("layout/visual/hide_deadend_autowidgets").value and \
           not manipulator.hasNonAutoWidgetDescendants()
    */

    void setLocked(bool locked);
    void setTreeItem(WidgetHierarchyItem* treeItem) { _treeItem = treeItem; }

    // NB: in python was getWidget() and all fields requested directly, I need the same!
    QString getWidgetName() const;
    QString getWidgetType() const;

protected:

    WidgetHierarchyItem* _treeItem = nullptr;
    bool _showOutline = true;
};

#endif // LAYOUTMANIPULATOR_H
