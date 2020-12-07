#include "src/editors/layout/LayoutEditor.h"
#include "src/editors/layout/LayoutCodeMode.h"
#include "src/editors/layout/LayoutVisualMode.h"
#include "src/editors/layout/LayoutPreviewerMode.h"
#include "src/cegui/CEGUIManager.h"
#include "src/cegui/CEGUIProject.h"
#include "src/cegui/CEGUIUtils.h"
#include "src/util/Settings.h"
#include "src/util/SettingsCategory.h"
#include "src/util/SettingsSection.h"
#include "src/util/SettingsEntry.h"
#include "src/ui/MainWindow.h"
#include "src/ui/layout/WidgetHierarchyDockWidget.h"
#include "src/ui/layout/CreateWidgetDockWidget.h"
#include "src/ui/layout/LayoutManipulator.h"
#include "src/Application.h"
#include "qmenu.h"
#include "qtoolbar.h"
#include "qmessagebox.h"
#include "qfileinfo.h"
#include "QDir"
#include <CEGUI/WindowManager.h>

LayoutEditor::LayoutEditor(const QString& filePath)
    : MultiModeEditor(/*layout_compatibility.manager, */ filePath)
{
    visualMode = new LayoutVisualMode(*this);
    tabs.addTab(visualMode, "Visual");

    codeMode = new LayoutCodeMode(*this);
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
}

void LayoutEditor::initialize()
{
    MultiModeEditor::initialize();

    visualMode->setRootWidgetManipulator(nullptr);

    try
    {
        if (!_filePath.isEmpty())
        {
            QByteArray rawData;
            {
                QFile file(_filePath);
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    QMessageBox::warning(nullptr, "File read error", "Layout editor can't read file " + _filePath);
                    return;
                }

                rawData = file.readAll();
            }

            if (rawData.size() > 0)
            {
                CEGUI::Window* widget = CEGUI::WindowManager::getSingleton().loadLayoutFromString(CEGUIUtils::qStringToString(rawData));
                auto root = new LayoutManipulator(*visualMode, nullptr, widget);
                root->updateFromWidget();
                root->createChildManipulators(true, false, false);
                visualMode->setRootWidgetManipulator(root);
            }
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(nullptr, "Exception", e.what());
    }

    visualMode->getCreateWidgetDockWidget()->populate();
}

void LayoutEditor::activate(MainWindow& mainWindow)
{
    MultiModeEditor::activate(mainWindow);

    mainWindow.addDockWidget(Qt::LeftDockWidgetArea, visualMode->getHierarchyDockWidget());
    visualMode->getHierarchyDockWidget()->setVisible(true);
    mainWindow.addDockWidget(Qt::LeftDockWidgetArea, visualMode->getCreateWidgetDockWidget());
    visualMode->getCreateWidgetDockWidget()->setVisible(true);
    mainWindow.addDockWidget(Qt::RightDockWidgetArea, mainWindow.getPropertyDockWidget());
    mainWindow.getPropertyDockWidget()->setVisible(true);

    auto toolBar = mainWindow.getToolbar("Layout");
    mainWindow.addToolBar(Qt::ToolBarArea::TopToolBarArea, toolBar);
    toolBar->show();

    auto editorMenu = mainWindow.getEditorMenu();
    editorMenu->setTitle("&Layout");
    visualMode->rebuildEditorMenu(editorMenu);
    editorMenu->menuAction()->setVisible(true);
    editorMenu->menuAction()->setEnabled(tabs.currentWidget() == visualMode);
}

void LayoutEditor::deactivate(MainWindow& mainWindow)
{
    mainWindow.removeDockWidget(visualMode->getHierarchyDockWidget());
    mainWindow.removeDockWidget(visualMode->getCreateWidgetDockWidget());
    mainWindow.removeDockWidget(mainWindow.getPropertyDockWidget());
    mainWindow.removeToolBar(mainWindow.getToolbar("Layout"));

    MultiModeEditor::deactivate(mainWindow);
}

void LayoutEditor::copy()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->copy();
}

void LayoutEditor::cut()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->cut();
}

void LayoutEditor::paste()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->paste();
}

void LayoutEditor::deleteSelected()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->deleteSelected();
}

void LayoutEditor::zoomIn()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->zoomIn();
}

void LayoutEditor::zoomOut()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->zoomOut();
}

void LayoutEditor::zoomReset()
{
    if (tabs.currentWidget() == visualMode)
        visualMode->zoomReset();
}

QString LayoutEditor::getFileTypesDescription() const
{
    return LayoutEditorFactory::layoutFileTypesDescription();
}

QStringList LayoutEditor::getFileExtensions() const
{
    return LayoutEditorFactory::layoutFileExtensions();
}

