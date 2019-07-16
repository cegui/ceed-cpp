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

    Application* app = qobject_cast<Application*>(qApp);

    actionShowAnchors = app->getAction("layout/show_anchors");
    actionShowLCHandles = app->getAction("layout/show_lc_handles");
    actionAbsoluteMode = app->getAction("layout/absolute_mode");
    actionAbsoluteIntegerMode = app->getAction("layout/abs_integers_mode");
    actionSnapGrid = app->getAction("layout/snap_grid");
    actionSelectParent = app->getAction("layout/select_parent");
    actionAlignHLeft = app->getAction("layout/align_hleft");
    actionAlignHCenter = app->getAction("layout/align_hcentre");
    actionAlignHRight = app->getAction("layout/align_hright");
    actionAlignVTop = app->getAction("layout/align_vtop");
    actionAlignVCenter = app->getAction("layout/align_vcentre");
    actionAlignVBottom = app->getAction("layout/align_vbottom");
    actionNormalizePosition = app->getAction("layout/normalise_position");
    actionNormalizeSize = app->getAction("layout/normalise_size");
    actionRoundPosition = app->getAction("layout/round_position");
    actionRoundSize = app->getAction("layout/round_size");
    actionMoveBackward = app->getAction("layout/move_backward_in_parent_list");
    actionMoveForward = app->getAction("layout/move_forward_in_parent_list");

    /*
        self.focusPropertyInspectorFilterBoxAction = action.getAction("layout/focus_property_inspector_filter_box")
    */

    // For a context menu only
    actionAnchorPresets = new QAction(QIcon(":/icons/anchors/SelfBottom.png"), "Anchor Presets", contextMenu);

    // Initial state of checkable actions
    actionShowAnchors->setChecked(true);
    actionShowLCHandles->setChecked(true);
    actionAbsoluteMode->setChecked(true);
    actionAbsoluteIntegerMode->setChecked(true);
    actionSnapGrid->setChecked(false);

    // These actions are global (cross-scene), we connect to them once and keep the connection active
    _anyStateConnections.push_back(connect(actionShowAnchors, &QAction::toggled, [this](bool /*toggled*/) { scene->updateAnchorItems(); }));
    _anyStateConnections.push_back(connect(actionShowLCHandles, &QAction::toggled, scene, &LayoutScene::showLayoutContainerHandles));

    auto mainWindow = app->getMainWindow();

    contextMenu = new QMenu(this);
    contextMenu->addAction(actionSelectParent);
    contextMenu->addSeparator();
    contextMenu->addAction(mainWindow->getActionCut());
    contextMenu->addAction(mainWindow->getActionCopy());
    contextMenu->addAction(mainWindow->getActionPaste());
    contextMenu->addSeparator();
    contextMenu->addAction(actionAnchorPresets);
    contextMenu->addAction(actionShowAnchors);
    contextMenu->addAction(actionShowLCHandles);

    hierarchyDockWidget->setupContextMenu();

    auto onSnapGridSettingsChanged = [this]() { snapGridBrushValid = false; };
    connect(settings->getEntry("layout/visual/snap_grid_x"), &SettingsEntry::valueChanged, onSnapGridSettingsChanged);
    connect(settings->getEntry("layout/visual/snap_grid_y"), &SettingsEntry::valueChanged, onSnapGridSettingsChanged);
    connect(settings->getEntry("layout/visual/snap_grid_point_colour"), &SettingsEntry::valueChanged, onSnapGridSettingsChanged);
    connect(settings->getEntry("layout/visual/snap_grid_point_shadow_colour"), &SettingsEntry::valueChanged, onSnapGridSettingsChanged);
}

LayoutVisualMode::~LayoutVisualMode()
{
    auto oldRoot = getRootWidget();
    if (oldRoot)
        CEGUI::WindowManager::getSingleton().destroyWindow(oldRoot);

    delete hierarchyDockWidget;
    delete createWidgetDockWidget;
}

void LayoutVisualMode::setRootWidgetManipulator(LayoutManipulator* manipulator)
{
    auto oldRoot = getRootWidget();

    scene->setRootWidgetManipulator(manipulator);
    hierarchyDockWidget->setRootWidgetManipulator(manipulator);

    if (oldRoot)
        CEGUI::WindowManager::getSingleton().destroyWindow(oldRoot);
}

CEGUI::Window* LayoutVisualMode::getRootWidget() const
{
    auto manip = scene->getRootWidgetManipulator();
    return manip ? manip->getWidget() : nullptr;
}

