#include "src/ui/CEGUIGraphicsView.h"

CEGUIGraphicsView::CEGUIGraphicsView(QWidget *parent) :
    QGraphicsView(parent)
{
}

CEGUIGraphicsView::~CEGUIGraphicsView()
{
}

void CEGUIGraphicsView::injectInput(bool inject)
{
    _injectInput = inject;
}
