#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/cegui/CEGUIUtils.h"
#include <CEGUI/widgets/LayoutContainer.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/CoordConverter.h>
#include "qtreeview.h"

static LayoutManipulator* CreateManipulatorFromDataStream(LayoutVisualMode& visualMode, LayoutManipulator* parent, QDataStream& stream)
{
    LayoutManipulator* manipulator;

    if (parent)
    {
        CEGUI::Window* widget = CEGUIUtils::deserializeWidget(stream, parent->getWidget());
        manipulator = parent->createChildManipulator(widget);
    }
    else
    {
        // No parent, root widget
        CEGUI::Window* widget = CEGUIUtils::deserializeWidget(stream, nullptr);
        manipulator = new LayoutManipulator(visualMode, nullptr, widget);
        visualMode.setRootWidgetManipulator(manipulator);
    }

    manipulator->createChildManipulators(true, false);
    manipulator->updateFromWidget();
    manipulator->setSelected(true);

    return manipulator;
}

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

        manipulator->updatePropertiesFromWidget({"Position", "Area"});
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

        manipulator->updatePropertiesFromWidget({"Position", "Area"});
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

        manipulator->updatePropertiesFromWidget({"Size", "Position", "Area"});
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

        manipulator->updatePropertiesFromWidget({"Size", "Position", "Area"});
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
    for (const QString& currPath : paths)
    {
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
        const QString& path = _paths[i++];
        const int sepPos = path.lastIndexOf('/');
        LayoutManipulator* parent = (sepPos < 0) ? nullptr : _visualMode.getScene()->getManipulatorByPath(path.left(sepPos));

        CreateManipulatorFromDataStream(_visualMode, parent, stream);
    }

    _visualMode.getHierarchyDockWidget()->refresh();
}

void LayoutDeleteCommand::redo()
{
    for (const QString& path : _paths)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(path);
        if (!manipulator)
        {
            assert(false && "LayoutDeleteCommand::redo() > manipulator not found!");
            continue;
        }

        manipulator->detach();
    }

    _visualMode.getHierarchyDockWidget()->refresh();

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

LayoutCreateCommand::LayoutCreateCommand(LayoutVisualMode& visualMode, const QString& parentPath, const QString& type, const QString& name, QPointF scenePos)
    : _visualMode(visualMode)
    , _parentPath(parentPath)
    , _type(type)
    , _name(name)
    , _scenePos(scenePos)
{
    setText(QString("Create '%1' of type '%2'").arg(name, type));
}

void LayoutCreateCommand::undo()
{
    QUndoCommand::undo();

    const QString fullPath = _parentPath.isEmpty() ? _name : _parentPath + '/' + _name;
    auto manipulator = _visualMode.getScene()->getManipulatorByPath(fullPath);
    auto parentManipulator = dynamic_cast<LayoutManipulator*>(manipulator->parentItem());
    manipulator->detach();

    // Mostly for the LC case, its area depends on the children
    if (parentManipulator) parentManipulator->updateFromWidget(true, true);

    _visualMode.getHierarchyDockWidget()->refresh();
}

