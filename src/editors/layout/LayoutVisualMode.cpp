#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutEditor.h"
#include "src/ui/layout/LayoutScene.h"
#include "src/ui/layout/CreateWidgetDockWidget.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/util/Settings.h"
#include "src/Application.h"
#include "qboxlayout.h"
#include "qgraphicsview.h"
#include "qtoolbar.h"
#include "qmenu.h"

LayoutVisualMode::LayoutVisualMode(LayoutEditor& editor)
    : IEditMode(editor)
{
    hierarchyDockWidget = new WidgetHierarchyDockWidget(this);
    connect(hierarchyDockWidget, &WidgetHierarchyDockWidget::deleteRequested, [this]()
    {
        if (scene) scene->deleteSelectedWidgets();
    });

    createWidgetDockWidget = new CreateWidgetDockWidget(this);

    propertiesDockWidget = new QDockWidget(this);
    propertiesDockWidget->setObjectName("PropertiesDockWidget");
    propertiesDockWidget->setWindowTitle("Selection Properties");
    // Make the dock take as much space as it can vertically
    propertiesDockWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

/*
    inspector = PropertyInspectorWidget()
    inspector.ptree.setupRegistry(PropertyEditorRegistry(True))
    propertiesDockWidget->setWidget(inspector)
*/

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    scene = new LayoutScene(*this);

    setupActions();
    setupToolBar();
/*
        self.hierarchyDockWidget.treeView.setupContextMenu()

        self.oldViewState = None
*/
}

// arg rootWidget
void LayoutVisualMode::initialize()
{
/*
        pmap = mainwindow.MainWindow.instance.project.propertyMap
        self.propertiesDockWidget.inspector.setPropertyManager(CEGUIWidgetPropertyManager(pmap, self))

        self.setRootWidget(rootWidget)
*/
    createWidgetDockWidget->populate();
}

void LayoutVisualMode::rebuildEditorMenu(QMenu* editorMenu)
{
    // Similar to the toolbar, includes the focus filter box action
/*
        editorMenu.addAction(self.alignHLeftAction)
        editorMenu.addAction(self.alignHCentreAction)
        editorMenu.addAction(self.alignHRightAction)
        editorMenu.addSeparator() # ---------------------------
        editorMenu.addAction(self.alignVTopAction)
        editorMenu.addAction(self.alignVCentreAction)
        editorMenu.addAction(self.alignVBottomAction)
        editorMenu.addSeparator() # ---------------------------
        editorMenu.addAction(action.getAction("layout/snap_grid"))
        editorMenu.addAction(action.getAction("layout/absolute_mode"))
        editorMenu.addAction(action.getAction("layout/abs_integers_mode"))
        editorMenu.addAction(action.getAction("layout/normalise_position"))
        editorMenu.addAction(action.getAction("layout/normalise_size"))
        editorMenu.addAction(action.getAction("layout/round_position"))
        editorMenu.addAction(action.getAction("layout/round_size"))
        editorMenu.addSeparator() # ---------------------------
        editorMenu.addAction(action.getAction("layout/move_backward_in_parent_list"))
        editorMenu.addAction(action.getAction("layout/move_forward_in_parent_list"))
        editorMenu.addSeparator() # ---------------------------
        editorMenu.addAction(self.focusPropertyInspectorFilterBoxAction)
*/
    _editorMenu = editorMenu;
}

void LayoutVisualMode::setRootWidgetManipulator(LayoutManipulator* manipulator)
{
/*
        oldRoot = self.getCurrentRootWidget()
*/

    scene->setRootWidgetManipulator(manipulator);
    hierarchyDockWidget->setRootWidgetManipulator(manipulator);

/*
        PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(self.getCurrentRootWidget())

        if oldRoot:
            PyCEGUI.WindowManager.getSingleton().destroyWindow(oldRoot)

        # cause full redraw of the default GUI context to ensure nothing gets stuck
        PyCEGUI.System.getSingleton().getDefaultGUIContext().markAsDirty()
*/
}

