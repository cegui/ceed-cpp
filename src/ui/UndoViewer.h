#ifndef UNDOVIEWER_H
#define UNDOVIEWER_H

#include <QDockWidget>

// A dockwidget able to view the entire undo history of given undo stack

class QUndoView;
class QUndoStack;

class UndoViewer : public QDockWidget
{
    Q_OBJECT

public:

    explicit UndoViewer(QWidget *parent = nullptr);

    void setUndoStack(QUndoStack* undoStack);

protected:

    QUndoView* view = nullptr;
};

#endif // UNDOVIEWER_H
