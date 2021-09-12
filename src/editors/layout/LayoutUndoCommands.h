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

    struct Record
    {
        QString path;
        size_t indexInParent;
        QByteArray data;
    };

    LayoutVisualMode& _visualMode;
    std::vector<Record> _records;
};

// This command creates one widget
class LayoutCreateCommand : public QUndoCommand
{
public:

    LayoutCreateCommand(LayoutVisualMode& visualMode, const QString& parentPath, const QString& type,
                        size_t indexInParent = std::numeric_limits<size_t>().max());
    LayoutCreateCommand(LayoutVisualMode& visualMode, const QString& parentPath, const QString& type,
                        QPointF scenePos, size_t indexInParent = std::numeric_limits<size_t>().max());

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 4; }

protected:

    void generateName();

    LayoutVisualMode& _visualMode;
    QString _fullPath; // Filled after widget creation. Sometimes it is not equal to _parentPath + _name!
    QString _parentPath;
    QString _type;
    QString _name;
    QPointF _scenePos;
    size_t _indexInParent;
    bool _useScenePos = false;
};

// This command changes a property of a widget
class LayoutPropertyEditCommand : public QUndoCommand
{
public:

    struct Record
    {
        QString path;
        CEGUI::String oldValue;
        CEGUI::String newValue;
    };

    LayoutPropertyEditCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records, const QString& propertyName, size_t multiChangeId);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 5; }
    virtual bool mergeWith(const QUndoCommand* other) override;

    bool isValueInvalid() const { return _invalidValue; }

protected:

    void setProperty(const QString& widgetPath, const CEGUI::String& value, const QStringList& propertiesToUpdate);
    void fillInfluencedPropertyList(QStringList& list);
    void refreshText();

    LayoutVisualMode& _visualMode;
    std::vector<Record> _records;
    CEGUI::String _propertyName;
    size_t _multiChangeId;
    bool _invalidValue; // Becomes known after the first redo()
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

// This command changes the parent of given windows and their positions in it.
// Only one new parent can be specified due to complexity of the logic.
// NB: we don't merge these commands
class LayoutMoveInHierarchyCommand : public QUndoCommand
{
public:

    struct Record
    {
        QString oldParentPath;
        size_t oldChildIndex;
        size_t newChildIndex;

        // For the case when the new parent contains a child with the same name
        QString oldName;
        QString newName;

        // Layout container may change these values of widgets parented into it
        CEGUI::UVector2 oldPos;
        CEGUI::USize oldSize;
    };

    LayoutMoveInHierarchyCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records, const QString& newParentPath);

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

// TThis command changes the name of the given widget
// NB: we don't merge these commands
class LayoutRenameCommand : public QUndoCommand
{
public:

    LayoutRenameCommand(LayoutVisualMode& visualMode, const QString& path, const QString& newName);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 14; }

protected:

    void refreshText();

    LayoutVisualMode& _visualMode;
    QString _parentPath;
    QString _oldName;
    QString _newName;
};

class MoveInParentWidgetListCommand : public QUndoCommand
{
public:

    MoveInParentWidgetListCommand(LayoutVisualMode& visualMode, QStringList&& paths, int delta);

    virtual void undo() override;
    virtual void redo() override;
    virtual int id() const override { return LayoutUndoCommandBase + 17; }
    virtual bool mergeWith(const QUndoCommand* other) override;

protected:

    void refreshText();

    LayoutVisualMode& _visualMode;
    QStringList _paths;
    int _delta = 0;
};

#endif // LAYOUTUNDOCOMMANDS_H