void LayoutVisualMode::setupActions()
{
/*
        self.connectionGroup = action.ConnectionGroup(action.ActionManager.instance)

        # horizontal alignment actions
        self.alignHLeftAction = action.getAction("layout/align_hleft")
        self.connectionGroup.add(self.alignHLeftAction, receiver = lambda: self.scene.alignSelectionHorizontally(PyCEGUI.HA_LEFT))
        self.alignHCentreAction = action.getAction("layout/align_hcentre")
        self.connectionGroup.add(self.alignHCentreAction, receiver = lambda: self.scene.alignSelectionHorizontally(PyCEGUI.HA_CENTRE))
        self.alignHRightAction = action.getAction("layout/align_hright")
        self.connectionGroup.add(self.alignHRightAction, receiver = lambda: self.scene.alignSelectionHorizontally(PyCEGUI.HA_RIGHT))

        # vertical alignment actions
        self.alignVTopAction = action.getAction("layout/align_vtop")
        self.connectionGroup.add(self.alignVTopAction, receiver = lambda: self.scene.alignSelectionVertically(PyCEGUI.VA_TOP))
        self.alignVCentreAction = action.getAction("layout/align_vcentre")
        self.connectionGroup.add(self.alignVCentreAction, receiver = lambda: self.scene.alignSelectionVertically(PyCEGUI.VA_CENTRE))
        self.alignVBottomAction = action.getAction("layout/align_vbottom")
        self.connectionGroup.add(self.alignVBottomAction, receiver = lambda: self.scene.alignSelectionVertically(PyCEGUI.VA_BOTTOM))

        self.focusPropertyInspectorFilterBoxAction = action.getAction("layout/focus_property_inspector_filter_box")
        self.connectionGroup.add(self.focusPropertyInspectorFilterBoxAction, receiver = lambda: self.focusPropertyInspectorFilterBox())

        # normalise actions
        self.connectionGroup.add("layout/normalise_position", receiver = lambda: self.scene.normalisePositionOfSelectedWidgets())
        self.connectionGroup.add("layout/normalise_size", receiver = lambda: self.scene.normaliseSizeOfSelectedWidgets())

        # rounding position and size actions
        self.connectionGroup.add("layout/round_position", receiver = lambda: self.scene.roundPositionOfSelectedWidgets())
        self.connectionGroup.add("layout/round_size", receiver = lambda: self.scene.roundSizeOfSelectedWidgets())

        # moving in parent widget list
        self.connectionGroup.add("layout/move_backward_in_parent_list", receiver = lambda: self.scene.moveSelectedWidgetsInParentWidgetLists(-1))
        self.connectionGroup.add("layout/move_forward_in_parent_list", receiver = lambda: self.scene.moveSelectedWidgetsInParentWidgetLists(1))
*/
}

void LayoutVisualMode::setupToolBar()
{
    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();
    toolBar = mainWindow->createToolbar("Layout");
/*
        self.toolBar.addAction(self.alignHLeftAction)
        self.toolBar.addAction(self.alignHCentreAction)
        self.toolBar.addAction(self.alignHRightAction)
        self.toolBar.addSeparator() # ---------------------------
        self.toolBar.addAction(self.alignVTopAction)
        self.toolBar.addAction(self.alignVCentreAction)
        self.toolBar.addAction(self.alignVBottomAction)
        self.toolBar.addSeparator() # ---------------------------
        self.toolBar.addAction(action.getAction("layout/snap_grid"))
        self.toolBar.addAction(action.getAction("layout/absolute_mode"))
        self.toolBar.addAction(action.getAction("layout/abs_integers_mode"))
        self.toolBar.addAction(action.getAction("layout/normalise_position"))
        self.toolBar.addAction(action.getAction("layout/normalise_size"))
        self.toolBar.addAction(action.getAction("layout/round_position"))
        self.toolBar.addAction(action.getAction("layout/round_size"))
        self.toolBar.addSeparator() # ---------------------------
        self.toolBar.addAction(action.getAction("layout/move_backward_in_parent_list"))
        self.toolBar.addAction(action.getAction("layout/move_forward_in_parent_list"))
*/
}

//!!!???to PropertyInspectorWidget?
void LayoutVisualMode::focusPropertyInspectorFilterBox()
{
/*
        """Focuses into property set inspector filter

        This potentially allows the user to just press a shortcut to find properties to edit,
        instead of having to reach for a mouse.
        """

        filterBox = self.propertiesDockWidget.inspector.filterBox
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
/*
        topMostSelected = []

        for item in self.scene.selectedItems():
            if not isinstance(item, widgethelpers.Manipulator):
                continue

            hasAncestorSelected = False

            for item2 in self.scene.selectedItems():
                if not isinstance(item2, widgethelpers.Manipulator):
                    continue

                if item is item2:
                    continue

                if item2.isAncestorOf(item):
                    hasAncestorSelected = True
                    break

            if not hasAncestorSelected:
                topMostSelected.append(item)

        if len(topMostSelected) == 0:
            return False

        # now we serialise the top most selected widgets (and thus their entire hierarchies)
        topMostSerialisationData = []
        for wdt in topMostSelected:
            serialisationData = widgethelpers.SerialisationData(self, wdt.widget)
            # we set the visual to None because we can't pickle QWidgets (also it would prevent copying across editors)
            # we will set it to the correct visual when we will be pasting it back
            serialisationData.setVisual(None)

            topMostSerialisationData.append(serialisationData)

        data = QtCore.QMimeData()
        data.setData("application/x-ceed-widget-hierarchy-list", QtCore.QByteArray(cPickle.dumps(topMostSerialisationData)))
        QtGui.QApplication.clipboard().setMimeData(data)
*/
    return true;
}

