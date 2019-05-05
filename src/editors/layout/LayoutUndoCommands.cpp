#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/cegui/CEGUIUtils.h"
#include <CEGUI/Window.h>
#include <CEGUI/WindowManager.h>

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

    QStringList pathes;
    for (const auto& rec : _records)
        pathes.push_back(rec.path);

    for (const auto& rec : otherCmd->_records)
        if (!pathes.contains(rec.path)) return false;

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

    QStringList pathes;
    for (const auto& rec : _records)
        pathes.push_back(rec.path);

    for (const auto& rec : otherCmd->_records)
        if (!pathes.contains(rec.path)) return false;

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
    /*
        if self.widgetPaths == cmd.widgetPaths and self.propertyName == cmd.propertyName:
            self.newValue = cmd.newValue

            return True
    */
    return false;
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
    , _records(records)
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

    QStringList pathes;
    for (const auto& rec : _records)
        pathes.push_back(rec.path);

    for (const auto& rec : otherCmd->_records)
        if (!pathes.contains(rec.path)) return false;

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
    , _records(records)
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

    QStringList pathes;
    for (const auto& rec : _records)
        pathes.push_back(rec.path);

    for (const auto& rec : otherCmd->_records)
        if (!pathes.contains(rec.path)) return false;

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

/*
class ReparentCommand(commands.UndoCommand):
    """This command changes parent of given windows
    """

    def __init__(self, visual, oldWidgetPaths, newWidgetPaths):
        super(ReparentCommand, self).__init__()

        self.visual = visual

        self.oldWidgetPaths = oldWidgetPaths
        self.newWidgetPaths = newWidgetPaths

        self.refreshText()

    def refreshText(self):
        if len(self.oldWidgetPaths) == 1:
            self.setText("Reparent '%s' to '%s'" % (self.oldWidgetPaths[0], self.newWidgetPaths[0]))
        else:
            self.setText("Reparent %i widgets" % (len(self.oldWidgetPaths)))

    def id(self):
        return idbase + 8

    def mergeWith(self, cmd):
        if self.newWidgetPaths == cmd.oldWidgetPaths:
            self.newWidgetPaths = cmd.newWidgetPaths
            self.refreshText()

            return True

        return False

    def undo(self):
        super(ReparentCommand, self).undo()

        self.visual.scene.clearSelection()
        self.visual.hierarchyDockWidget.treeView.clearSelection()

        i = 0
        while i < len(self.newWidgetPaths):
            widgetPath = self.newWidgetPaths[i]
            oldWidgetPath = self.oldWidgetPaths[i]
            newWidgetName = widgetPath[widgetPath.rfind("/") + 1:]
            oldWidgetName = oldWidgetPath[oldWidgetPath.rfind("/") + 1:]

            widgetManipulator = self.visual.scene.getManipulatorByPath(widgetPath)
            oldParentPath = oldWidgetPath[0:oldWidgetPath.rfind("/")]
            oldParentManipulator = self.visual.scene.getManipulatorByPath(oldParentPath)

            # remove it from the current CEGUI parent widget
            ceguiParentWidget = widgetManipulator.widget.getParent()
            if ceguiParentWidget is not None:
                ceguiParentWidget.removeChild(widgetManipulator.widget)

            # rename it if necessary
            if oldWidgetName != newWidgetName:
                widgetManipulator.widget.setProperty("Name", oldWidgetName)

            # add it to the old CEGUI parent widget
            ceguiOldParentWidget = oldParentManipulator.widget
            ceguiOldParentWidget.addChild(widgetManipulator.widget)

            # and sort out the manipulators
            widgetManipulator.setParentItem(oldParentManipulator)

            widgetManipulator.updateFromWidget(True, True)

            i += 1

        self.visual.hierarchyDockWidget.refresh()

    def redo(self):
        self.visual.scene.clearSelection()
        self.visual.hierarchyDockWidget.treeView.clearSelection()

        i = 0
        while i < len(self.oldWidgetPaths):
            widgetPath = self.oldWidgetPaths[i]
            newWidgetPath = self.newWidgetPaths[i]
            oldWidgetName = widgetPath[widgetPath.rfind("/") + 1:]
            newWidgetName = newWidgetPath[newWidgetPath.rfind("/") + 1:]

            widgetManipulator = self.visual.scene.getManipulatorByPath(widgetPath)
            newParentPath = newWidgetPath[0:newWidgetPath.rfind("/")]
            newParentManipulator = self.visual.scene.getManipulatorByPath(newParentPath)

            # remove it from the current CEGUI parent widget
            ceguiParentWidget = widgetManipulator.widget.getParent()
            if ceguiParentWidget is not None:
                ceguiParentWidget.removeChild(widgetManipulator.widget)

            # rename it if necessary
            if oldWidgetName != newWidgetName:
                widgetManipulator.widget.setProperty("Name", newWidgetName)

            # add it to the new CEGUI parent widget
            ceguiNewParentWidget = newParentManipulator.widget
            ceguiNewParentWidget.addChild(widgetManipulator.widget)

            # and sort out the manipulators
            widgetManipulator.setParentItem(newParentManipulator)

            widgetManipulator.updateFromWidget(True, True)

            i += 1

        self.visual.hierarchyDockWidget.refresh()
        super(ReparentCommand, self).redo()



class NormaliseSizeCommand(ResizeCommand):
    def __init__(self, visual, widgetPaths, oldPositions, oldSizes):
        newSizes = {}
        for widgetPath in widgetPaths:
            newSizes[widgetPath] = self.normaliseSize(widgetPath)

        # we use oldPositions as newPositions because this command never changes positions of anything
        super(NormaliseSizeCommand, self).__init__(visual, widgetPaths, oldPositions, oldSizes, oldPositions, newSizes)

    def normaliseSize(self, widgetPath):
        raise NotImplementedError("Each subclass of NormaliseSizeCommand must implement the normaliseSize method")

    def id(self):
        raise NotImplementedError("Each subclass of NormaliseSizeCommand must implement the id method")

    def mergeWith(self, cmd):
        # we never merge size normalising commands
        return False


class NormaliseSizeToRelativeCommand(NormaliseSizeCommand):
    def __init__(self, visual, widgetPaths, oldPositions, oldSizes):
        # even though this will be set again in the ResizeCommand constructor we need to set it right now
        # because otherwise the normaliseSize will not work!
        self.visual = visual

        if len(self.widgetPaths) == 1:
            self.setText("Normalise size of '%s' to relative" % (self.widgetPaths[0]))
        else:
            self.setText("Normalise size of %i widgets to relative" % (len(self.widgetPaths)))

        super(NormaliseSizeToRelativeCommand, self).__init__(visual, widgetPaths, oldPositions, oldSizes)

    def normaliseSize(self, widgetPath):
        manipulator = self.visual.scene.getManipulatorByPath(widgetPath)
        pixelSize = manipulator.widget.getPixelSize()
        baseSize = manipulator.getBaseSize()

        return PyCEGUI.USize(PyCEGUI.UDim(pixelSize.d_width / baseSize.d_width, 0),
                             PyCEGUI.UDim(pixelSize.d_height / baseSize.d_height, 0))

    def id(self):
        return idbase + 10


class NormaliseSizeToAbsoluteCommand(NormaliseSizeCommand):
    def __init__(self, visual, widgetPaths, oldPositions, oldSizes):
        # even though this will be set again in the ResizeCommand constructor we need to set it right now
        # because otherwise the normaliseSize will not work!
        self.visual = visual

        if len(self.widgetPaths) == 1:
            self.setText("Normalise size of '%s' to absolute" % (self.widgetPaths[0]))
        else:
            self.setText("Normalise size of %i widgets to absolute" % (len(self.widgetPaths)))

        super(NormaliseSizeToAbsoluteCommand, self).__init__(visual, widgetPaths, oldPositions, oldSizes)

    def normaliseSize(self, widgetPath):
        manipulator = self.visual.scene.getManipulatorByPath(widgetPath)
        pixelSize = manipulator.widget.getPixelSize()

        return PyCEGUI.USize(PyCEGUI.UDim(0, pixelSize.d_width),
                             PyCEGUI.UDim(0, pixelSize.d_height))

    def id(self):
        return idbase + 11


class NormalisePositionCommand(MoveCommand):
    def __init__(self, visual, widgetPaths, oldPositions):
        newPositions = {}
        for widgetPath, oldPosition in oldPositions.iteritems():
            newPositions[widgetPath] = self.normalisePosition(widgetPath, oldPosition)

        super(NormalisePositionCommand, self).__init__(visual, widgetPaths, oldPositions, newPositions)

    def normalisePosition(self, widgetPath, oldPosition):
        raise NotImplementedError("Each subclass of NormalisePositionCommand must implement the normalisePosition method")

    def id(self):
        raise NotImplementedError("Each subclass of NormalisePositionCommand must implement the id method")

    def mergeWith(self, cmd):
        # we never merge position normalising commands
        return False


class NormalisePositionToRelativeCommand(NormalisePositionCommand):
    def __init__(self, visual, widgetPaths, oldPositions):
        # even though this will be set again in the MoveCommand constructor we need to set it right now
        # because otherwise the normalisePosition method will not work!
        self.visual = visual

        if len(self.widgetPaths) == 1:
            self.setText("Normalise position of '%s' to relative" % (self.widgetPaths[0]))
        else:
            self.setText("Normalise position of %i widgets to relative" % (len(self.widgetPaths)))

        super(NormalisePositionToRelativeCommand, self).__init__(visual, widgetPaths, oldPositions)

    def normalisePosition(self, widgetPath, position):
        manipulator = self.visual.scene.getManipulatorByPath(widgetPath)
        baseSize = manipulator.getBaseSize()

        return PyCEGUI.UVector2(PyCEGUI.UDim((position.d_x.d_offset + position.d_x.d_scale * baseSize.d_width) / baseSize.d_width, 0),
                                PyCEGUI.UDim((position.d_y.d_offset + position.d_y.d_scale * baseSize.d_height) / baseSize.d_height, 0))

    def id(self):
        return idbase + 12


class NormalisePositionToAbsoluteCommand(NormalisePositionCommand):
    def __init__(self, visual, widgetPaths, oldPositions):
        # even though this will be set again in the MoveCommand constructor we need to set it right now
        # because otherwise the normalisePosition method will not work!
        self.visual = visual

        if len(self.widgetPaths) == 1:
            self.setText("Normalise position of '%s' to absolute" % (self.widgetPaths[0]))
        else:
            self.setText("Normalise position of %i widgets to absolute" % (len(self.widgetPaths)))

        super(NormalisePositionToAbsoluteCommand, self).__init__(visual, widgetPaths, oldPositions)

    def normalisePosition(self, widgetPath, position):
        manipulator = self.visual.scene.getManipulatorByPath(widgetPath)
        baseSize = manipulator.getBaseSize()

        return PyCEGUI.UVector2(PyCEGUI.UDim(0, position.d_x.d_offset + position.d_x.d_scale * baseSize.d_width),
                                PyCEGUI.UDim(0, position.d_y.d_offset + position.d_y.d_scale * baseSize.d_height))

    def id(self):
        return idbase + 13


class RenameCommand(commands.UndoCommand):
    """This command changes the name of the given widget
    """

    def __init__(self, visual, oldWidgetPath, newWidgetName):
        super(RenameCommand, self).__init__()

        self.visual = visual

        # NOTE: rfind returns -1 when '/' can't be found, so in case the widget
        #       is the root widget, -1 + 1 = 0 and the slice just returns
        #       full name of the widget

        self.oldWidgetPath = oldWidgetPath
        self.oldWidgetName = oldWidgetPath[oldWidgetPath.rfind("/") + 1:]

        self.newWidgetPath = oldWidgetPath[:oldWidgetPath.rfind("/") + 1] + newWidgetName
        self.newWidgetName = newWidgetName

        self.refreshText()

    def refreshText(self):
        self.setText("Rename '%s' to '%s'" % (self.oldWidgetName, self.newWidgetName))

    def id(self):
        return idbase + 14

    def mergeWith(self, cmd):
        # don't merge if the new rename command will simply revert to previous commands old name
        if self.newWidgetPath == cmd.oldWidgetPath and self.oldWidgetName != cmd.newWidgetName:
            self.newWidgetName = cmd.newWidgetName
            self.newWidgetPath = self.oldWidgetPath[0:self.oldWidgetPath.rfind("/") + 1] + self.newWidgetName

            self.refreshText()
            return True

        return False

    def undo(self):
        super(RenameCommand, self).undo()

        widgetManipulator = self.visual.scene.getManipulatorByPath(self.newWidgetPath)
        assert(hasattr(widgetManipulator, "treeItem"))
        assert(widgetManipulator.treeItem is not None)

        widgetManipulator.widget.setName(self.oldWidgetName)
        widgetManipulator.treeItem.setText(self.oldWidgetName)
        widgetManipulator.treeItem.refreshPathData()

        widgetManipulator.triggerPropertyManagerCallback({"Name", "NamePath"})

    def redo(self):
        widgetManipulator = self.visual.scene.getManipulatorByPath(self.oldWidgetPath)
        assert(hasattr(widgetManipulator, "treeItem"))
        assert(widgetManipulator.treeItem is not None)

        widgetManipulator.widget.setName(self.newWidgetName)
        widgetManipulator.treeItem.setText(self.newWidgetName)
        widgetManipulator.treeItem.refreshPathData()

        widgetManipulator.triggerPropertyManagerCallback({"Name", "NamePath"})

        super(RenameCommand, self).redo()


class RoundPositionCommand(MoveCommand):
    def __init__(self, visual, widgetPaths, oldPositions):

        # calculate the new, rounded positions for the widget(s)
        newPositions = {}
        for widgetPath, oldPosition in oldPositions.iteritems():
            newPositions[widgetPath] = self.roundAbsolutePosition(oldPosition)

        super(RoundPositionCommand, self).__init__(visual, widgetPaths, oldPositions, newPositions)

        if len(self.widgetPaths) == 1:
            self.setText("Round absolute position of '%s' to nearest integer" % (self.widgetPaths[0]))
        else:
            self.setText("Round absolute positions of %i widgets to nearest integers" % (len(self.widgetPaths)))

    @staticmethod
    def roundAbsolutePosition(oldPosition):
        return PyCEGUI.UVector2(PyCEGUI.UDim(oldPosition.d_x.d_scale, PyCEGUI.CoordConverter.alignToPixels(oldPosition.d_x.d_offset)),
                                PyCEGUI.UDim(oldPosition.d_y.d_scale, PyCEGUI.CoordConverter.alignToPixels(oldPosition.d_y.d_offset)))

    def id(self):
        return idbase + 15

    def mergeWith(self, cmd):
        # merge if the new round position command will apply to the same widget
        if self.widgetPaths == cmd.widgetPaths:
            return True
        else:
            return False


class RoundSizeCommand(ResizeCommand):
    def __init__(self, visual, widgetPaths, oldPositions, oldSizes):

        # calculate the new, rounded sizes for the widget(s)
        newSizes = {}
        for widgetPath, oldSize in oldSizes.iteritems():
            newSizes[widgetPath] = self.roundAbsoluteSize(oldSize)

        # we use oldPositions as newPositions because this command never changes positions of anything
        super(RoundSizeCommand, self).__init__(visual, widgetPaths, oldPositions, oldSizes, oldPositions, newSizes)

        if len(self.widgetPaths) == 1:
            self.setText("Round absolute size of '%s' to nearest integer" % (self.widgetPaths[0]))
        else:
            self.setText("Round absolute sizes of %i widgets to nearest integers" % (len(self.widgetPaths)))

    @staticmethod
    def roundAbsoluteSize(oldSize):
        return PyCEGUI.USize(PyCEGUI.UDim(oldSize.d_width.d_scale, PyCEGUI.CoordConverter.alignToPixels(oldSize.d_width.d_offset)),
                             PyCEGUI.UDim(oldSize.d_height.d_scale, PyCEGUI.CoordConverter.alignToPixels(oldSize.d_height.d_offset)))

    def id(self):
        return idbase + 16

    def mergeWith(self, cmd):
        # merge if the new round size command will apply to the same widget
        if self.widgetPaths == cmd.widgetPaths:
            return True
        else:
            return False

class MoveInParentWidgetListCommand(commands.UndoCommand):
    def __init__(self, visual, widgetPaths, delta):
        super(MoveInParentWidgetListCommand, self).__init__()

        self.visual = visual
        self.widgetPaths = widgetPaths
        self.delta = delta

        self.refreshText()

    def refreshText(self):
        if len(self.widgetPaths) == 1:
            self.setText("Move '%s' by %i in parent widget list" % (self.widgetPaths[0], self.delta))
        else:
            self.setText("Move %i widgets by %i in parent widget list" % (len(self.widgetPaths), self.delta))

    def id(self):
        return idbase + 17

    def mergeWith(self, cmd):
        if self.widgetPaths == cmd.widgetPaths:
            self.delta += cmd.delta
            self.refreshText()
            return True

        return False

    def undo(self):
        super(MoveInParentWidgetListCommand, self).undo()

        if self.delta != 0:
            for widgetPath in reversed(self.widgetPaths):
                widgetManipulator = self.visual.scene.getManipulatorByPath(widgetPath)
                parentManipulator = widgetManipulator.parentItem()
                assert(isinstance(parentManipulator, widgethelpers.Manipulator))
                assert(isinstance(parentManipulator.widget, PyCEGUI.SequentialLayoutContainer))

                oldPosition = parentManipulator.widget.getPositionOfChild(widgetManipulator.widget)
                newPosition = oldPosition - self.delta
                parentManipulator.widget.swapChildPositions(oldPosition, newPosition)
                assert(newPosition == parentManipulator.widget.getPositionOfChild(widgetManipulator.widget))

                parentManipulator.updateFromWidget(True, True)
                parentManipulator.treeItem.refreshOrderingData(True, True)

    def redo(self):
        if self.delta != 0:
            for widgetPath in self.widgetPaths:
                widgetManipulator = self.visual.scene.getManipulatorByPath(widgetPath)
                parentManipulator = widgetManipulator.parentItem()
                assert(isinstance(parentManipulator, widgethelpers.Manipulator))
                assert(isinstance(parentManipulator.widget, PyCEGUI.SequentialLayoutContainer))

                oldPosition = parentManipulator.widget.getPositionOfChild(widgetManipulator.widget)
                newPosition = oldPosition + self.delta
                parentManipulator.widget.swapChildPositions(oldPosition, newPosition)
                assert(newPosition == parentManipulator.widget.getPositionOfChild(widgetManipulator.widget))

                parentManipulator.updateFromWidget(True, True)
                parentManipulator.treeItem.refreshOrderingData(True, True)

        super(MoveInParentWidgetListCommand, self).redo()
*/
