#ifndef LAYOUTMANIPULATOR_H
#define LAYOUTMANIPULATOR_H

#include "src/cegui/CEGUIManipulator.h"

class LayoutManipulator : public CEGUIManipulator
{
public:

    LayoutManipulator();

    // NB: in python was getWidget() and all fields requested directly, I need the same!
    QString getWidgetName() const;
    QString getWidgetType() const;
};

#endif // LAYOUTMANIPULATOR_H
