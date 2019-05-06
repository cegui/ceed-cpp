#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/cegui/CEGUIUtils.h"
#include <CEGUI/widgets/SequentialLayoutContainer.h>
#include <CEGUI/WindowManager.h>
#include "qtreeview.h"

LayoutMoveCommand::LayoutMoveCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records)
    : _visualMode(visualMode)
    , _records(std::move(records))
{
    if (_records.size() == 1)
        setText(QString("Move '%1'").arg(_records[0].path));
    else
        setText(QString("Move %1 widgets'").arg(_records.size()));
}

void LayoutMoveCommand::undo()
{
    QUndoCommand::undo();

    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        assert(manipulator);
        manipulator->getWidget()->setPosition(rec.oldPos);
        manipulator->updateFromWidget(false, true);

        // In case the pixel position didn't change but the absolute and negative components changed and canceled each other out
        manipulator->update();

        manipulator->triggerPropertyManagerCallback({"Position", "Area"});
    }
}

void LayoutMoveCommand::redo()
{
    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        assert(manipulator);
        manipulator->getWidget()->setPosition(rec.newPos);
        manipulator->updateFromWidget(false, true);

        // In case the pixel position didn't change but the absolute and negative components changed and canceled each other out
        manipulator->update();

        manipulator->triggerPropertyManagerCallback({"Position", "Area"});
    }

    QUndoCommand::redo();
}

bool LayoutMoveCommand::mergeWith(const QUndoCommand* other)
{
    const LayoutMoveCommand* otherCmd = dynamic_cast<const LayoutMoveCommand*>(other);
    if (!otherCmd) return false;

    // It is nearly impossible to do the delta guesswork right, the parent might get resized etc,
    // it might be possible in this exact scenario (no resizes) but in the generic one it's a pain
    // and can't be done consistently, so I don't even try and just merge if the paths match

    if (_records.size() != otherCmd->_records.size()) return false;

    QStringList paths;
    for (const auto& rec : _records)
        paths.push_back(rec.path);

    for (const auto& rec : otherCmd->_records)
        if (!paths.contains(rec.path)) return false;

    // The same set of widgets, can merge

    for (auto& rec : _records)
    {
        const QString& path = rec.path;
        auto it = std::find_if(otherCmd->_records.begin(), otherCmd->_records.end(), [&path](const Record& otherRec)
        {
            return otherRec.path == path;
        });
        assert(it != otherCmd->_records.end());

        rec.newPos = it->newPos;
    }

    return true;
}

//---------------------------------------------------------------------

LayoutResizeCommand::LayoutResizeCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records)
    : _visualMode(visualMode)
    , _records(std::move(records))
{
    if (_records.size() == 1)
        setText(QString("Resize '%1'").arg(_records[0].path));
    else
        setText(QString("Resize %1 widgets'").arg(_records.size()));
}

void LayoutResizeCommand::undo()
{
    QUndoCommand::undo();

    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        assert(manipulator);
        manipulator->getWidget()->setPosition(rec.oldPos);
        manipulator->getWidget()->setSize(rec.oldSize);
        manipulator->updateFromWidget(false, true);

        // In case the pixel position didn't change but the absolute and negative components changed and canceled each other out
        manipulator->update();

        manipulator->triggerPropertyManagerCallback({"Size", "Position", "Area"});
    }
}

void LayoutResizeCommand::redo()
{
    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        assert(manipulator);
        manipulator->getWidget()->setPosition(rec.newPos);
        manipulator->getWidget()->setSize(rec.newSize);
        manipulator->updateFromWidget(false, true);

        // In case the pixel position didn't change but the absolute and negative components changed and canceled each other out
        manipulator->update();

        manipulator->triggerPropertyManagerCallback({"Size", "Position", "Area"});
    }

    QUndoCommand::redo();
}

