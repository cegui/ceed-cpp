#include "src/ui/UndoViewer.h"
#include "qundoview.h"
#include "qboxlayout.h"

UndoViewer::UndoViewer(QWidget *parent) :
    QDockWidget(parent)
{
    setObjectName("Undo Viewer dock widget");
    setWindowTitle("Undo Viewer");

    view = new QUndoView();
    view->setCleanIcon(QIcon(":/icons/clean_undo_state.png"));

    auto contentsWidget = new QWidget();
    auto contentsLayout = new QVBoxLayout(contentsWidget);
    auto margins = contentsLayout->contentsMargins();
    margins.setTop(0);
    contentsLayout->setContentsMargins(margins);
    contentsLayout->addWidget(view);

    setWidget(contentsWidget);
}

void UndoViewer::setUndoStack(QUndoStack* undoStack)
{
    view->setStack(undoStack);

    // If stack is None this effectively disables the entire dock widget to improve UX
    setEnabled(!!undoStack);
}