void LayoutCreateCommand::redo()
{
    CEGUI::Window* widget = CEGUI::WindowManager::getSingleton().createWindow(
                CEGUIUtils::qStringToString(_type), CEGUIUtils::qStringToString(_name));

    LayoutManipulator* parent = _parentPath.isEmpty() ? nullptr :
                _visualMode.getScene()->getManipulatorByPath(_parentPath);

    // Setup position and size of the new widget
    if (_type == "DefaultWindow" && !parent)
    {
        // Special case - root widget. Setup it with most useful parameters.
        widget->setPosition(CEGUI::UVector2(CEGUI::UDim(0.f, 0.f), CEGUI::UDim(0.f, 0.f)));
        widget->setSize(CEGUI::USize(CEGUI::UDim(1.f, 0.f), CEGUI::UDim(1.f, 0.f)));
        widget->setCursorPassThroughEnabled(true);
    }
    else
    {
        // Convert requested position into parent cordinate system
        glm::vec2 pos(static_cast<float>(_scenePos.x()), static_cast<float>(_scenePos.y()));
        if (parent)
            pos = CEGUI::CoordConverter::screenToWindow(*parent->getWidget(), pos);

        // Place new window at the requested point in context coords
        widget->setPosition(CEGUI::UVector2(CEGUI::UDim(0.f, pos.x), CEGUI::UDim(0.f, pos.y)));

        // If the size is 0x0, the widget will be hard to deal with, lets fix that in that case
        if (widget->getSize() == CEGUI::USize(CEGUI::UDim(0.f, 0.f), CEGUI::UDim(0.f, 0.f)))
            widget->setSize(CEGUI::USize(CEGUI::UDim(0.f, 50.f), CEGUI::UDim(0.f, 50.f)));
    }

    // Default maximum size to the whole screen
    widget->setMaxSize(CEGUI::USize(CEGUI::UDim(1.f, 0.f), CEGUI::UDim(1.f, 0.f)));

    LayoutManipulator* manipulator;
    if (parent)
    {
        manipulator = parent->createChildManipulator(widget);
        parent->getWidget()->addChild(widget);
    }
    else
    {
        manipulator = new LayoutManipulator(_visualMode, nullptr, widget);
        _visualMode.setRootWidgetManipulator(manipulator);
    }

    manipulator->updateFromWidget(true, true);
    manipulator->createChildManipulators(true, false);

    // Ensure this isn't obscured by it's parent
    manipulator->moveToFront();

    // Make only the new widget selected
    _visualMode.getScene()->clearSelection();
    _visualMode.getHierarchyDockWidget()->getTreeView()->clearSelection();
    manipulator->setSelected(true);

    _visualMode.getHierarchyDockWidget()->refresh();

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

LayoutPropertyEditCommand::LayoutPropertyEditCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records,
                                                     const QString& propertyName, size_t multiChangeId)
    : _visualMode(visualMode)
    , _records(std::move(records))
    , _propertyName(CEGUIUtils::qStringToString(propertyName))
    , _multiChangeId(multiChangeId)
{
    refreshText();
}

void LayoutPropertyEditCommand::undo()
{
    QUndoCommand::undo();

    QStringList properties;
    fillInfluencedPropertyList(properties);

    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        assert(manipulator);
        manipulator->getWidget()->setProperty(_propertyName, rec.oldValue);
        manipulator->updateFromWidget(false, true);
        manipulator->update();
        manipulator->updatePropertiesFromWidget(properties);
    }

    //_firstCall = false;

    // Make sure to redraw the scene so the changes are visible
    //_visualMode.getScene()->update();
}

void LayoutPropertyEditCommand::redo()
{
    QStringList properties;
    fillInfluencedPropertyList(properties);

    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        assert(manipulator);
        if (manipulator->getWidget()->getProperty(_propertyName) != rec.newValue)
        {
            manipulator->getWidget()->setProperty(_propertyName, rec.newValue);
            manipulator->updateFromWidget(false, true);
            manipulator->update();
            manipulator->updatePropertiesFromWidget(properties);
        }
    }

    QUndoCommand::redo();
}

bool LayoutPropertyEditCommand::mergeWith(const QUndoCommand* other)
{
    const LayoutPropertyEditCommand* otherCmd = dynamic_cast<const LayoutPropertyEditCommand*>(other);
    if (!otherCmd) return false;

    // Can merge only the same property
    if (_propertyName != otherCmd->_propertyName) return false;

    // Multiproperty merge
    if (_multiChangeId > 0 && _multiChangeId == otherCmd->_multiChangeId)
    {
        for (const auto& otherRec : otherCmd->_records)
        {
            auto it = std::find_if(_records.cbegin(), _records.cend(), [&otherRec](const Record& rec)
            {
                return rec.path == otherRec.path;
            });

            if (it == _records.cend())
                _records.push_back(otherRec);
        }
        refreshText();
        return true;
    }

    // We don't merge subsequent property changes intentionally to have a full history
    return false;
}

void LayoutPropertyEditCommand::refreshText()
{
    const QString propertyName = CEGUIUtils::stringToQString(_propertyName);
    if (_records.size() == 1)
        setText(QString("Change '%1' in '%2'").arg(propertyName, _records[0].path));
    else
        setText(QString("Change '%1' in %2 widgets").arg(propertyName).arg(_records.size()));
}

// Some properties are related to others so that when one changes, the others change too.
// The following ensures that notifications are sent about the related properties as well.
void LayoutPropertyEditCommand::fillInfluencedPropertyList(QStringList& list)
{
    //???to CEGUIManipulator? call its onPropertyChanged?
    //???is critical not to update changed property value? will send onPropertyDidChange?
    if (true) //!_firstCall)
        list.append(CEGUIUtils::stringToQString(_propertyName));

    if (_propertyName == "Size" || _propertyName == "Position")
        list.append("Area");
    else if (_propertyName == "Area")
        list.append({"Position", "Size"});
    else if (_propertyName == "Name")
        list.append("NamePath");
    else if (_propertyName == "NamePath")
        list.append("Name");
}

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

        manipulator->updatePropertiesFromWidget({"HorizontalAlignment"});
    }
}

