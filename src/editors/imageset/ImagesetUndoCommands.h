#ifndef IMAGESETUNDOCOMMANDS_H
#define IMAGESETUNDOCOMMANDS_H

#include "qundostack.h"

constexpr int ImagesetUndoCommandBase = 1100;

// Undo commands in this file intentionally use Qt's primitives because
// it's easier to work with and avoids unnecessary conversions all the time

class ImagesetVisualMode;

// This command simply moves given images from old position to the new.
// You can use GeometryChangeCommand instead and use the same rects as old new as current rects,
// this is there just to save memory.
class ImagesetMoveCommand : public QUndoCommand
{
public:

    ImagesetMoveCommand(ImagesetVisualMode& visualMode, const QStringList& imageNames);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return ImagesetUndoCommandBase + 1; }
    virtual bool mergeWith(const QUndoCommand* other) override;
    void refreshText();

protected:

    ImagesetVisualMode& _visualMode;
    QStringList _imageNames;
};

#endif // IMAGESETUNDOCOMMANDS_H