bool LayoutVisualMode::paste()
{
/*
        data = QtGui.QApplication.clipboard().mimeData()

        if not data.hasFormat("application/x-ceed-widget-hierarchy-list"):
            return False

        topMostSerialisationData = cPickle.loads(data.data("application/x-ceed-widget-hierarchy-list").data())

        if len(topMostSerialisationData) == 0:
            return False

        targetManipulator = None
        for item in self.scene.selectedItems():
            if not isinstance(item, widgethelpers.Manipulator):
                continue

            # multiple targets, we can't decide!
            if targetManipulator is not None:
                return False

            targetManipulator = item

        if targetManipulator is None:
            return False

        for serialisationData in topMostSerialisationData:
            serialisationData.setVisual(self)

        cmd = undo.PasteCommand(self, topMostSerialisationData, targetManipulator.widget.getNamePath())
        self.tabbedEditor.undoStack.push(cmd)

        # select the topmost pasted widgets for convenience
        self.scene.clearSelection()
        for serialisationData in topMostSerialisationData:
            manipulator = targetManipulator.getManipulatorByPath(serialisationData.name)
            manipulator.setSelected(True)
*/
    return true;
}

bool LayoutVisualMode::deleteSelected()
{
    return scene->deleteSelectedWidgets();
}

void LayoutVisualMode::zoomIn()
{
    /*
        scene.views()[0].zoomIn()
    */
}

void LayoutVisualMode::zoomOut()
{
    /*
        scene.views()[0].zoomOut()
    */
}

void LayoutVisualMode::zoomReset()
{
    /*
        scene.views()[0].zoomOriginal()
    */
}

void LayoutVisualMode::showEvent(QShowEvent* event)
{
    /*
        mainwindow.MainWindow.instance.ceguiContainerWidget.activate(self, self.scene)
        mainwindow.MainWindow.instance.ceguiContainerWidget.setViewFeatures(wheelZoom = True,
                                                                            middleButtonScroll = True,
                                                                            continuousRendering = settings.getEntry("layout/visual/continuous_rendering").value)

        PyCEGUI.System.getSingleton().getDefaultGUIContext().setRootWindow(self.getCurrentRootWidget())
*/
    hierarchyDockWidget->setEnabled(true);
    propertiesDockWidget->setEnabled(true);
    createWidgetDockWidget->setEnabled(true);
    toolBar->setEnabled(true);

    //???signal from editor to main window instead of storing ptr here?
    if (_editorMenu) _editorMenu->menuAction()->setEnabled(true);

    // Make sure all the manipulators are in sync to matter what
    // this is there mainly for the situation when you switch to live preview, then change resolution, then switch
    // back to visual editing and all manipulators are of different size than they should be
    scene->updateFromWidgets();
/*
        # connect all our actions
        self.connectionGroup.connectAll()

        if self.oldViewState is not None:
            mainwindow.MainWindow.instance.ceguiContainerWidget.setViewState(self.oldViewState)
    */

    QWidget::showEvent(event);
}

void LayoutVisualMode::hideEvent(QHideEvent* event)
{
    // Remember our view transform
/*
    self.oldViewState = mainwindow.MainWindow.instance.ceguiContainerWidget.getViewState()

    # disconnected all our actions
    self.connectionGroup.disconnectAll()
*/

    hierarchyDockWidget->setEnabled(false);
    propertiesDockWidget->setEnabled(false);
    createWidgetDockWidget->setEnabled(false);
    toolBar->setEnabled(false);

    //???signal from editor to main window instead of storing ptr here?
    if (_editorMenu) _editorMenu->menuAction()->setEnabled(false);

/*
    mainwindow.MainWindow.instance.ceguiContainerWidget.deactivate(self)
*/
    QWidget::hideEvent(event);
}

/*
    def getCurrentRootWidget(self):
        return self.scene.rootManipulator.widget if self.scene.rootManipulator is not None else None

    def setRootWidget(self, widget):
        """Sets the root widget we want to edit
        """

        if widget is None:
            self.setRootWidgetManipulator(None)

        else:
            self.setRootWidgetManipulator(widgethelpers.Manipulator(self, None, widget))
*/


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