bool LayoutResizeCommand::mergeWith(const QUndoCommand* other)
{
    const LayoutResizeCommand* otherCmd = dynamic_cast<const LayoutResizeCommand*>(other);
    if (!otherCmd) return false;

    // It is nearly impossible to do the delta guesswork right, the parent might get resized etc,
    // it might be possible in this exact scenario (no resizes) but in the generic one it's a pain
    // and can't be done consistently, so I don't even try and just merge if the paths match

    if (_records.size() != otherCmd->_records.size()) return false;

    QStringList paths;
    for (const auto& rec : _records)
        paths.push_back(rec.path);

    for (const auto& rec : otherCmd->_records)
        if (!paths.contains(rec.path)) return false;

    // The same set of widgets, can merge

    for (auto& rec : _records)
    {
        const QString& path = rec.path;
        auto it = std::find_if(otherCmd->_records.begin(), otherCmd->_records.end(), [&path](const Record& otherRec)
        {
            return otherRec.path == path;
        });
        assert(it != otherCmd->_records.end());

        rec.newPos = it->newPos;
        rec.newSize = it->newSize;
    }

    return true;
}

//---------------------------------------------------------------------

LayoutDeleteCommand::LayoutDeleteCommand(LayoutVisualMode& visualMode, QStringList&& paths)
    : _visualMode(visualMode)
{
    // Exclude child widgets of widgets being deleted from the explicit list
    while (!paths.empty())
    {
        QString currPath = paths.back();
        paths.pop_back();

        bool parentFound = false;
        for (const QString& potentialParentPath : paths)
        {
            if (currPath.startsWith(potentialParentPath + '/'))
            {
                parentFound = true;
                break;
            }
        }

        if (!parentFound && !_paths.contains(currPath))
            _paths.push_back(std::move(currPath));
    }

    // Serialize deleted hierarchies for undo
    QDataStream stream(&_data, QIODevice::WriteOnly);
    for (const QString& path : _paths)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(path);
        assert(manipulator);
        CEGUIUtils::serializeWidget(*manipulator->getWidget(), stream, true);
    }

    if (_paths.size() == 1)
        setText(QString("Delete '%1'").arg(_paths[0]));
    else
        setText(QString("Delete %1 widgets'").arg(_paths.size()));
}

void LayoutDeleteCommand::undo()
{
    QUndoCommand::undo();

    int i = 0;
    QDataStream stream(&_data, QIODevice::ReadOnly);
    while (!stream.atEnd())
    {
        //!!!TODO: test for root widget!
        const QString& path = _paths[i++];
        int sepPos = path.lastIndexOf('/');
        LayoutManipulator* parent = (sepPos < 0) ? nullptr :
                    _visualMode.getScene()->getManipulatorByPath(path.left(sepPos));

        CEGUI::Window* widget = CEGUIUtils::deserializeWidget(stream, parent->getWidget());
        LayoutManipulator* manipulator = parent ? parent->createChildManipulator(widget) : _visualMode.setRootWidget(widget);
        manipulator->setSelected(true);
    }

    _visualMode.getHierarchyDockWidget()->refresh();
}

void LayoutDeleteCommand::redo()
{
    for (const QString& path : _paths)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(path);
        assert(manipulator);
        manipulator->detach();
    }

    _visualMode.getHierarchyDockWidget()->refresh();

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

LayoutCreateCommand::LayoutCreateCommand(LayoutVisualMode& visualMode, const QString& parentPath, const QString& type, const QString& name)
    : _visualMode(visualMode)
    , _parentPath(parentPath)
    , _type(type)
    , _name(name)
{
    setText(QString("Create '%1' of type '%2'").arg(name, type));
}

void LayoutCreateCommand::undo()
{
    QUndoCommand::undo();

    const QString fullPath = _parentPath.isEmpty() ? _name : _parentPath + '/' + _name;
    auto manipulator = _visualMode.getScene()->getManipulatorByPath(fullPath);
    manipulator->detach();

    _visualMode.getHierarchyDockWidget()->refresh();
}