void LayoutEditor::getRawData(QByteArray& outRawData)
{
    // If user saved in code mode, we process the code by propagating it to visual
    // (allowing the change propagation to do the code validation and other work for us)
    if (tabs.currentWidget() == codeMode)
        codeMode->propagateToVisual();

    auto currentRootWidget = visualMode->getRootWidget();
    if (!currentRootWidget)
    {
        QMessageBox::warning(nullptr, "No root widget in the layout!",
                             "Please create a root widget in order to use this layout in CEGUI.");
        return;
    }

    CEGUI::String layoutString = CEGUI::WindowManager::getSingleton().getLayoutAsString(*currentRootWidget);
    outRawData = CEGUIUtils::stringToQString(layoutString).toUtf8();
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

    entry.reset(new SettingsEntry(*secVisual, "close_anchor_presets", false, "Close anchor presets after select",
                                  "Choose whether to close an anchor presets menu when any option is selected or keep it opened",
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

    entry.reset(new SettingsEntry(*secVisual, "snap_grid_x", 5, "Snap grid cell width (X)",
                                  "Snap grid X metric.",
                                  "int", false, 5));
    secVisual->addEntry(std::move(entry));

    entry.reset(new SettingsEntry(*secVisual, "snap_grid_y", 5, "Snap grid cell width (Y)",
                                  "Snap grid Y metric.",
                                  "int", false, 6));
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

void LayoutEditor::createActions(Application& app)
{
    app.getOrCreateShortcutSettingsSection("layout", "Layout Editor");

    app.registerAction("layout", "show_anchors", "Show anchor handles",
                       "Show or hide anchor handles for the selected widget.",
                       QIcon(":/icons/anchors/Anchors.png"), QKeySequence(), true);

    app.registerAction("layout", "show_lc_handles", "Show layout container handles",
                       "Show or hide handles of layout containers.",
                       QIcon(":/icons/widgets/GLCt.png"), QKeySequence(), true);

    app.registerAction("layout", "absolute_mode", "&Absolute Resizing && Moving Deltas",
                       "When resizing and moving widgets, if checked this makes the delta absolute, it is relative if unchecked.",
                       QIcon(":/icons/layout_editing/absolute_mode.png"), QKeySequence(Qt::Key_A), true);

    app.registerAction("layout", "abs_integers_mode", "Only Increase/Decrease by Integers When Moving or Resizing",
                       "If checked, while resizing or moving widgets in the editor only integer values (e.g. no"
                       "0.25 or 0.5 etc.) will be added to the current absolute values. This is only relevant if editing"
                       "in zoomed-in view while 'Absolute Resizing and Moving' is activated.",
                       QIcon(":/icons/layout_editing/abs_integers_mode.png"), QKeySequence(Qt::Key_Q), true);

    app.registerAction("layout", "snap_grid", "Snap to &Grid",
                       "When resizing and moving widgets, if checked this makes sure they snap to a snap grid (see "
                       "settings for snap grid related entries), also shows the snap grid if checked.",
                       QIcon(":/icons/layout_editing/snap_grid.png"), QKeySequence(Qt::Key_Space), true);

    app.registerAction("layout", "screenshot", "Scene Screenshot",
                       "Takes a screenshot of the whole scene in 100% zoom and without HUD items.",
                       QIcon(":/icons/actions/screenshot.png"), QKeySequence(Qt::Key_Pause));

    app.registerAction("layout", "select_parent", "Select &Parent",
                       "Select an immediate parent of the widget.",
                       QIcon(":/icons/layout_editing/SelectParent.png"), QKeySequence(Qt::Key_Backspace));

    app.registerAction("layout", "align_hleft", "Align &Left (horizontally)",
                       "Sets horizontal alignment of all selected widgets to left.",
                       QIcon(":/icons/layout_editing/align_hleft.png"));

    app.registerAction("layout", "align_hcentre", "Align Centre (&horizontally)",
                       "Sets horizontal alignment of all selected widgets to centre.",
                       QIcon(":/icons/layout_editing/align_hcentre.png"));

    app.registerAction("layout", "align_hright", "Align &Right (horizontally)",
                       "Sets horizontal alignment of all selected widgets to right.",
                       QIcon(":/icons/layout_editing/align_hright.png"));

    app.registerAction("layout", "align_vtop", "Align &Top (vertically)",
                       "Sets vertical alignment of all selected widgets to top.",
                       QIcon(":/icons/layout_editing/align_vtop.png"));

    app.registerAction("layout", "align_vcentre", "Align Centre (&vertically)",
                       "Sets vertical alignment of all selected widgets to centre.",
                       QIcon(":/icons/layout_editing/align_vcentre.png"));

    app.registerAction("layout", "align_vbottom", "Align &Bottom (vertically)",
                       "Sets vertical alignment of all selected widgets to bottom.",
                       QIcon(":/icons/layout_editing/align_vbottom.png"));

    app.registerAction("layout", "normalise_position", "Normalize &Position (cycle)",
                       "If the position is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                       QIcon(":/icons/layout_editing/normalise_position.png"), QKeySequence(Qt::Key_D));

    app.registerAction("layout", "normalise_size", "Normalize &Size (cycle)",
                       "If the size is mixed (absolute and relative) it becomes relative only, if it's relative it becomes absolute, if it's absolute it becomes relative.",
                       QIcon(":/icons/layout_editing/normalise_size.png"), QKeySequence(Qt::Key_S));

    app.registerAction("layout", "round_position", "Round the absolute position to nearest integer",
                       "The value of the absolute position will be rounded to the nearest integer value (e.g.: 1.7 will become 2.0 and -4.2 will become -4.0",
                       QIcon(":/icons/layout_editing/round_position.png"), QKeySequence(Qt::Key_M));

    app.registerAction("layout", "round_size", "Round the absolute size to nearest integer",
                       "The value of the absolute size will be rounded to the nearest integer value (e.g.: 1.7 will become 2.0 and -4.2 will become -4.0",
                       QIcon(":/icons/layout_editing/round_size.png"), QKeySequence(Qt::Key_N));

    app.registerAction("layout", "move_backward_in_parent_list", "Move widget -1 step in the parent's widget list",
                       "Moves selected widget(s) one step backward in their parent's widget list",
                       QIcon(":/icons/layout_editing/move_backward_in_parent_list.png"));

    app.registerAction("layout", "move_forward_in_parent_list", "Move widget +1 step in the parent's widget list",
                       "Moves selected widget(s) one step forward in their parent's widget list",
                       QIcon(":/icons/layout_editing/move_forward_in_parent_list.png"));

    app.registerAction("layout", "copy_widget_path", "C&opy Widget Paths",
                       "Copies the 'NamePath' properties of the selected widgets to the clipboard.",
                       QIcon(":/icons/actions/copy.png"));

    app.registerAction("layout", "rename", "&Rename Widget",
                       "Edits the selected widget's name.",
                       QIcon(":/icons/layout_editing/rename.png"));

    app.registerAction("layout", "lock_widget", "&Lock Widget",
                       "Locks the widget for moving and resizing in the visual editing mode.",
                       QIcon(":/icons/layout_editing/lock_widget.png"));

    app.registerAction("layout", "unlock_widget", "&Unlock Widget",
                       "Unlocks the widget for moving and resizing in the visual editing mode.",
                       QIcon(":/icons/layout_editing/unlock_widget.png"));

    app.registerAction("layout", "recursively_lock_widget", "&Lock Widget (recursively)",
                       "Locks the widget and all its child widgets for moving and resizing in the visual editing mode.",
                       QIcon(":/icons/layout_editing/lock_widget_recursively.png"));

    app.registerAction("layout", "recursively_unlock_widget", "&Unlock Widget (recursively)",
                       "Unlocks the widget and all its child widgets for moving and resizing in the visual editing mode.",
                       QIcon(":/icons/layout_editing/unlock_widget_recursively.png"));

    /*
        cat.createAction(name = "focus_property_inspector_filter_box", label = "&Focus Property Inspector Filter Box",
                         help_ = "This allows you to easily press a shortcut and immediately search through properties without having to reach for a mouse.",
                         icon = QtGui.QIcon("icons/layout_editing/focus_property_inspector_filter_box.png"),
                         defaultShortcut = QtGui.QKeySequence(QtGui.QKeySequence.Find))
    */
}

void LayoutEditor::createToolbar(Application& app)
{
    auto toolBar = app.getMainWindow()->createToolbar("Layout");
    toolBar->addAction(app.getAction("layout/align_hleft"));
    toolBar->addAction(app.getAction("layout/align_hcentre"));
    toolBar->addAction(app.getAction("layout/align_hright"));
    toolBar->addSeparator();
    toolBar->addAction(app.getAction("layout/align_vtop"));
    toolBar->addAction(app.getAction("layout/align_vcentre"));
    toolBar->addAction(app.getAction("layout/align_vbottom"));
    toolBar->addSeparator();
    toolBar->addAction(app.getAction("layout/snap_grid"));
    toolBar->addAction(app.getAction("layout/absolute_mode"));
    toolBar->addAction(app.getAction("layout/abs_integers_mode"));
    toolBar->addSeparator();
    toolBar->addAction(app.getAction("layout/normalise_position"));
    toolBar->addAction(app.getAction("layout/normalise_size"));
    toolBar->addAction(app.getAction("layout/round_position"));
    toolBar->addAction(app.getAction("layout/round_size"));
    toolBar->addSeparator();
    toolBar->addAction(app.getAction("layout/move_backward_in_parent_list"));
    toolBar->addAction(app.getAction("layout/move_forward_in_parent_list"));
    toolBar->setVisible(false);
}

//---------------------------------------------------------------------

QString LayoutEditorFactory::layoutFileTypesDescription()
{
    return "Layout";
}

QStringList LayoutEditorFactory::layoutFileExtensions()
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
