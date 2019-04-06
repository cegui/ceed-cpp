#ifndef CEGUIMANIPULATOR_H
#define CEGUIMANIPULATOR_H

#include "src/ui/ResizableRectItem.h"

// This is a rectangle that is synchronised with given CEGUI widget,
// it provides moving and resizing functionality

class CEGUIManipulator : public ResizableRectItem
{
public:

    CEGUIManipulator(QGraphicsItem* parent = nullptr);

    virtual void updateFromWidget(bool callUpdate = false, bool updateAncestorLCs = false);
    virtual void detach(bool detachWidget = true, bool destroyWidget = true, bool recursive = true);
};

#endif // CEGUIMANIPULATOR_H