void LayoutHorizontalAlignCommand::redo()
{
    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        manipulator->getWidget()->setHorizontalAlignment(_newAlignment);
        manipulator->updateFromWidget();

        manipulator->updatePropertiesFromWidget({"HorizontalAlignment"});
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

        manipulator->updatePropertiesFromWidget({"VerticalAlignment"});
    }
}

void LayoutVerticalAlignCommand::redo()
{
    for (const auto& rec : _records)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(rec.path);
        manipulator->getWidget()->setVerticalAlignment(_newAlignment);
        manipulator->updateFromWidget();

        manipulator->updatePropertiesFromWidget({"VerticalAlignment"});
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

LayoutMoveInHierarchyCommand::LayoutMoveInHierarchyCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records, const QString& newParentPath)
    : _visualMode(visualMode)
    , _records(std::move(records))
    , _newParentPath(newParentPath)
{
    // Remember initial position and size
    for (auto& rec : _records)
    {
        auto widget = _visualMode.getScene()->getManipulatorByPath(rec.oldParentPath + '/' + rec.oldName)->getWidget();
        rec.oldPos = widget->getPosition();
        rec.oldSize = widget->getSize();
    }

    if (_records.size() == 1)
        setText(QString("Move '%1' in hierarchy").arg(_records[0].oldName));
    else
        setText(QString("Move %1 widgets in hierarchy").arg(_records.size()));
}

void LayoutMoveInHierarchyCommand::undo()
{
    QUndoCommand::undo();

    _visualMode.getScene()->clearSelection();
    _visualMode.getHierarchyDockWidget()->getTreeView()->clearSelection();

    for (const auto& rec : _records)
    {
        auto widgetManipulator = _visualMode.getScene()->getManipulatorByPath(_newParentPath + '/' + rec.newName);
        auto oldParentManipulator = _visualMode.getScene()->getManipulatorByPath(rec.oldParentPath);
        auto newParentManipulator = dynamic_cast<LayoutManipulator*>(widgetManipulator->parentItem());

        // Remove it from the current CEGUI parent widget
        if (oldParentManipulator != newParentManipulator)
        {
            auto parentWidget = widgetManipulator->getWidget()->getParent();
            if (parentWidget) parentWidget->removeChild(widgetManipulator->getWidget());
        }

        // Rename it if necessary
        if (rec.oldName != rec.newName)
        {
            widgetManipulator->getWidget()->setName(CEGUIUtils::qStringToString(rec.oldName));
            widgetManipulator->updatePropertiesFromWidget({"Name", "NamePath"});
        }

        // Restore initial position and size if necessary
        // TODO: is LC the only possible reason of pos & size change when reparenting?
        //if (newParentManipulator->isLayoutContainer())
        {
            widgetManipulator->getWidget()->setPosition(rec.oldPos);
            widgetManipulator->getWidget()->setSize(rec.oldSize);
            widgetManipulator->updatePropertiesFromWidget({"Size", "Position", "Area"});
        }

        if (oldParentManipulator != newParentManipulator)
        {
            // Add it to the old CEGUI parent widget and sort out the manipulators
            oldParentManipulator->getWidget()->addChild(widgetManipulator->getWidget());
            widgetManipulator->setParentItem(oldParentManipulator);
        }

        // FIXME: allow reordering in any window? Needs CEGUI change.
        // http://cegui.org.uk/forum/viewtopic.php?f=3&t=7542
        auto parentLC = dynamic_cast<CEGUI::LayoutContainer*>(oldParentManipulator->getWidget());
        if (parentLC)
        {
            const size_t currIndex = widgetManipulator->getWidgetIndexInParent();
            const size_t destIndex = rec.oldChildIndex;
            parentLC->moveChildToIndex(currIndex, destIndex > currIndex ? destIndex + 1 : destIndex);
        }

        // Update widget and its previous parent (the second is mostly for the layout container case)
        widgetManipulator->updateFromWidget(true, true);
        if (newParentManipulator) newParentManipulator->updateFromWidget(true, true);
    }

    _visualMode.getHierarchyDockWidget()->refresh();
}

