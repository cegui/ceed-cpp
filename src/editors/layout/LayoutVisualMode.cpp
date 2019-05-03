#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutEditor.h"
#include "src/editors/layout/LayoutUndoCommands.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/ui/CEGUIWidget.h"
#include "src/ui/CEGUIGraphicsView.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/ui/layout/CreateWidgetDockWidget.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsEntry.h"
#include "src/util/ConfigurableAction.h"
#include "src/Application.h"
#include <CEGUI/GUIContext.h>
#include <CEGUI/WindowManager.h>
#include "qboxlayout.h"
#include "qgraphicsview.h"
#include "qtoolbar.h"
#include "qmenu.h"
#include "qmimedata.h"
#include "qclipboard.h"

LayoutVisualMode::LayoutVisualMode(LayoutEditor& editor)
    : IEditMode(editor)
{
    scene = new LayoutScene(*this);

    hierarchyDockWidget = new WidgetHierarchyDockWidget(*this);
    connect(hierarchyDockWidget, &WidgetHierarchyDockWidget::deleteRequested, [this]()
    {
        if (scene) scene->deleteSelectedWidgets();
    });

    createWidgetDockWidget = new CreateWidgetDockWidget(this);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    const bool continuousRendering = settings->getEntryValue("layout/visual/continuous_rendering").toBool();

    ceguiWidget = new CEGUIWidget(this);
    layout->addWidget(ceguiWidget);
    ceguiWidget->setScene(scene);
    ceguiWidget->setViewFeatures(true, true, continuousRendering);
    ceguiWidget->setInputEnabled(true);

    setupActions();
    setupToolBar();

    hierarchyDockWidget->setupContextMenu();

    auto onSnapGridSettingsChanged = [this]() { snapGridBrushValid = false; };
    connect(settings->getEntry("layout/visual/snap_grid_x"), &SettingsEntry::valueChanged, onSnapGridSettingsChanged);
    connect(settings->getEntry("layout/visual/snap_grid_y"), &SettingsEntry::valueChanged, onSnapGridSettingsChanged);
    connect(settings->getEntry("layout/visual/snap_grid_point_colour"), &SettingsEntry::valueChanged, onSnapGridSettingsChanged);
    connect(settings->getEntry("layout/visual/snap_grid_point_shadow_colour"), &SettingsEntry::valueChanged, onSnapGridSettingsChanged);
}

void LayoutVisualMode::initialize(CEGUI::Window* rootWidget)
{
/*
    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
    mainWindow.propertiesDockWidget.inspector.setPropertyManager(CEGUIWidgetPropertyManager(project.propertyMap, self))
*/
    setRootWidget(rootWidget);

    createWidgetDockWidget->populate();
}

void LayoutVisualMode::setRootWidget(CEGUI::Window* widget)
{
    if (widget)
    {
        auto manipulator = new LayoutManipulator(*this, nullptr, widget);
        manipulator->createChildManipulators(true, false, false);
        setRootWidgetManipulator(manipulator);
    }
    else
        setRootWidgetManipulator(nullptr);
}

void LayoutVisualMode::setRootWidgetManipulator(LayoutManipulator* manipulator)
{
    auto oldRoot = getRootWidget();

    scene->setRootWidgetManipulator(manipulator);
    hierarchyDockWidget->setRootWidgetManipulator(manipulator);

    if (oldRoot) CEGUI::WindowManager::getSingleton().destroyWindow(oldRoot);
}

CEGUI::Window* LayoutVisualMode::getRootWidget() const
{
    auto manip = scene->getRootWidgetManipulator();
    return manip ? manip->getWidget() : nullptr;
}

void LayoutVisualMode::rebuildEditorMenu(QMenu* editorMenu)
{
    // Similar to the toolbar, includes the focus filter box action
    editorMenu->addAction(actionAlignHLeft);
    editorMenu->addAction(actionAlignHCenter);
    editorMenu->addAction(actionAlignHRight);
    editorMenu->addSeparator();
    editorMenu->addAction(actionAlignVTop);
    editorMenu->addAction(actionAlignVCenter);
    editorMenu->addAction(actionAlignVBottom);
    editorMenu->addSeparator();
    editorMenu->addAction(actionSnapGrid);
    editorMenu->addAction(actionAbsoluteMode);
    editorMenu->addAction(actionAbsoluteIntegerMode);
    editorMenu->addSeparator();
    editorMenu->addAction(actionNormalizePosition);
    editorMenu->addAction(actionNormalizeSize);
    editorMenu->addAction(actionRoundPosition);
    editorMenu->addAction(actionRoundSize);
    editorMenu->addSeparator();
    editorMenu->addAction(actionMoveBackward);
    editorMenu->addAction(actionMoveForward);
    editorMenu->addSeparator();
/*
        editorMenu.addAction(self.focusPropertyInspectorFilterBoxAction)
*/
    _editorMenu = editorMenu;
}