void LayoutVisualMode::rebuildEditorMenu(QMenu* editorMenu)
{
    // Similar to the toolbar, includes the focus filter box action
    editorMenu->addAction(actionSelectParent);
    editorMenu->addSeparator();
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

void LayoutVisualMode::createActiveStateConnections()
{
    _activeStateConnections.push_back(connect(hierarchyDockWidget, &WidgetHierarchyDockWidget::deleteRequested, scene, &LayoutScene::deleteSelectedWidgets));
    _activeStateConnections.push_back(connect(actionSelectParent, &QAction::triggered, scene, &LayoutScene::selectParent));
    _activeStateConnections.push_back(connect(actionAlignHLeft, &QAction::triggered, [this]() { scene->alignSelectionHorizontally(CEGUI::HorizontalAlignment::Left); }));
    _activeStateConnections.push_back(connect(actionAlignHCenter, &QAction::triggered, [this]() { scene->alignSelectionHorizontally(CEGUI::HorizontalAlignment::Centre); }));
    _activeStateConnections.push_back(connect(actionAlignHRight, &QAction::triggered, [this]() { scene->alignSelectionHorizontally(CEGUI::HorizontalAlignment::Right); }));
    _activeStateConnections.push_back(connect(actionAlignVTop, &QAction::triggered, [this]() { scene->alignSelectionVertically(CEGUI::VerticalAlignment::Top); }));
    _activeStateConnections.push_back(connect(actionAlignVCenter, &QAction::triggered, [this]() { scene->alignSelectionVertically(CEGUI::VerticalAlignment::Centre); }));
    _activeStateConnections.push_back(connect(actionAlignVBottom, &QAction::triggered, [this]() { scene->alignSelectionVertically(CEGUI::VerticalAlignment::Bottom); }));
    _activeStateConnections.push_back(connect(actionNormalizePosition, &QAction::triggered, scene, &LayoutScene::normalizePositionOfSelectedWidgets));
    _activeStateConnections.push_back(connect(actionNormalizeSize, &QAction::triggered, scene, &LayoutScene::normalizeSizeOfSelectedWidgets));
    _activeStateConnections.push_back(connect(actionRoundPosition, &QAction::triggered, scene, &LayoutScene::roundPositionOfSelectedWidgets));
    _activeStateConnections.push_back(connect(actionRoundSize, &QAction::triggered, scene, &LayoutScene::roundSizeOfSelectedWidgets));
    _activeStateConnections.push_back(connect(actionMoveBackward, &QAction::triggered, [this]() { scene->moveSelectedWidgetsInParentWidgetLists(-1); }));
    _activeStateConnections.push_back(connect(actionMoveForward, &QAction::triggered, [this]() { scene->moveSelectedWidgetsInParentWidgetLists(+1); }));
    /*
        self.connectionGroup.add(self.focusPropertyInspectorFilterBoxAction, receiver = lambda: self.focusPropertyInspectorFilterBox())
    */
    _activeStateConnections.push_back(connect(actionAnchorPresets, &QAction::triggered, [this]() { scene->showAnchorPopupMenu(QCursor::pos()); }));
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

    {
        std::set<LayoutManipulator*> selectedWidgets;
        scene->collectSelectedWidgets(selectedWidgets);

        for (LayoutManipulator* manipulator : selectedWidgets)
        {
            bool hasAncestorSelected = false;
            for (LayoutManipulator* manipulator2 : selectedWidgets)
            {
                if (manipulator == manipulator2) continue;

                if (manipulator2->isAncestorOf(manipulator))
                {
                    hasAncestorSelected = true;
                    break;
                }
            }

            if (!hasAncestorSelected) topMostSelected.push_back(manipulator);
        }
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

    std::set<LayoutManipulator*> selectedWidgets;
    scene->collectSelectedWidgets(selectedWidgets);

    // Multiple targets, we can't decide!
    if (selectedWidgets.size() > 1) return false;

    LayoutManipulator* target = selectedWidgets.empty() ? nullptr : *selectedWidgets.begin();
    if (target && !target->canAcceptChildren(1, true)) return false;

    _editor.getUndoStack()->push(new LayoutPasteCommand(*this, target ? target->getWidgetPath() : QString(), std::move(bytes)));

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
    mainWindow->getToolbar("Layout")->setEnabled(true);

    //???signal from editor to main window instead of storing ptr here?
    if (_editorMenu) _editorMenu->menuAction()->setEnabled(true);

    // Make sure all the manipulators are in sync to matter what
    // this is there mainly for the situation when you switch to live preview, then change resolution, then switch
    // back to visual editing and all manipulators are of different size than they should be
    scene->updateFromWidgets();

    createActiveStateConnections();

    QWidget::showEvent(event);
}

void LayoutVisualMode::hideEvent(QHideEvent* event)
{
    disconnectActiveStateConnections();

    auto mainWindow = qobject_cast<Application*>(qApp)->getMainWindow();

    hierarchyDockWidget->setEnabled(false);
    mainWindow->getPropertyDockWidget()->setEnabled(false);
    createWidgetDockWidget->setEnabled(false);
    mainWindow->getToolbar("Layout")->setEnabled(false);

    //???signal from editor to main window instead of storing ptr here?
    if (_editorMenu) _editorMenu->menuAction()->setEnabled(false);

    QWidget::hideEvent(event);
}
