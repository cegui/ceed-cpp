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

    try
    {
        // FIXME: open manually and load from string?
        auto layoutFileName = QDir(CEGUIManager::Instance().getCurrentProject()->getResourceFilePath("", "layouts")).relativeFilePath(_filePath);
        CEGUI::Window* root = CEGUI::WindowManager::getSingleton().loadLayoutFromFile(CEGUIUtils::qStringToString(layoutFileName));
        visualMode->initialize(root);
    }
    catch (const std::exception& e)
    {
        QMessageBox::warning(nullptr, "Exception", e.what());
    }
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
    mainWindow.addToolBar(Qt::ToolBarArea::TopToolBarArea, visualMode->getToolBar());
    visualMode->getToolBar()->show();

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
    mainWindow.removeToolBar(visualMode->getToolBar());

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

void LayoutEditor::getRawData(QByteArray& outRawData)
{
    // If user saved in code mode, we process the code by propagating it to visual
    // (allowing the change propagation to do the code validating and other work for us)
    if (tabs.currentWidget() == codeMode)
        codeMode->propagateToVisual();

    auto currentRootWidget = visualMode->getRootWidget();
    if (!currentRootWidget)
    {
        QMessageBox::warning(nullptr, "No root widget in the layout!",
                             "I am refusing to save your layout, CEGUI layouts are invalid unless they have a root widget!\n\n"
                             "Please create a root widget before saving.");
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