void LayoutVisualMode::setActionsEnabled(bool enabled)
{
    actionAlignHLeft->setEnabled(enabled);
    actionAlignHCenter->setEnabled(enabled);
    actionAlignHRight->setEnabled(enabled);
    actionAlignVTop->setEnabled(enabled);
    actionAlignVCenter->setEnabled(enabled);
    actionAlignVBottom->setEnabled(enabled);
    actionNormalizePosition->setEnabled(enabled);
    actionNormalizeSize->setEnabled(enabled);
    actionRoundPosition->setEnabled(enabled);
    actionRoundSize->setEnabled(enabled);
    actionMoveBackward->setEnabled(enabled);
    actionMoveForward->setEnabled(enabled);

    // Weren't in a connection group:
    //actionAbsoluteMode->setEnabled(enabled);
    //actionAbsoluteIntegerMode->setEnabled(enabled);
    //actionSnapGrid->setEnabled(enabled);
}

void LayoutVisualMode::setupActions()
{
    Application* app = qobject_cast<Application*>(qApp);

    auto&& settings = app->getSettings();
    auto category = settings->getCategory("shortcuts");
    if (!category) category = settings->createCategory("shortcuts", "Shortcuts");
    auto section = category->createSection("layout", "Layout Editor");

    auto mainWindow = app->getMainWindow();

    actionAbsoluteMode = new ConfigurableAction(mainWindow,
                                               *section, "absolute_mode", "&Absolute Resizing && Moving Deltas",
                                               "When resizing and moving widgets, if checked this makes the delta absolute, it is relative if unchecked.",
                                               QIcon(":/icons/layout_editing/absolute_mode.png"), QKeySequence(Qt::Key_A));
    actionAbsoluteMode->setCheckable(true);
    actionAbsoluteMode->setChecked(true);

    actionAbsoluteIntegerMode = new ConfigurableAction(mainWindow,
                                               *section, "abs_integers_mode", "Only Increase/Decrease by Integers When Moving or Resizing",
                                               "If checked, while resizing or moving widgets in the editor only integer values (e.g. no"
                                               "0.25 or 0.5 etc.) will be added to the current absolute values. This is only relevant if editing"
                                               "in zoomed-in view while 'Absolute Resizing and Moving' is activated.",
                                               QIcon(":/icons/layout_editing/abs_integers_mode.png"), QKeySequence(Qt::Key_Q));
    actionAbsoluteIntegerMode->setCheckable(true);
    actionAbsoluteIntegerMode->setChecked(true);

    actionSnapGrid = new ConfigurableAction(mainWindow,
                                            *section, "snap_grid", "Snap to &Grid",
                                            "When resizing and moving widgets, if checked this makes sure they snap to a snap grid (see "
                                            "settings for snap grid related entries), also shows the snap grid if checked.",
                                            QIcon(":/icons/layout_editing/snap_grid.png"), QKeySequence(Qt::Key_Space));
    actionSnapGrid->setCheckable(true);

    actionAlignHLeft = new ConfigurableAction(mainWindow,
                                               *section, "align_hleft", "Align &Left (horizontally)",
                                               "Sets horizontal alignment of all selected widgets to left.",
                                               QIcon(":/icons/layout_editing/align_hleft.png"));
    connect(actionAlignHLeft, &ConfigurableAction::triggered, [this]() { scene->alignSelectionHorizontally(CEGUI::HorizontalAlignment::Left); });

    actionAlignHCenter = new ConfigurableAction(mainWindow,
                                               *section, "align_hcentre", "Align Centre (&horizontally)",
                                               "Sets horizontal alignment of all selected widgets to centre.",
                                               QIcon(":/icons/layout_editing/align_hcentre.png"));
    connect(actionAlignHCenter, &ConfigurableAction::triggered, [this]() { scene->alignSelectionHorizontally(CEGUI::HorizontalAlignment::Centre); });

    actionAlignHRight = new ConfigurableAction(mainWindow,
                                               *section, "align_hright", "Align &Right (horizontally)",
                                               "Sets horizontal alignment of all selected widgets to right.",
                                               QIcon(":/icons/layout_editing/align_hright.png"));
    connect(actionAlignHRight, &ConfigurableAction::triggered, [this]() { scene->alignSelectionHorizontally(CEGUI::HorizontalAlignment::Right); });

    actionAlignVTop = new ConfigurableAction(mainWindow,
                                               *section, "align_vtop", "Align &Top (vertically)",
                                               "Sets vertical alignment of all selected widgets to top.",
                                               QIcon(":/icons/layout_editing/align_vtop.png"));
    connect(actionAlignVTop, &ConfigurableAction::triggered, [this]() { scene->alignSelectionVertically(CEGUI::VerticalAlignment::Top); });

    actionAlignVCenter = new ConfigurableAction(mainWindow,
                                               *section, "align_vcentre", "Align Centre (&vertically)",
                                               "Sets vertical alignment of all selected widgets to centre.",
                                               QIcon(":/icons/layout_editing/align_vcentre.png"));
    connect(actionAlignVCenter, &ConfigurableAction::triggered, [this]() { scene->alignSelectionVertically(CEGUI::VerticalAlignment::Centre); });

    actionAlignVBottom = new ConfigurableAction(mainWindow,
                                               *section, "align_vbottom", "Align &Bottom (vertically)",
                                               "Sets vertical alignment of all selected widgets to bottom.",
                                               QIcon(":/icons/layout_editing/align_vbottom.png"));
    connect(actionAlignVBottom, &ConfigurableAction::triggered, [this]() { scene->alignSelectionVertically(CEGUI::VerticalAlignment::Bottom); });

    actionNormalizePosition = new ConfigurableAction(mainWindow,
                                               *section, "normalise_position", "Normalize &Position (cycle)",
                                               "If the position is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                                               QIcon(":/icons/layout_editing/normalise_position.png"), QKeySequence(Qt::Key_D));
    connect(actionNormalizePosition, &ConfigurableAction::triggered, [this]() { scene->normalizePositionOfSelectedWidgets(); });

    actionNormalizeSize = new ConfigurableAction(mainWindow,
                                               *section, "normalise_size", "Normalize &Size (cycle)",
                                               "If the size is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                                               QIcon(":/icons/layout_editing/normalise_size.png"), QKeySequence(Qt::Key_S));
    connect(actionNormalizeSize, &ConfigurableAction::triggered, [this]() { scene->normalizeSizeOfSelectedWidgets(); });

    actionRoundPosition = new ConfigurableAction(mainWindow,
                                               *section, "round_position", "Rounds the absolute position to nearest integer",
                                               "The value of the absolute position will be rounded to the nearest integer value (e.g.: 1.7 will become 2.0 and -4.2 will become -4.0",
                                               QIcon(":/icons/layout_editing/round_position.png"), QKeySequence(Qt::Key_M));
    connect(actionRoundPosition, &ConfigurableAction::triggered, [this]() { scene->roundPositionOfSelectedWidgets(); });

    actionRoundSize = new ConfigurableAction(mainWindow,
                                               *section, "round_size", "Rounds the absolute size to nearest integer",
                                               "The value of the absolute size will be rounded to the nearest integer value (e.g.: 1.7 will become 2.0 and -4.2 will become -4.0",
                                               QIcon(":/icons/layout_editing/round_size.png"), QKeySequence(Qt::Key_N));
    connect(actionRoundSize, &ConfigurableAction::triggered, [this]() { scene->roundSizeOfSelectedWidgets(); });

    actionMoveBackward = new ConfigurableAction(mainWindow,
                                               *section, "move_backward_in_parent_list", "Moves widget -1 step in the parent's widget list",
                                               "Moves selected widget(s) one step backward in their parent's widget list (Only applicable to SequentialLayoutContainer widgets, VerticalLayoutContainer and HorizontalLayoutContainer in particular.)",
                                               QIcon(":/icons/layout_editing/move_backward_in_parent_list.png"));
    connect(actionMoveBackward, &ConfigurableAction::triggered, [this]() { scene->moveSelectedWidgetsInParentWidgetLists(-1); });

    actionMoveForward = new ConfigurableAction(mainWindow,
                                               *section, "move_forward_in_parent_list", "Moves widget +1 step in the parent's widget list",
                                               "Moves selected widget(s) one step forward in their parent's widget list (Only applicable to SequentialLayoutContainer widgets, VerticalLayoutContainer and HorizontalLayoutContainer in particular.)",
                                               QIcon(":/icons/layout_editing/move_forward_in_parent_list.png"));
    connect(actionMoveForward, &ConfigurableAction::triggered, [this]() { scene->moveSelectedWidgetsInParentWidgetLists(+1); });

    /*
        cat.createAction(name = "focus_property_inspector_filter_box", label = "&Focus Property Inspector Filter Box",
                         help_ = "This allows you to easily press a shortcut and immediately search through properties without having to reach for a mouse.",
                         icon = QtGui.QIcon("icons/layout_editing/focus_property_inspector_filter_box.png"),
                         defaultShortcut = QtGui.QKeySequence(QtGui.QKeySequence.Find))

        self.focusPropertyInspectorFilterBoxAction = action.getAction("layout/focus_property_inspector_filter_box")
        self.connectionGroup.add(self.focusPropertyInspectorFilterBoxAction, receiver = lambda: self.focusPropertyInspectorFilterBox())
    */

    /*
        //???to corresponding widget?
        cat.createAction(name = "copy_widget_path", label = "C&opy Widget Paths",
                         help_ = "Copies the 'NamePath' properties of the selected widgets to the clipboard.",
                         icon = QtGui.QIcon("icons/actions/copy.png"))

        cat.createAction(name = "rename", label = "&Rename Widget",
                         help_ = "Edits the selected widget's name.",
                         icon = QtGui.QIcon("icons/layout_editing/rename.png"))

        cat.createAction(name = "lock_widget", label = "&Lock Widget",
                         help_ = "Locks the widget for moving and resizing in the visual editing mode.",
                         icon = QtGui.QIcon("icons/layout_editing/lock_widget.png"))

        cat.createAction(name = "unlock_widget", label = "&Unlock Widget",
                         help_ = "Unlocks the widget for moving and resizing in the visual editing mode.",
                         icon = QtGui.QIcon("icons/layout_editing/unlock_widget.png"))

        cat.createAction(name = "recursively_lock_widget", label = "&Lock Widget (recursively)",
                         help_ = "Locks the widget and all its child widgets for moving and resizing in the visual editing mode.",
                         icon = QtGui.QIcon("icons/layout_editing/lock_widget_recursively.png"))

        cat.createAction(name = "recursively_unlock_widget", label = "&Unlock Widget (recursively)",
                         help_ = "Unlocks the widget and all its child widgets for moving and resizing in the visual editing mode.",
                         icon = QtGui.QIcon("icons/layout_editing/unlock_widget_recursively.png"))
    */
}

void LayoutVisualMode::setupToolBar()
{
    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
    toolBar = mainWindow->createToolbar("Layout");
    toolBar->addAction(actionAlignHLeft);
    toolBar->addAction(actionAlignHCenter);
    toolBar->addAction(actionAlignHRight);
    toolBar->addSeparator();
    toolBar->addAction(actionAlignVTop);
    toolBar->addAction(actionAlignVCenter);
    toolBar->addAction(actionAlignVBottom);
    toolBar->addSeparator();
    toolBar->addAction(actionSnapGrid);
    toolBar->addAction(actionAbsoluteMode);
    toolBar->addAction(actionAbsoluteIntegerMode);
    toolBar->addSeparator();
    toolBar->addAction(actionNormalizePosition);
    toolBar->addAction(actionNormalizeSize);
    toolBar->addAction(actionRoundPosition);
    toolBar->addAction(actionRoundSize);
    toolBar->addSeparator();
    toolBar->addAction(actionMoveBackward);
    toolBar->addAction(actionMoveForward);
}

//!!!???to PropertyWidget / PropertyDockWidget?
void LayoutVisualMode::focusPropertyInspectorFilterBox()
{
/*
        """Focuses into property set inspector filter

        This potentially allows the user to just press a shortcut to find properties to edit,
        instead of having to reach for a mouse.
        """

        auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
        filterBox = mainWindow.propertiesDockWidget.inspector.filterBox
        # selects all contents of the filter so that user can replace that with their search phrase
        filterBox.selectAll()
        # sets focus so that typing puts text into the filter box without clicking
        filterBox.setFocus()
*/
}

bool LayoutVisualMode::cut()
{
    if (!copy()) return false;
    scene->deleteSelectedWidgets();
    return true;
}

bool LayoutVisualMode::copy()
{
    std::vector<LayoutManipulator*> topMostSelected;
    for (QGraphicsItem* item : scene->selectedItems())
    {
        auto manip = dynamic_cast<LayoutManipulator*>(item);
        if (!manip) continue;

        bool hasAncestorSelected = false;
        for (QGraphicsItem* item2 : scene->selectedItems())
        {
            auto manip2 = dynamic_cast<LayoutManipulator*>(item2);
            if (!manip2 || manip == manip2) continue;

            if (manip2->isAncestorOf(manip))
            {
                hasAncestorSelected = true;
                break;
            }
        }

        if (!hasAncestorSelected) topMostSelected.push_back(manip);
    }

    if (topMostSelected.empty()) return false;

    // Now we serialize the topmost selected widgets (and thus their entire hierarchies)
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    for (LayoutManipulator* manipulator : topMostSelected)
    {
        if (!CEGUIUtils::serializeWidget(*manipulator->getWidget(), stream, true))
            return false;
    }

    if (!bytes.size()) return false;

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("application/x-ceed-widget-hierarchy-list", bytes);
    QApplication::clipboard()->setMimeData(mimeData);

    return true;
}

bool LayoutVisualMode::paste()
{
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData->hasFormat("application/x-ceed-widget-hierarchy-list")) return false;
    QByteArray bytes = mimeData->data("application/x-ceed-widget-hierarchy-list");
    if (bytes.size() <= 0) return false;

    LayoutManipulator* target = nullptr;
    for (QGraphicsItem* item : scene->selectedItems())
    {
        auto manipulator = dynamic_cast<LayoutManipulator*>(item);
        if (!manipulator) continue;

        // Multiple targets, we can't decide!
        if (target) return false;

        target = manipulator;
    }

    if (!target) return false;

    _editor.getUndoStack()->push(new LayoutPasteCommand(*this, target->getWidgetPath(), std::move(bytes)));

    return true;
}

bool LayoutVisualMode::deleteSelected()
{
    return scene->deleteSelectedWidgets();
}

void LayoutVisualMode::zoomIn()
{
    ceguiWidget->getView()->zoomIn();
}

void LayoutVisualMode::zoomOut()
{
    ceguiWidget->getView()->zoomOut();
}

void LayoutVisualMode::zoomReset()
{
    ceguiWidget->getView()->zoomReset();
}

// Retrieves a (cached) snap grid brush
const QBrush& LayoutVisualMode::getSnapGridBrush() const
{
    if (snapGridBrushValid) return snapGridBrush;

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();

    auto snapGridX = settings->getEntryValue("layout/visual/snap_grid_x").toInt();
    auto snapGridY = settings->getEntryValue("layout/visual/snap_grid_y").toInt();
    QColor snapGridPointColour = settings->getEntryValue("layout/visual/snap_grid_point_colour").value<QColor>();
    QColor snapGridPointShadowColour = settings->getEntryValue("layout/visual/snap_grid_point_shadow_colour").value<QColor>();

    // If snap grid wasn't created yet or if it's parameters changed, create it anew!

    QPixmap texture(snapGridX, snapGridY);
    texture.fill(QColor(Qt::transparent));

    QPainter painter(&texture);
    painter.setPen(QPen(snapGridPointColour));
    painter.drawPoint(0, 0);
    painter.setPen(QPen(snapGridPointShadowColour));
    painter.drawPoint(1, 0);
    painter.drawPoint(1, 1);
    painter.drawPoint(0, 1);
    painter.end();

    snapGridBrush.setTexture(texture);

    snapGridBrushValid = true;

    return snapGridBrush;
}

bool LayoutVisualMode::isAbsoluteMode() const
{
    return actionAbsoluteMode ? actionAbsoluteMode->isChecked() : true;
}

bool LayoutVisualMode::isAbsoluteIntegerMode() const
{
    return actionAbsoluteIntegerMode ? actionAbsoluteIntegerMode->isChecked() : true;
}

bool LayoutVisualMode::isSnapGridEnabled() const
{
    return actionSnapGrid ? actionSnapGrid->isChecked() : false;
}

void LayoutVisualMode::showEvent(QShowEvent* event)
{
    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();

    hierarchyDockWidget->setEnabled(true);
    mainWindow->getPropertyDockWidget()->setEnabled(true);
    createWidgetDockWidget->setEnabled(true);
    toolBar->setEnabled(true);

    //???signal from editor to main window instead of storing ptr here?
    if (_editorMenu) _editorMenu->menuAction()->setEnabled(true);

    // Make sure all the manipulators are in sync to matter what
    // this is there mainly for the situation when you switch to live preview, then change resolution, then switch
    // back to visual editing and all manipulators are of different size than they should be
    scene->updateFromWidgets();

    setActionsEnabled(true);

    QWidget::showEvent(event);
}

void LayoutVisualMode::hideEvent(QHideEvent* event)
{
    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();

    setActionsEnabled(false);

    hierarchyDockWidget->setEnabled(false);
    mainWindow->getPropertyDockWidget()->setEnabled(false);
    createWidgetDockWidget->setEnabled(false);
    toolBar->setEnabled(false);

    //???signal from editor to main window instead of storing ptr here?
    if (_editorMenu) _editorMenu->menuAction()->setEnabled(false);

    QWidget::hideEvent(event);
}


/*
from ceed.propertysetinspector import PropertyInspectorWidget
from ceed.propertysetinspector import CEGUIPropertyManager
import ceed.propertytree as pt
from ceed.propertytree.editors import PropertyEditorRegistry


class WidgetMultiPropertyWrapper(pt.properties.MultiPropertyWrapper):
    """Overrides the default MultiPropertyWrapper to update the 'inner properties'
    and then create undo commands to update the CEGUI widgets.
    """

    def __init__(self, templateProperty, innerProperties, takeOwnership, ceguiProperty=None, ceguiSets=None, visual=None):
        super(WidgetMultiPropertyWrapper, self).__init__(templateProperty, innerProperties, takeOwnership)

        self.ceguiProperty = ceguiProperty
        self.ceguiSets = ceguiSets
        self.visual = visual

    def tryUpdateInner(self, newValue, reason=pt.properties.Property.ChangeValueReason.Unknown):
        if super(WidgetMultiPropertyWrapper, self).tryUpdateInner(newValue, reason):
            ceguiValue = unicode(newValue)

            # create and execute command
            widgetPaths = []
            undoOldValues = {}

            # set the properties where applicable
            for ceguiSet in self.ceguiSets:
                widgetPath = ceguiSet.getNamePath()
                widgetPaths.append(widgetPath)
                undoOldValues[widgetPath] = self.ceguiProperty.get(ceguiSet)

            # create the undoable command
            # but tell it not to trigger the property changed callback
            # on first run because our on value has already changed,
            # we just want to sync the widget value now.
            cmd = undo.PropertyEditCommand(self.visual, self.ceguiProperty.getName(), widgetPaths, undoOldValues, ceguiValue,
                                           ignoreNextPropertyManagerCallback = True)
            self.visual.tabbedEditor.undoStack.push(cmd)

            # make sure to redraw the scene to preview the property
            self.visual.scene.update()

            return True

        return False


class CEGUIWidgetPropertyManager(CEGUIPropertyManager):
    """Customises the CEGUIPropertyManager by binding to a 'visual'
    so it can manipulate the widgets via undo commands.

    It also customises the sorting of the categories.
    """

    def __init__(self, propertyMap, visual):
        super(CEGUIWidgetPropertyManager, self).__init__(propertyMap)
        self.visual = visual

    def createProperty(self, ceguiProperty, ceguiSets):
        prop = super(CEGUIWidgetPropertyManager, self).createProperty(ceguiProperty, ceguiSets, self.propertyMap, WidgetMultiPropertyWrapper)
        prop.ceguiProperty = ceguiProperty
        prop.ceguiSets = ceguiSets
        prop.visual = self.visual

        return prop

    def buildCategories(self, ceguiPropertySets):
        categories = super(CEGUIWidgetPropertyManager, self).buildCategories(ceguiPropertySets)

        # sort categories by name but keep some special categories on top
        def getSortKey(t):
            name, _ = t

            if name == "Element":
                return "000Element"
            elif name == "NamedElement":
                return "001NamedElement"
            elif name == "Window":
                return "002Window"
            elif name.startswith("CEGUI/"):
                return name[6:]
            elif name == "Unknown":
                return "ZZZUnknown"
            else:
                return name

        categories = OrderedDict(sorted(categories.items(), key=getSortKey))

        return categories
*/
