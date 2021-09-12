#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/WidgetHierarchyItem.h"
#include "src/cegui/CEGUIUtils.h"
#include <CEGUI/widgets/GridLayoutContainer.h>
#include <CEGUI/WindowManager.h>
#include <CEGUI/CoordConverter.h>
#include <qtreeview.h>
#include <qmessagebox.h>
#include <qtimer.h>

static LayoutManipulator* CreateManipulatorFromDataStream(LayoutVisualMode& visualMode, LayoutManipulator* parent,
                                                          QDataStream& stream, size_t index = std::numeric_limits<size_t>().max())
{
    LayoutManipulator* manipulator;

    if (parent)
    {
        CEGUI::Window* widget = CEGUIUtils::deserializeWidget(stream, parent->getWidget(), index);
        assert(widget);
        if (!widget) return nullptr;

        // Insertion of the new child into GLC might result in its growing
        if (auto glc = dynamic_cast<CEGUI::GridLayoutContainer*>(parent->getWidget()))
        {
            if (glc->isAutoGrowing())
                parent->updatePropertiesFromWidget({"GridWidth", "GridHeight"});
        }

        manipulator = parent->createChildManipulator(widget);
    }
    else
    {
        // No parent, root widget
        CEGUI::Window* widget = CEGUIUtils::deserializeWidget(stream, nullptr);
        assert(widget);
        if (!widget) return nullptr;

        manipulator = new LayoutManipulator(visualMode, nullptr, widget);
        visualMode.setRootWidgetManipulator(manipulator);
    }

    manipulator->createChildManipulators(true, false, true);
    manipulator->updateFromWidget(false, true);
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
        CEGUIUtils::setWidgetArea(manipulator->getWidget(), rec.oldPos, rec.oldSize);
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
        CEGUIUtils::setWidgetArea(manipulator->getWidget(), rec.newPos, rec.newSize);
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
    CEGUIUtils::removeNestedPaths(paths);

    for (const QString& path : paths)
    {
        auto manipulator = _visualMode.getScene()->getManipulatorByPath(path);
        if (!manipulator)
        {
            assert(false);
            continue;
        }

        Record rec;
        rec.path = path;
        rec.indexInParent = manipulator->getWidgetIndexInParent();

        // Serialize deleted hierarchy for undo
        QDataStream stream(&rec.data, QIODevice::WriteOnly);
        CEGUIUtils::serializeWidget(*manipulator->getWidget(), stream, true);

        _records.push_back(std::move(rec));
    }

    // Sort by indexInParent asc for insertion in undo
    std::sort(_records.begin(), _records.end(), [](const Record& a, const Record& b)
    {
        return a.indexInParent < b.indexInParent;
    });

    if (paths.size() == 1)
        setText(QString("Delete '%1'").arg(paths[0]));
    else
        setText(QString("Delete %1 widgets'").arg(paths.size()));
}

void LayoutDeleteCommand::undo()
{
    QUndoCommand::undo();

    for (auto& rec : _records)
    {
        const int sepPos = rec.path.lastIndexOf('/');
        LayoutManipulator* parent = (sepPos < 0) ? nullptr : _visualMode.getScene()->getManipulatorByPath(rec.path.left(sepPos));

        QDataStream stream(&rec.data, QIODevice::ReadOnly);
        CreateManipulatorFromDataStream(_visualMode, parent, stream, rec.indexInParent);
    }

    _visualMode.getHierarchyDockWidget()->refresh();
}

