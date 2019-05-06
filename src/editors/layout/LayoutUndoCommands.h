#ifndef LAYOUTUNDOCOMMANDS_H
#define LAYOUTUNDOCOMMANDS_H

#include "qundostack.h"
#include "qvariant.h"
#include "qrect.h"
#include <CEGUI/String.h>
#include <CEGUI/UVector.h>
#include <CEGUI/USize.h>
#include <CEGUI/HorizontalAlignment.h>
#include <CEGUI/VerticalAlignment.h>

constexpr int LayoutUndoCommandBase = 1200;

class LayoutVisualMode;

// This command simply moves given widgets from old positions to new
class LayoutMoveCommand : public QUndoCommand
{
public:

    struct Record
    {
        QString path;
        CEGUI::UVector2 oldPos;
        CEGUI::UVector2 newPos;
    };

    LayoutMoveCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 1; }
    virtual bool mergeWith(const QUndoCommand* other) override;

protected:

    LayoutVisualMode& _visualMode;
    std::vector<Record> _records;
};

// This command resizes given widgets from old positions and old sizes to new
class LayoutResizeCommand : public QUndoCommand
{
public:

    struct Record
    {
        QString path;
        CEGUI::UVector2 oldPos;
        CEGUI::UVector2 newPos;
        CEGUI::USize oldSize;
        CEGUI::USize newSize;
    };

    LayoutResizeCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 2; }
    virtual bool mergeWith(const QUndoCommand* other) override;

protected:

    LayoutVisualMode& _visualMode;
    std::vector<Record> _records;
};

// This command deletes given widgets
class LayoutDeleteCommand : public QUndoCommand
{
public:

    LayoutDeleteCommand(LayoutVisualMode& visualMode, QStringList&& paths);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 3; }

protected:

    LayoutVisualMode& _visualMode;
    QStringList _paths;
    QByteArray _data;
};

// This command creates one widget
class LayoutCreateCommand : public QUndoCommand
{
public:

    LayoutCreateCommand(LayoutVisualMode& visualMode, const QString& parentPath, const QString& type, const QString& name);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 4; }

protected:

    LayoutVisualMode& _visualMode;
    QString _parentPath;
    QString _type;
    QString _name;
};

// This command resizes given widgets from old positions and old sizes to new
class LayoutPropertyEditCommand : public QUndoCommand
{
public:

    LayoutPropertyEditCommand(LayoutVisualMode& visualMode);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 5; }
    virtual bool mergeWith(const QUndoCommand* other) override;

protected:

    LayoutVisualMode& _visualMode;
    CEGUI::String _propertyName;
};

// This command aligns selected widgets accordingly
class LayoutHorizontalAlignCommand : public QUndoCommand
{
public:

    struct Record
    {
        QString path;
        CEGUI::HorizontalAlignment oldAlignment;
    };

    LayoutHorizontalAlignCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records, CEGUI::HorizontalAlignment newAlignment);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 6; }
    virtual bool mergeWith(const QUndoCommand* other) override;

protected:

    void refreshText();

    LayoutVisualMode& _visualMode;
    std::vector<Record> _records;
    CEGUI::HorizontalAlignment _newAlignment;
};

// This command aligns selected widgets accordingly
class LayoutVerticalAlignCommand : public QUndoCommand
{
public:

    struct Record
    {
        QString path;
        CEGUI::VerticalAlignment oldAlignment;
    };

    LayoutVerticalAlignCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records, CEGUI::VerticalAlignment newAlignment);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 7; }
    virtual bool mergeWith(const QUndoCommand* other) override;

protected:

    void refreshText();

    LayoutVisualMode& _visualMode;
    std::vector<Record> _records;
    CEGUI::VerticalAlignment _newAlignment;
};

// This command changes parent of given windows
// NB: we don't merge these commands
class LayoutReparentCommand : public QUndoCommand
{
public:

    struct Record
    {
        QString oldParentPath;
        QString oldName;
        QString newName;
    };

    LayoutReparentCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records, const QString& newParentPath);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 8; }

protected:

    void refreshText();

    LayoutVisualMode& _visualMode;
    std::vector<Record> _records;
    QString _newParentPath;
};

// This command pastes clipboard data to the given widget
class LayoutPasteCommand : public QUndoCommand
{
public:

    LayoutPasteCommand(LayoutVisualMode& visualMode, const QString& targetPath, QByteArray&& data);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 9; }

protected:

    LayoutVisualMode& _visualMode;
    QString _targetPath;
    QByteArray _data;
    std::vector<QString> _createdWidgets;
};

#endif // LAYOUTUNDOCOMMANDS_H
