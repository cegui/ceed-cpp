#ifndef LAYOUTUNDOCOMMANDS_H
#define LAYOUTUNDOCOMMANDS_H

#include "qundostack.h"
#include "qvariant.h"
#include "qrect.h"

constexpr int LayoutUndoCommandBase = 1200;

// This command pastes clipboard data to the given widget
class LayoutPasteCommand : public QUndoCommand
{
public:

    LayoutPasteCommand(/*LayoutVisualMode& visualMode, std::vector<Record>&& records*/);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 9; }

protected:

    //LayoutVisualMode& _visualMode;
    //std::vector<Record> _records;
};

#endif // LAYOUTUNDOCOMMANDS_H