void LayoutDeleteCommand::redo()
{
    for (const auto& rec : _records)
        _visualMode.getScene()->deleteWidgetByPath(rec.path);

    _visualMode.getScene()->updatePropertySet();

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

LayoutCreateCommand::LayoutCreateCommand(LayoutVisualMode& visualMode, const QString& parentPath,
                                         const QString& type, size_t indexInParent)
    : _visualMode(visualMode)
    , _parentPath(parentPath)
    , _type(type)
    , _indexInParent(indexInParent)
{
    generateName();
    setText(QString("Create '%1' of type '%2'").arg(_name, type));
}

LayoutCreateCommand::LayoutCreateCommand(LayoutVisualMode& visualMode, const QString& parentPath,
                                         const QString& type, QPointF scenePos, size_t indexInParent)
    : _visualMode(visualMode)
    , _parentPath(parentPath)
    , _type(type)
    , _scenePos(scenePos)
    , _indexInParent(indexInParent)
    , _useScenePos(true)
{
    generateName();
    setText(QString("Create '%1' of type '%2' at (%3; %4)").arg(_name).arg(type).arg(scenePos.x()).arg(scenePos.y()));
}

void LayoutCreateCommand::generateName()
{
    if (_type == "DefaultWindow" && _parentPath.isEmpty())
    {
        // Special case - root widget. Setup it with most useful parameters.
        // Naming convention is from docs:
        // http://static.cegui.org.uk/docs/0.8.7/window_tutorial.html
        _name = "root";
        // can instead use: widgetName = QFileInfo(_visualMode.getEditor().getFilePath()).baseName();
    }
    else
    {
        const int sepPos = _type.lastIndexOf('/');
        _name = (sepPos < 0) ? _type : _type.mid(sepPos + 1);
        if (!_parentPath.isEmpty())
        {
            LayoutManipulator* parent = _visualMode.getScene()->getManipulatorByPath(_parentPath);
            _name = CEGUIUtils::getUniqueChildWidgetName(*parent->getWidget(), _name);
        }
    }
}

void LayoutCreateCommand::undo()
{
    QUndoCommand::undo();
    _visualMode.getScene()->deleteWidgetByPath(_fullPath);
    _visualMode.getScene()->updatePropertySet();
}

void LayoutCreateCommand::redo()
{
    // Most of (but not all) widgets require a font to be rendered properly
    _visualMode.getScene()->ensureDefaultFontExists();

    CEGUI::Window* widget = CEGUI::WindowManager::getSingleton().createWindow(
                CEGUIUtils::qStringToString(_type), CEGUIUtils::qStringToString(_name));

    // Disable maximum size by default
    widget->setMaxSize(CEGUI::USize(CEGUI::UDim(0.f, 0.f), CEGUI::UDim(0.f, 0.f)));

    LayoutManipulator* parent = _parentPath.isEmpty() ? nullptr :
                _visualMode.getScene()->getManipulatorByPath(_parentPath);

    // Setup position and size of the new widget
    if (_type == "DefaultWindow" && !parent)
    {
        // Special case - root widget. Setup it with most useful parameters.
        CEGUIUtils::setWidgetArea(widget,
                                  CEGUI::UVector2(CEGUI::UDim(0.f, 0.f), CEGUI::UDim(0.f, 0.f)),
                                  CEGUI::USize(CEGUI::UDim(1.f, 0.f), CEGUI::UDim(1.f, 0.f)));
        widget->setCursorPassThroughEnabled(true);
    }
    else
    {
        glm::vec2 pos(0.f, 0.f);
        if (_useScenePos)
        {
            // Convert requested position into parent cordinate system
            pos = glm::vec2(static_cast<float>(_scenePos.x()), static_cast<float>(_scenePos.y()));
            if (parent)
                pos -= parent->getWidget()->getChildContentArea(widget->isNonClient()).get().getPosition();
        }

        // If the size is 0x0, the widget will be hard to deal with, lets fix that in that case
        const auto size = (widget->getSize() == CEGUI::USize(CEGUI::UDim(0.f, 0.f), CEGUI::UDim(0.f, 0.f))) ?
                    CEGUI::USize(CEGUI::UDim(0.f, 50.f), CEGUI::UDim(0.f, 50.f)) :
                    widget->getSize();

        // Place new window at the requested point in context coords and fix the size if required
        CEGUIUtils::setWidgetArea(widget,
                                  CEGUI::UVector2(CEGUI::UDim(0.f, pos.x), CEGUI::UDim(0.f, pos.y)),
                                  size);
    }

    // Create GLC capable to accept children
    if (auto glc = dynamic_cast<CEGUI::GridLayoutContainer*>(widget))
    {
        glc->setAutoGrowing(true);
        glc->setRowMajor(true);
        glc->setGridWidth(2);
    }

    LayoutManipulator* manipulator;
    if (parent)
    {
        // Insert first to get valid GUI context for the widget
        CEGUIUtils::insertChild(parent->getWidget(), widget, _indexInParent);
        manipulator = parent->createChildManipulator(widget);

        // Insertion of the new child into GLC might result in its growing
        if (auto glc = dynamic_cast<CEGUI::GridLayoutContainer*>(parent->getWidget()))
        {
            if (glc->isAutoGrowing())
                parent->updatePropertiesFromWidget({"GridWidth", "GridHeight"});
        }
    }
    else
    {
        manipulator = new LayoutManipulator(_visualMode, nullptr, widget);
        _visualMode.setRootWidgetManipulator(manipulator);
    }

    manipulator->updateFromWidget(true, true);
    manipulator->createChildManipulators(true, false, true);

    // Make only the new widget selected. It is moved to front inside setSelected().
    _visualMode.getScene()->clearSelection();
    _visualMode.getHierarchyDockWidget()->getTreeView()->clearSelection();
    manipulator->setSelected(true);

    _visualMode.getHierarchyDockWidget()->refresh();

    // Stored for undo(), because we can't be sure it is equal to _parentPath + '/' + _name.
    // E.g. TabControl adds its children into an auto content pane.
    _fullPath = manipulator->getWidgetPath();

    QUndoCommand::redo();
}

//---------------------------------------------------------------------

LayoutPropertyEditCommand::LayoutPropertyEditCommand(LayoutVisualMode& visualMode, std::vector<Record>&& records,
                                                     const QString& propertyName, size_t multiChangeId)
    : _visualMode(visualMode)
    , _records(std::move(records))
    , _propertyName(CEGUIUtils::qStringToString(propertyName))
    , _multiChangeId(multiChangeId)
    , _invalidValue(false)
{
    refreshText();
}

void LayoutPropertyEditCommand::undo()
{
    QUndoCommand::undo();

    QStringList properties;
    fillInfluencedPropertyList(properties);

    for (const auto& rec : _records)
        setProperty(rec.path, rec.oldValue, properties);
}

void LayoutPropertyEditCommand::redo()
{
    QStringList properties;
    fillInfluencedPropertyList(properties);

    for (const auto& rec : _records)
        setProperty(rec.path, rec.newValue, properties);

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

void LayoutPropertyEditCommand::setProperty(const QString& widgetPath, const CEGUI::String& value, const QStringList& propertiesToUpdate)
{
    auto manipulator = _visualMode.getScene()->getManipulatorByPath(widgetPath);
    assert(manipulator);

    if (!manipulator || manipulator->getWidget()->getProperty(_propertyName) == value) return;

    try
    {
        CEGUIUtils::setWidgetProperty(manipulator->getWidget(), _propertyName, value);
        manipulator->updateFromWidget(false, true);
        manipulator->update();
        manipulator->updatePropertiesFromWidget(propertiesToUpdate);
        _invalidValue = false;
    }
    catch (const std::exception& e)
    {
        _invalidValue = true;

        // Restore previous value
        manipulator->updatePropertiesFromWidget({ CEGUIUtils::stringToQString(_propertyName) });

        // Synchronous message box leads to a crash here
        QString reason = e.what();
        QTimer::singleShot(0, [reason]()
        {
            QMessageBox::warning(nullptr, "Can't set property", reason);
        });
    }
}

// Some properties are related to others so that when one changes, the others change too.
// The following ensures that notifications are sent about the related properties as well.
void LayoutPropertyEditCommand::fillInfluencedPropertyList(QStringList& list)
{
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

void LayoutPropertyEditCommand::refreshText()
{
    const QString propertyName = CEGUIUtils::stringToQString(_propertyName);
    if (_records.size() == 1)
        setText(QString("Change '%1' in '%2'").arg(propertyName, _records[0].path));
    else
        setText(QString("Change '%1' in %2 widgets").arg(propertyName).arg(_records.size()));
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
    // Only one root widget can exist, and it can't be reparented
    _records.erase(std::remove_if(_records.begin(), _records.end(), [this](const Record& rec)
    {
        return !_visualMode.getScene()->getManipulatorByPath(rec.oldParentPath);
    }), _records.end());

    // Sort by old child index to maintain predictable insertion order.
    // Otherwise it would depend on the order in which user selected items.
    std::sort(_records.begin(), _records.end(), [this](const Record& a, const Record& b)
    {
        // Inside a parent sort by index asc
        if (a.oldParentPath == b.oldParentPath)
            return a.oldChildIndex < b.oldChildIndex;

        // Force the target parent be the first processed
        if (a.oldParentPath == _newParentPath) return true;
        if (b.oldParentPath == _newParentPath) return false;

        // Otherwise sort by parent, no much difference in what exact order
        return a.oldParentPath < b.oldParentPath;
    });

    // Fix indices to take into account offsets that happen during the command execution.
    // We have only one target widget, it simplifies things a lot.
    size_t beforeTargetPos = 0;
    size_t afterTargetPos = 0;
    for (auto& rec : _records)
    {
        if (rec.oldParentPath == _newParentPath && rec.oldChildIndex < rec.newChildIndex)
        {
            rec.oldChildIndex -= beforeTargetPos;
            ++beforeTargetPos;
        }
        else
        {
            // All items from other parents are inserted after our own moved ones
            rec.newChildIndex += afterTargetPos;
            ++afterTargetPos;
        }

        // Remember initial position and size
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

    for (auto it = _records.rbegin(); it != _records.rend(); ++it)
    {
        const auto& rec = *it;
        auto widgetManipulator = _visualMode.getScene()->getManipulatorByPath(_newParentPath + '/' + rec.newName);
        auto newParentManipulator = dynamic_cast<LayoutManipulator*>(widgetManipulator->parentItem());
        auto oldParentManipulator = _visualMode.getScene()->getManipulatorByPath(rec.oldParentPath);

        // Remove it from the current CEGUI parent widget
        if (oldParentManipulator != newParentManipulator)
            CEGUIUtils::removeChild(widgetManipulator->getWidget());

        // Rename it if necessary
        if (rec.oldName != rec.newName)
        {
            widgetManipulator->getWidget()->setName(CEGUIUtils::qStringToString(rec.oldName));
            widgetManipulator->updatePropertiesFromWidget({"Name"});
        }

        // Restore initial position and size if necessary
        // TODO: is LC the only possible reason of pos & size change when reparenting?
        //if (newParentManipulator->isLayoutContainer())
        {
            CEGUIUtils::setWidgetArea(widgetManipulator->getWidget(), rec.oldPos, rec.oldSize);
            widgetManipulator->updatePropertiesFromWidget({"Size", "Position", "Area"});
        }

        if (oldParentManipulator != newParentManipulator)
        {
            // Add it to the old CEGUI parent widget and sort out the manipulators
            CEGUIUtils::addChild(oldParentManipulator->getWidget(), widgetManipulator->getWidget());
            widgetManipulator->setParentItem(oldParentManipulator);
        }

        const size_t currIndex = widgetManipulator->getWidgetIndexInParent();
        const size_t destIndex = rec.oldChildIndex > currIndex ? rec.oldChildIndex + 1 : rec.oldChildIndex;
        if (destIndex <= oldParentManipulator->getWidget()->getChildCount())
            oldParentManipulator->getWidget()->moveChildToIndex(currIndex, destIndex);

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
        auto oldParentManipulator = dynamic_cast<LayoutManipulator*>(widgetManipulator->parentItem());
        auto newParentManipulator = _visualMode.getScene()->getManipulatorByPath(_newParentPath);

        // Remove it from the current CEGUI parent widget
        if (oldParentManipulator != newParentManipulator)
            CEGUIUtils::removeChild(widgetManipulator->getWidget());

        // Rename it if necessary
        if (rec.oldName != rec.newName)
        {
            widgetManipulator->getWidget()->setName(CEGUIUtils::qStringToString(rec.newName));
            widgetManipulator->updatePropertiesFromWidget({"Name"});
        }

        if (oldParentManipulator != newParentManipulator)
        {
            // Add it to the new CEGUI parent widget and sort out the manipulators
            CEGUIUtils::addChild(newParentManipulator->getWidget(), widgetManipulator->getWidget());
            widgetManipulator->setParentItem(newParentManipulator);

            // Insertion of the new child into GLC might result in its growing
            if (auto glc = dynamic_cast<CEGUI::GridLayoutContainer*>(newParentManipulator->getWidget()))
            {
                if (glc->isAutoGrowing())
                    newParentManipulator->updatePropertiesFromWidget({"GridWidth", "GridHeight"});
            }
        }

        if (rec.newChildIndex <= newParentManipulator->getWidget()->getChildCount())
            newParentManipulator->getWidget()->moveChildToIndex(widgetManipulator->getWidget(), rec.newChildIndex);

        // Update widget and its previous parent (the second is mostly for the layout container case)
        widgetManipulator->updateFromWidget(true, true);
        oldParentManipulator->updateFromWidget(true, true);
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

    for (const QString& path : _createdWidgets)
        _visualMode.getScene()->deleteWidgetByPath(path);

    _visualMode.getScene()->updatePropertySet();

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

        if (auto manipulator = CreateManipulatorFromDataStream(_visualMode, target, stream))
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

    const QString fullPath = _parentPath.isEmpty() ? _newName : _parentPath + '/' + _newName;
    auto manipulator = _visualMode.getScene()->getManipulatorByPath(fullPath);
    manipulator->getWidget()->setName(CEGUIUtils::qStringToString(_oldName));
    manipulator->updatePropertiesFromWidget({"Name"});
}

void LayoutRenameCommand::redo()
{
    const QString fullPath = _parentPath.isEmpty() ? _oldName : _parentPath + '/' + _oldName;
    auto manipulator = _visualMode.getScene()->getManipulatorByPath(fullPath);
    manipulator->getWidget()->setName(CEGUIUtils::qStringToString(_newName));
    manipulator->updatePropertiesFromWidget({"Name"});

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

        size_t oldPos = parentManipulator->getWidget()->getChildIndex(manipulator->getWidget());
        size_t newPos = static_cast<size_t>(static_cast<int>(oldPos) - _delta);
        parentManipulator->getWidget()->swapChildren(oldPos, newPos);
        assert(newPos == parentManipulator->getWidget()->getChildIndex(manipulator->getWidget()));

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

        size_t oldPos = parentManipulator->getWidget()->getChildIndex(manipulator->getWidget());
        size_t newPos = static_cast<size_t>(static_cast<int>(oldPos) + _delta);
        parentManipulator->getWidget()->swapChildren(oldPos, newPos);
        assert(newPos == parentManipulator->getWidget()->getChildIndex(manipulator->getWidget()));

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
