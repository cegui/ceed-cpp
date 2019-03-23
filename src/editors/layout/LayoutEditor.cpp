#include "src/editors/layout/LayoutEditor.h"
#include "src/editors/layout/LayoutCodeMode.h"
#include "src/editors/layout/LayoutPreviewerMode.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsEntry.h"

LayoutEditor::LayoutEditor(const QString& filePath)
    : MultiModeEditor(/*layout_compatibility.manager, */ filePath)
{
/*
        self.visual = visual.VisualEditing(self)
        self.addTab(self.visual, "Visual")

*/
    auto codeMode = new LayoutCodeMode(*this);
    tabs.addTab(codeMode, "Code");

    // Layout Previewer is not actually an edit mode, you can't edit the layout from it,
    // however for everything to work smoothly we do push edit mode changes to it to the
    // undo stack.
    //
    // TODO: This could be improved at least a little bit if 2 consecutive edit mode changes
    //       looked like this: A->Preview, Preview->C.  We could simply turn this into A->C,
    //       and if A = C it would eat the undo command entirely.
    auto previewer = new LayoutPreviewerMode(*this);
    tabs.addTab(previewer, "Live Preview");

/*
        # set the toolbar icon size according to the setting and subscribe to it
        self.tbIconSizeEntry = settings.getEntry("global/ui/toolbar_icon_size")
        self.updateToolbarSize(self.tbIconSizeEntry.value)
        self.tbIconSizeCallback = lambda value: self.updateToolbarSize(value)
        self.tbIconSizeEntry.subscribe(self.tbIconSizeCallback)
*/
}

void LayoutEditor::initialize()
{
    MultiModeEditor::initialize();
/*
        # we have to make the context the current context to ensure textures are fine
        self.mainWindow.ceguiContainerWidget.makeGLContextCurrent()

        root = None
        if self.nativeData != "":
            root = PyCEGUI.WindowManager.getSingleton().loadLayoutFromString(self.nativeData)

        self.visual.initialise(root)
*/
}

void LayoutEditor::finalize()
{
/* was in destroy!
        # unsubscribe from the toolbar icon size setting
        self.tbIconSizeEntry.unsubscribe(self.tbIconSizeCallback)
*/
    MultiModeEditor::finalize();
}

void LayoutEditor::activate(QMenu* editorMenu)
{
    MultiModeEditor::activate(editorMenu);

/*
        self.mainWindow.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.visual.hierarchyDockWidget)
        self.visual.hierarchyDockWidget.setVisible(True)
        self.mainWindow.addDockWidget(QtCore.Qt.RightDockWidgetArea, self.visual.propertiesDockWidget)
        self.visual.propertiesDockWidget.setVisible(True)
        self.mainWindow.addDockWidget(QtCore.Qt.LeftDockWidgetArea, self.visual.createWidgetDockWidget)
        self.visual.createWidgetDockWidget.setVisible(True)
        self.mainWindow.addToolBar(QtCore.Qt.ToolBarArea.TopToolBarArea, self.visual.toolBar)
        self.visual.toolBar.show()
*/
}

void LayoutEditor::setupEditorMenu(QMenu* editorMenu)
{
/*
        editorMenu.setTitle("&Layout")
        self.visual.rebuildEditorMenu(editorMenu)

        return True, self.currentWidget() == self.visual
        // visible, enabled
*/
}

void LayoutEditor::deactivate()
{
/*
        self.mainWindow.removeDockWidget(self.visual.hierarchyDockWidget)
        self.mainWindow.removeDockWidget(self.visual.propertiesDockWidget)
        self.mainWindow.removeDockWidget(self.visual.createWidgetDockWidget)
        self.mainWindow.removeToolBar(self.visual.toolBar)
*/
    MultiModeEditor::deactivate();
}

/*
    def updateToolbarSize(self, size):
        if size < 16:
            size = 16
        self.visual.toolBar.setIconSize(QtCore.QSize(size, size))

    def saveAs(self, targetPath, updateCurrentPath = True):
        codeMode = self.currentWidget() is self.code

        # if user saved in code mode, we process the code by propagating it to visual
        # (allowing the change propagation to do the code validating and other work for us)

        if codeMode:
            self.code.propagateToVisual()

        currentRootWidget = self.visual.getCurrentRootWidget()

        if currentRootWidget is None:
            QtGui.QMessageBox.warning(self.mainWindow, "No root widget in the layout!", "I am refusing to save your layout, CEGUI layouts are invalid unless they have a root widget!\n\nPlease create a root widget before saving.")
            return False

        self.nativeData = PyCEGUI.WindowManager.getSingleton().getLayoutAsString(currentRootWidget)
        return super(LayoutTabbedEditor, self).saveAs(targetPath, updateCurrentPath)

    def performCut(self):
        if self.currentWidget() is self.visual:
            return self.visual.performCut()

        return False

    def performCopy(self):
        if self.currentWidget() is self.visual:
            return self.visual.performCopy()

        return False

    def performPaste(self):
        if self.currentWidget() is self.visual:
            return self.visual.performPaste()

        return False

    def performDelete(self):
        if self.currentWidget() is self.visual:
            return self.visual.performDelete()

        return False

    def zoomIn(self):
        if self.currentWidget() is self.visual:
            self.visual.scene.views()[0].zoomIn()

    def zoomOut(self):
        if self.currentWidget() is self.visual:
            self.visual.scene.views()[0].zoomOut()

    def zoomReset(self):
        if self.currentWidget() is self.visual:
            self.visual.scene.views()[0].zoomOriginal()
*/

