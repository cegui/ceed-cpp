#ifndef IMAGESETUNDOCOMMANDS_H
#define IMAGESETUNDOCOMMANDS_H

#include "qundostack.h"
//#include "qpoint.h"
#include "qrect.h"

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

    struct Record
    {
        QString name;
        QPointF oldPos;
        QPointF newPos;
    };

    ImagesetMoveCommand(ImagesetVisualMode& visualMode, std::vector<Record>&& imageRecords);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return ImagesetUndoCommandBase + 1; }
    virtual bool mergeWith(const QUndoCommand* other) override;
    void refreshText();

protected:

    qreal biggestDelta = 0.0;

    ImagesetVisualMode& _visualMode;
    std::vector<Record> _imageRecords;
};

// Changes geometry of given images, that means that positions as well as rects might change.
// Can even implement MoveCommand as a special case but would eat more RAM.
class ImagesetGeometryChangeCommand : public QUndoCommand
{
public:

    struct Record
    {
        QString name;
        QPointF oldPos;
        QRectF oldRect;
        QPointF newPos;
        QRectF newRect;
    };

    ImagesetGeometryChangeCommand(ImagesetVisualMode& visualMode, std::vector<Record>&& imageRecords);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return ImagesetUndoCommandBase + 2; }
    virtual bool mergeWith(const QUndoCommand* other) override;
    void refreshText();

protected:

    qreal biggestMoveDelta = 0.0;
    qreal biggestResizeDelta = 0.0;

    ImagesetVisualMode& _visualMode;
    std::vector<Record> _imageRecords;
};

// TODO: create base class for ImagesetMoveCommand and ImagesetOffsetMoveCommand, very similar code
class ImagesetOffsetMoveCommand : public QUndoCommand
{
public:

    struct Record
    {
        QString name;
        QPointF oldPos;
        QPointF newPos;
    };

    ImagesetOffsetMoveCommand(ImagesetVisualMode& visualMode, std::vector<Record>&& imageRecords);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return ImagesetUndoCommandBase + 3; }
    virtual bool mergeWith(const QUndoCommand* other) override;
    void refreshText();

protected:

    qreal biggestDelta = 0.0;

    ImagesetVisualMode& _visualMode;
    std::vector<Record> _imageRecords;
};

// Changes name of one image (always just one image!)
class ImageRenameCommand : public QUndoCommand
{
public:

    ImageRenameCommand(ImagesetVisualMode& visualMode, const QString& oldName, const QString& newName);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return ImagesetUndoCommandBase + 4; }
    virtual bool mergeWith(const QUndoCommand* other) override;

protected:

    ImagesetVisualMode& _visualMode;
    QString _oldName;
    QString _newName;
};

#endif // IMAGESETUNDOCOMMANDS_H