void LayoutCreateCommand::redo()
{
    CEGUI::Window* widget = CEGUI::WindowManager::getSingleton().createWindow(
                CEGUIUtils::qStringToString(_type), CEGUIUtils::qStringToString(_name));

    LayoutManipulator* parent = _parentPath.isEmpty() ? nullptr :
                _visualMode.getScene()->getManipulatorByPath(_parentPath);

    LayoutManipulator* manipulator = parent ? parent->createChildManipulator(widget) : _visualMode.setRootWidget(widget);

    // If the size is 0x0, the widget will be hard to deal with, lets fix that in that case
    if (widget->getSize() == CEGUI::USize(CEGUI::UDim(0.f, 0.f), CEGUI::UDim(0.f, 0.f)))
        widget->setSize(CEGUI::USize(CEGUI::UDim(0.f, 50.f), CEGUI::UDim(0.f, 50.f)));

    manipulator->updateFromWidget(true, true); //???need? called on creation

    // Ensure this isn't obscured by it's parent
    manipulator->moveToFront();

    _visualMode.getHierarchyDockWidget()->refresh();

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

LayoutPropertyEditCommand::LayoutPropertyEditCommand(LayoutVisualMode& visualMode)
    : _visualMode(visualMode)
{
/*
    def __init__(self, visual, propertyName, widgetPaths, oldValues, newValue, ignoreNextPropertyManagerCallback=False):

        self.propertyName = propertyName
        self.widgetPaths = widgetPaths
        self.oldValues = oldValues
        self.newValue = newValue

        if len(self.widgetPaths) == 1:
            self.setText("Change '%s' in '%s'" % (self.propertyName, self.widgetPaths[0]))
        else:
            self.setText("Change '%s' in %i widgets" % (self.propertyName, len(self.widgetPaths)))

        self.ignoreNextPropertyManagerCallback = ignoreNextPropertyManagerCallback
*/
}

void LayoutPropertyEditCommand::undo()
{
    QUndoCommand::undo();

    /*
        for widgetPath in self.widgetPaths:
            widgetManipulator = self.visual.scene.getManipulatorByPath(widgetPath)
            widgetManipulator.widget.setProperty(self.propertyName, self.oldValues[widgetPath])
            widgetManipulator.updateFromWidget(False, True)

            self.notifyPropertyManager(widgetManipulator, self.ignoreNextPropertyManagerCallback)
        self.ignoreNextPropertyManagerCallback = False

        # make sure to redraw the scene so the changes are visible
        self.visual.scene.update()
    */
}

void LayoutPropertyEditCommand::redo()
{
/*
        for widgetPath in self.widgetPaths:
            widgetManipulator = self.visual.scene.getManipulatorByPath(widgetPath)
            widgetManipulator.widget.setProperty(self.propertyName, self.newValue)
            widgetManipulator.updateFromWidget(False, True)

            self.notifyPropertyManager(widgetManipulator, self.ignoreNextPropertyManagerCallback)
        self.ignoreNextPropertyManagerCallback = False

        # make sure to redraw the scene so the changes are visible
        self.visual.scene.update()
*/

    QUndoCommand::redo();
}

bool LayoutPropertyEditCommand::mergeWith(const QUndoCommand* other)
{
    const LayoutPropertyEditCommand* otherCmd = dynamic_cast<const LayoutPropertyEditCommand*>(other);
    if (!otherCmd) return false;

    if (_propertyName != otherCmd->_propertyName) return false;

    /*
    if (_records.size() != otherCmd->_records.size()) return false;

    QStringList paths;
    for (const auto& rec : _records)
        paths.push_back(rec.path);

    for (const auto& rec : otherCmd->_records)
        if (!paths.contains(rec.path)) return false;
    */

    // The same set of widgets, can merge

    /*
            self.newValue = cmd.newValue
    */
    return true;
}

/*

    def notifyPropertyManager(self, widgetManipulator, ignoreTarget):
        if not ignoreTarget:
            widgetManipulator.triggerPropertyManagerCallback({self.propertyName})
        # some properties are related to others so that
        # when one changes, the others change too.
        # the following ensures that notifications are sent
        # about the related properties as well.
        related = None
        if self.propertyName == "Size":
            related = set([ "Area" ])
        elif self.propertyName == "Area":
            related = {"Position", "Size"}
        elif self.propertyName == "Position":
            related = set([ "Area" ])

        if related is not None:
            widgetManipulator.triggerPropertyManagerCallback(related)
*/

//---------------------------------------------------------------------

LayoutHorizontalAlignCommand::LayoutHorizontalAlignCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records, CEGUI::HorizontalAlignment newAlignment)
    : _visualMode(visualMode)
    , _records(std::move(records))
    , _newAlignment(newAlignment)
{
    refreshText();
}

void LayoutHorizontalAlignCommand::undo()
{
    QUndoCommand::undo();

    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        manipulator->getWidget()->setHorizontalAlignment(rec.oldAlignment);
        manipulator->updateFromWidget();

        manipulator->triggerPropertyManagerCallback({"HorizontalAlignment"});
    }
}