void LayoutMoveInHierarchyCommand::redo()
{
    _visualMode.getScene()->clearSelection();
    _visualMode.getHierarchyDockWidget()->getTreeView()->clearSelection();

    for (const auto& rec : _records)
    {
        auto widgetManipulator = _visualMode.getScene()->getManipulatorByPath(rec.oldParentPath + '/' + rec.oldName);
        auto newParentManipulator = _visualMode.getScene()->getManipulatorByPath(_newParentPath);
        auto oldParentManipulator = dynamic_cast<LayoutManipulator*>(widgetManipulator->parentItem());

        // FIXME: allow reordering in any window? Needs CEGUI change.
        if (newParentManipulator == oldParentManipulator && !newParentManipulator->isLayoutContainer())
        {
            // Reordering inside a parent is supported only for layout containers for now
            assert(false);
            continue;
        }

        // Remove it from the current CEGUI parent widget
        if (oldParentManipulator != newParentManipulator)
        {
            auto parentWidget = widgetManipulator->getWidget()->getParent();
            if (parentWidget) parentWidget->removeChild(widgetManipulator->getWidget());
        }

        // Rename it if necessary
        if (rec.oldName != rec.newName)
        {
            widgetManipulator->getWidget()->setName(CEGUIUtils::qStringToString(rec.newName));
            widgetManipulator->updatePropertiesFromWidget({"Name", "NamePath"});
        }

        if (oldParentManipulator != newParentManipulator)
        {
            // Add it to the new CEGUI parent widget and sort out the manipulators
            newParentManipulator->getWidget()->addChild(widgetManipulator->getWidget());
            widgetManipulator->setParentItem(newParentManipulator);
        }

        // FIXME: allow reordering in any window? Needs CEGUI change.
        // http://cegui.org.uk/forum/viewtopic.php?f=3&t=7542
        auto parentLC = dynamic_cast<CEGUI::LayoutContainer*>(newParentManipulator->getWidget());
        if (parentLC)
            parentLC->moveChildToIndex(widgetManipulator->getWidget(), rec.newChildIndex);

        // Update widget and its previous parent (the second is mostly for the layout container case)
        widgetManipulator->updateFromWidget(true, true);
        if (oldParentManipulator) oldParentManipulator->updateFromWidget(true, true);
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

    scene->clearSelection();

    QDataStream stream(&_data, QIODevice::ReadOnly);
    while (!stream.atEnd())
    {
        if (!target && !_createdWidgets.empty())
        {
            assert(false && "Can't paste more than one hierarchy as the root");
            break;
        }

        LayoutManipulator* manipulator = CreateManipulatorFromDataStream(_visualMode, target, stream);
        _createdWidgets.push_back(manipulator->getWidgetPath());
    }

    // Update the topmost parent widget recursively to get possible resize or
    // repositions of the pasted widgets into the manipulator data.
    if (target) target->updateFromWidget(true, true);

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
    manipulator->getWidget()->setName(CEGUIUtils::qStringToString(_oldName));
    manipulator->updatePropertiesFromWidget({"Name", "NamePath"});
}

void LayoutRenameCommand::redo()
{
    auto manipulator = _visualMode.getScene()->getManipulatorByPath(_parentPath + '/' + _oldName);
    manipulator->getWidget()->setName(CEGUIUtils::qStringToString(_newName));
    manipulator->updatePropertiesFromWidget({"Name", "NamePath"});

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

MoveInParentWidgetListCommand::MoveInParentWidgetListCommand(LayoutVisualMode& visualMode, QStringList&& paths, int delta)
    : _visualMode(visualMode)
    , _paths(std::move(paths))
    , _delta(delta)
{
    // TODO: We currently only support moving one widget at a time.
    //       Fixing this involves sorting the widgets by their position in
    //       the parent widget and then either working from the "right" side
    //       if delta > 0 or from the left side if delta < 0.
    assert(_paths.size() == 1);

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
        auto container = static_cast<CEGUI::LayoutContainer*>(parentManipulator->getWidget());

        size_t oldPos = container->getChildIndex(manipulator->getWidget());
        size_t newPos = static_cast<size_t>(static_cast<int>(oldPos) - _delta);
        container->swapChildren(oldPos, newPos);
        assert(newPos == container->getChildIndex(manipulator->getWidget()));

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
        auto container = static_cast<CEGUI::LayoutContainer*>(parentManipulator->getWidget());

        size_t oldPos = container->getChildIndex(manipulator->getWidget());
        size_t newPos = static_cast<size_t>(static_cast<int>(oldPos) + _delta);
        container->swapChildren(oldPos, newPos);
        assert(newPos == container->getChildIndex(manipulator->getWidget()));

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