void LayoutEditor::createActions(ActionManager& mgr)
{

    /*
    def declare(actionManager):
        cat = actionManager.createCategory(name = "layout", label = "Layout Editor")

        cat.createAction(name = "align_hleft", label = "Align &Left (horizontally)",
                         help_ = "Sets horizontal alignment of all selected widgets to left.",
                         icon = QtGui.QIcon("icons/layout_editing/align_hleft.png"))
        cat.createAction(name = "align_hcentre", label = "Align Centre (&horizontally)",
                         help_ = "Sets horizontal alignment of all selected widgets to centre.",
                         icon = QtGui.QIcon("icons/layout_editing/align_hcentre.png"))
        cat.createAction(name = "align_hright", label = "Align &Right (horizontally)",
                         help_ = "Sets horizontal alignment of all selected widgets to right.",
                         icon = QtGui.QIcon("icons/layout_editing/align_hright.png"))

        cat.createAction(name = "align_vtop", label = "Align &Top (vertically)",
                         help_ = "Sets vertical alignment of all selected widgets to top.",
                         icon = QtGui.QIcon("icons/layout_editing/align_vtop.png"))
        cat.createAction(name = "align_vcentre", label = "Align Centre (&vertically)",
                         help_ = "Sets vertical alignment of all selected widgets to centre.",
                         icon = QtGui.QIcon("icons/layout_editing/align_vcentre.png"))
        cat.createAction(name = "align_vbottom", label = "Align &Bottom (vertically)",
                         help_ = "Sets vertical alignment of all selected widgets to bottom.",
                         icon = QtGui.QIcon("icons/layout_editing/align_vbottom.png"))

        cat.createAction(name = "snap_grid", label = "Snap to &Grid",
                         help_ = "When resizing and moving widgets, if checked this makes sure they snap to a snap grid (see settings for snap grid related entries), also shows the snap grid if checked.",
                         icon = QtGui.QIcon("icons/layout_editing/snap_grid.png"),
                         defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_Space)).setCheckable(True)

        absolute_mode = cat.createAction(
                         name = "absolute_mode", label = "&Absolute Resizing && Moving Deltas",
                         help_ = "When resizing and moving widgets, if checked this makes the delta absolute, it is relative if unchecked.",
                         icon = QtGui.QIcon("icons/layout_editing/absolute_mode.png"),
                         defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_A))
        absolute_mode.setCheckable(True)
        absolute_mode.setChecked(True)

        abs_integers_mode = cat.createAction(
                         name = "abs_integers_mode", label = "Only Increase/Decrease by Integers When Moving or Resizing",
                         help_ = "If checked, while resizing or moving widgets in the editor only integer values (e.g. no"
                                 "0.25 or 0.5 etc.) will be added to the current absolute values. This is only relevant if editing"
                                 "in zoomed-in view while 'Absolute Resizing and Moving' is activated.",
                         icon = QtGui.QIcon("icons/layout_editing/abs_integers_mode.png"),
                         defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_Q))
        abs_integers_mode.setCheckable(True)
        abs_integers_mode.setChecked(True)

        cat.createAction(name = "normalise_position", label = "Normalise &Position (cycle)",
                         help_ = "If the position is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                         icon = QtGui.QIcon("icons/layout_editing/normalise_position.png"),
                         defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_D))

        cat.createAction(name = "normalise_size", label = "Normalise &Size (cycle)",
                         help_ = "If the size is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                         icon = QtGui.QIcon("icons/layout_editing/normalise_size.png"),
                         defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_S))

        cat.createAction(name = "round_position", label = "Rounds the absolute position to nearest integer",
                         help_ = "The value of the absolute position will be rounded to the nearest integer value (e.g.: 1.7 will become 2.0 and -4.2 will become -4.0",
                         icon = QtGui.QIcon("icons/layout_editing/round_position.png"),
                         defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_M))
        cat.createAction(name = "round_size", label = "Rounds the absolute size to nearest integer",
                         help_ = "The value of the absolute size will be rounded to the nearest integer value (e.g.: 1.7 will become 2.0 and -4.2 will become -4.0",
                         icon = QtGui.QIcon("icons/layout_editing/round_size.png"),
                         defaultShortcut = QtGui.QKeySequence(QtCore.Qt.Key_N))

        cat.createAction(name = "move_backward_in_parent_list", label = "Moves widget -1 step in the parent's widget list",
                         help_ = "Moves selected widget(s) one step backward in their parent's widget list (Only applicable to SequentialLayoutContainer widgets, VerticalLayoutContainer and HorizontalLayoutContainer in particular.)",
                         icon = QtGui.QIcon("icons/layout_editing/move_backward_in_parent_list.png"))
        cat.createAction(name = "move_forward_in_parent_list", label = "Moves widget +1 step in the parent's widget list",
                         help_ = "Moves selected widget(s) one step forward in their parent's widget list (Only applicable to SequentialLayoutContainer widgets, VerticalLayoutContainer and HorizontalLayoutContainer in particular.)",
                         icon = QtGui.QIcon("icons/layout_editing/move_forward_in_parent_list.png"))

        cat.createAction(name = "focus_property_inspector_filter_box", label = "&Focus Property Inspector Filter Box",
                         help_ = "This allows you to easily press a shortcut and immediately search through properties without having to reach for a mouse.",
                         icon = QtGui.QIcon("icons/layout_editing/focus_property_inspector_filter_box.png"),
                         defaultShortcut = QtGui.QKeySequence(QtGui.QKeySequence.Find))

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

void LayoutEditor::createSettings(Settings& mgr)
{
    auto catImageset = mgr.createCategory("layout", "Layout editing");
    auto secVisual = catImageset->createSection("visual", "Visual editing");

    // FIXME: Only applies to newly switched to visual modes!
    SettingsEntryPtr entry(new SettingsEntry(*secVisual, "continuous_rendering", false, "Continuous rendering",
                                             "Check this if you are experiencing redraw issues (your skin contains an idle animation or such)."
                                             "\nOnly applies to newly switched to visual modes so switch to Code mode or back or restart the application for this to take effect.",
                                             "checkbox", false, -1));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "prevent_manipulator_overlap", false, "Prevent manipulator overlap",
                                  "Only enable if you have a very fast computer and only edit small layouts. Very performance intensive!",
                                  "checkbox", false, 0));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "normal_outline", QPen(QColor(255, 255, 0, 255)), "Normal outline",
                                  "Pen for normal outline.",
                                  "pen", false, 1));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "hover_outline", QPen(QColor(0, 255, 255, 255)), "Hover outline",
                                  "Pen for hover outline.",
                                  "pen", false, 2));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "resizing_outline", QPen(QColor(255, 0, 255, 255)), "Outline while resizing",
                                  "Pen for resizing outline.",
                                  "pen", false, 3));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "moving_outline", QPen(QColor(255, 0, 255, 255)), "Outline while moving",
                                  "Pen for moving outline.",
                                  "pen", false, 4));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "snap_grid_x", 5.f, "Snap grid cell width (X)",
                                  "Snap grid X metric.",
                                  "float", false, 5));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "snap_grid_y", 5.f, "Snap grid cell width (Y)",
                                  "Snap grid Y metric.",
                                  "float", false, 6));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "snap_grid_point_colour", QColor(255, 255, 255, 192), "Snap grid point colour",
                                  "Color of snap grid points.",
                                  "colour", false, 7));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "snap_grid_point_shadow_colour", QColor(64, 64, 64, 192), "Snap grid point shadow colour",
                                  "Color of snap grid points (shadows).",
                                  "colour", false, 8));
    secVisual->addEntry(std::move(entry));

    // TODO: Full restart is not actually needed, just a refresh on all layout visual editing modes
    entry.reset(new SettingsEntry(*secVisual, "hide_deadend_autowidgets", true, "Hide deadend auto widgets",
                                  "Should auto widgets with no non-auto widgets descendants be hidden in the widget hierarchy?",
                                  "checkbox", true, 9));
    secVisual->addEntry(std::move(entry));

    // FIXME: Only applies to newly refreshed visual modes!
    entry.reset(new SettingsEntry(*secVisual, "auto_widgets_selectable", false, "Make auto widgets selectable",
                                  "Auto widgets are usually handled by LookNFeel and except in very special circumstances, you don't want to "
                                  "deal with them at all. Only for EXPERT use! Will make CEED crash in cases where you don't know what you are doing!",
                                  "checkbox", false, 10));
    secVisual->addEntry(std::move(entry));

    // FIXME: Only applies to newly refreshed visual modes!
    entry.reset(new SettingsEntry(*secVisual, "auto_widgets_show_outline", false, "Show outline of auto widgets",
                                  "Auto widgets are usually handled by LookNFeel and except in very special circumstances, you don't want to "
                                  "deal with them at all. Only use if you know what you are doing! This might clutter the interface a lot.",
                                  "checkbox", false, 11));
    secVisual->addEntry(std::move(entry));
}
//---------------------------------------------------------------------

QString LayoutEditorFactory::getFileTypesDescription() const
{
    return "Layout files";
}

QStringList LayoutEditorFactory::getFileExtensions() const
{
    /*
        extensions = layout_compatibility.manager.getAllPossibleExtensions()
    */
    return { "layout" };
}

EditorBasePtr LayoutEditorFactory::create(const QString& filePath) const
{
    return std::make_unique<LayoutEditor>(filePath);
}