void LayoutHorizontalAlignCommand::redo()
{
    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        manipulator->getWidget()->setHorizontalAlignment(_newAlignment);
        manipulator->updateFromWidget();

        manipulator->triggerPropertyManagerCallback({"HorizontalAlignment"});
    }

    QUndoCommand::redo();
}

bool LayoutHorizontalAlignCommand::mergeWith(const QUndoCommand* other)
{
    const LayoutHorizontalAlignCommand* otherCmd = dynamic_cast<const LayoutHorizontalAlignCommand*>(other);
    if (!otherCmd) return false;

    if (_records.size() != otherCmd->_records.size()) return false;

    QStringList paths;
    for (const auto& rec : _records)
        paths.push_back(rec.path);

    for (const auto& rec : otherCmd->_records)
        if (!paths.contains(rec.path)) return false;

    // The same set of widgets, can merge

    _newAlignment = otherCmd->_newAlignment;
    refreshText();
    return true;
}

void LayoutHorizontalAlignCommand::refreshText()
{
    QString alignStr;
    switch (_newAlignment)
    {
        case CEGUI::HorizontalAlignment::Left: alignStr = "left"; break;
        case CEGUI::HorizontalAlignment::Centre: alignStr = "center"; break;
        case CEGUI::HorizontalAlignment::Right: alignStr = "right"; break;
    }

    if (_records.size() == 1)
        setText(QString("Horizontally align '%1' %2").arg(_records[0].path, alignStr));
    else
        setText(QString("Horizontally align %1 widgets %2").arg(_records.size()).arg(alignStr));
}

//---------------------------------------------------------------------

LayoutVerticalAlignCommand::LayoutVerticalAlignCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records, CEGUI::VerticalAlignment newAlignment)
    : _visualMode(visualMode)
    , _records(std::move(records))
    , _newAlignment(newAlignment)
{
    refreshText();
}

void LayoutVerticalAlignCommand::undo()
{
    QUndoCommand::undo();

    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        manipulator->getWidget()->setVerticalAlignment(rec.oldAlignment);
        manipulator->updateFromWidget();

        manipulator->triggerPropertyManagerCallback({"VerticalAlignment"});
    }
}

void LayoutVerticalAlignCommand::redo()
{
    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        manipulator->getWidget()->setVerticalAlignment(_newAlignment);
        manipulator->updateFromWidget();

        manipulator->triggerPropertyManagerCallback({"VerticalAlignment"});
    }

    QUndoCommand::redo();
}

bool LayoutVerticalAlignCommand::mergeWith(const QUndoCommand* other)
{
    const LayoutVerticalAlignCommand* otherCmd = dynamic_cast<const LayoutVerticalAlignCommand*>(other);
    if (!otherCmd) return false;

    if (_records.size() != otherCmd->_records.size()) return false;

    QStringList paths;
    for (const auto& rec : _records)
        paths.push_back(rec.path);

    for (const auto& rec : otherCmd->_records)
        if (!paths.contains(rec.path)) return false;

    // The same set of widgets, can merge

    _newAlignment = otherCmd->_newAlignment;
    refreshText();
    return true;
}

void LayoutVerticalAlignCommand::refreshText()
{
    QString alignStr;
    switch (_newAlignment)
    {
        case CEGUI::VerticalAlignment::Top: alignStr = "top"; break;
        case CEGUI::VerticalAlignment::Centre: alignStr = "center"; break;
        case CEGUI::VerticalAlignment::Bottom: alignStr = "bottom"; break;
    }

    if (_records.size() == 1)
        setText(QString("Vertically align '%1' %2").arg(_records[0].path, alignStr));
    else
        setText(QString("Vertically align %1 widgets %2").arg(_records.size()).arg(alignStr));
}

//---------------------------------------------------------------------

LayoutReparentCommand::LayoutReparentCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records, const QString& newParentPath)
    : _visualMode(visualMode)
    , _records(std::move(records))
    , _newParentPath(newParentPath)
{
    if (_records.size() == 1)
        setText(QString("Reparent '%1' to '%2'").arg(_records[0].oldName, _newParentPath));
    else
        setText(QString("Reparent %1 widgets").arg(_records.size()));
}

void LayoutReparentCommand::undo()
{
    QUndoCommand::undo();

    _visualMode.getScene()->clearSelection();
    _visualMode.getHierarchyDockWidget()->getTreeView()->clearSelection();

    for (const auto& rec : _records)
    {
        auto widgetManipulator = _visualMode.getScene()->getManipulatorByPath(_newParentPath + '/' + rec.newName);
        auto oldParentManipulator = _visualMode.getScene()->getManipulatorByPath(rec.oldParentPath);

        // Remove it from the current CEGUI parent widget
        auto parentWidget = widgetManipulator->getWidget()->getParent();
        if (parentWidget) parentWidget->removeChild(widgetManipulator->getWidget());

        // Rename it if necessary
        if (rec.oldName != rec.newName)
            widgetManipulator->getWidget()->setProperty("Name", CEGUIUtils::qStringToString(rec.oldName));

        // Add it to the old CEGUI parent widget
        oldParentManipulator->getWidget()->addChild(widgetManipulator->getWidget());

        // And sort out the manipulators
        widgetManipulator->setParentItem(oldParentManipulator);

        widgetManipulator->updateFromWidget(true, true);
    }

    _visualMode.getHierarchyDockWidget()->refresh();
}

void LayoutReparentCommand::redo()
{
    _visualMode.getScene()->clearSelection();
    _visualMode.getHierarchyDockWidget()->getTreeView()->clearSelection();

    for (const auto& rec : _records)
    {
        auto widgetManipulator = _visualMode.getScene()->getManipulatorByPath(rec.oldParentPath + '/' + rec.newName);
        auto newParentManipulator = _visualMode.getScene()->getManipulatorByPath(_newParentPath);

        // Remove it from the current CEGUI parent widget
        auto parentWidget = widgetManipulator->getWidget()->getParent();
        if (parentWidget) parentWidget->removeChild(widgetManipulator->getWidget());

        // Rename it if necessary
        if (rec.oldName != rec.newName)
            widgetManipulator->getWidget()->setProperty("Name", CEGUIUtils::qStringToString(rec.newName));

        // Add it to the new CEGUI parent widget
        newParentManipulator->getWidget()->addChild(widgetManipulator->getWidget());

        // And sort out the manipulators
        widgetManipulator->setParentItem(newParentManipulator);

        widgetManipulator->updateFromWidget(true, true);
    }

    _visualMode.getHierarchyDockWidget()->refresh();

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

LayoutPasteCommand::LayoutPasteCommand(LayoutVisualMode& visualMode,
                                       const QString& targetPath,
                                       QByteArray&& data)
    : _visualMode(visualMode)
    , _targetPath(targetPath)
    , _data(std::move(data))
{
}

void LayoutPasteCommand::undo()
{
    QUndoCommand::undo();

    LayoutScene* scene = _visualMode.getScene();
    for (const QString& path : _createdWidgets)
        scene->getManipulatorByPath(path)->detach();

    _visualMode.getHierarchyDockWidget()->refresh();

    _createdWidgets.clear();
}

void LayoutPasteCommand::redo()
{
    LayoutScene* scene = _visualMode.getScene();
    auto target = scene->getManipulatorByPath(_targetPath);
    if (!target) return;

    scene->clearSelection();

    QDataStream stream(&_data, QIODevice::ReadOnly);
    while (!stream.atEnd())
    {
        CEGUI::Window* widget = CEGUIUtils::deserializeWidget(stream, target->getWidget());
        LayoutManipulator* manipulator = target->createChildManipulator(widget);
        manipulator->setSelected(true);
        _createdWidgets.push_back(manipulator->getWidgetPath());
    }

    // Update the topmost parent widget recursively to get possible resize or
    // repositions of the pasted widgets into the manipulator data.
    target->updateFromWidget(true, true);

    _visualMode.getHierarchyDockWidget()->refresh();

    if (_createdWidgets.size() == 1)
        setText(QString("Paste '%1' hierarchy to '%2'").arg(_createdWidgets[0]).arg(_targetPath));
    else
        setText(QString("Paste %1 hierarchies to '%2'").arg(_createdWidgets.size()).arg(_targetPath));

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

LayoutRenameCommand::LayoutRenameCommand(LayoutVisualMode& visualMode, const QString& path, const QString& newName)
    : _visualMode(visualMode)
    , _newName(newName)
{
    int sepPos = path.lastIndexOf('/');
    if (sepPos < 0)
    {
        _oldName = path;
    }
    else
    {
        _oldName = path.mid(sepPos + 1);
        _parentPath = path.left(sepPos);
    }

    setText(QString("Rename '%1' to '%2'").arg(_oldName, _newName));
}

void LayoutRenameCommand::undo()
{
    QUndoCommand::undo();

    auto manipulator = _visualMode.getScene()->getManipulatorByPath(_parentPath + '/' + _newName);
    assert(manipulator->getTreeItem());
    manipulator->getWidget()->setName(CEGUIUtils::qStringToString(_oldName));
    manipulator->getTreeItem()->setText(_oldName);
    manipulator->getTreeItem()->refreshPathData();
    manipulator->triggerPropertyManagerCallback({"Name", "NamePath"});
}

void LayoutRenameCommand::redo()
{
    auto manipulator = _visualMode.getScene()->getManipulatorByPath(_parentPath + '/' + _oldName);
    assert(manipulator->getTreeItem());
    manipulator->getWidget()->setName(CEGUIUtils::qStringToString(_newName));
    manipulator->getTreeItem()->setText(_newName);
    manipulator->getTreeItem()->refreshPathData();
    manipulator->triggerPropertyManagerCallback({"Name", "NamePath"});

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

MoveInParentWidgetListCommand::MoveInParentWidgetListCommand(LayoutVisualMode& visualMode, QStringList&& paths, int delta)
    : _visualMode(visualMode)
    , _paths(std::move(paths))
    , _delta(delta)
{
    refreshText();
}

void MoveInParentWidgetListCommand::undo()
{
    QUndoCommand::undo();

    if (!_delta) return;

    for (const QString& path : _paths)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(path);
        auto parentManipulator = static_cast<LayoutManipulator*>(manipulator->parentItem());
        auto container = static_cast<CEGUI::SequentialLayoutContainer*>(parentManipulator->getWidget());

        size_t oldPos = container->getPositionOfChild(manipulator->getWidget());
        size_t newPos = static_cast<size_t>(static_cast<int>(oldPos) - _delta);
        container->swapChildPositions(oldPos, newPos);
        assert(newPos == container->getPositionOfChild(manipulator->getWidget()));

        parentManipulator->updateFromWidget(true, true);
        parentManipulator->getTreeItem()->refreshOrderingData();
    }
}

void MoveInParentWidgetListCommand::redo()
{
    if (!_delta) return;

    for (const QString& path : _paths)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(path);
        auto parentManipulator = static_cast<LayoutManipulator*>(manipulator->parentItem());
        auto container = static_cast<CEGUI::SequentialLayoutContainer*>(parentManipulator->getWidget());

        size_t oldPos = container->getPositionOfChild(manipulator->getWidget());
        size_t newPos = static_cast<size_t>(static_cast<int>(oldPos) + _delta);
        container->swapChildPositions(oldPos, newPos);
        assert(newPos == container->getPositionOfChild(manipulator->getWidget()));

        parentManipulator->updateFromWidget(true, true);
        parentManipulator->getTreeItem()->refreshOrderingData();
    }

    QUndoCommand::redo();
}

bool MoveInParentWidgetListCommand::mergeWith(const QUndoCommand* other)
{
    const MoveInParentWidgetListCommand* otherCmd = dynamic_cast<const MoveInParentWidgetListCommand*>(other);
    if (!otherCmd) return false;

    if (_paths.size() != otherCmd->_paths.size()) return false;

    for (const auto& rec : otherCmd->_paths)
        if (!_paths.contains(rec)) return false;

    // The same set of widgets, can merge

    _delta = otherCmd->_delta;
    refreshText();
    return true;
}

void MoveInParentWidgetListCommand::refreshText()
{
    if (_paths.size() == 1)
        setText(QString("Move '%1' by %2 in parent widget list").arg(_paths[0]).arg(_delta));
    else
        setText(QString("Move %1 widgets by %2 in parent widget list").arg(_paths.size()).arg(_delta));
}

//---------------------------------------------------------------------
