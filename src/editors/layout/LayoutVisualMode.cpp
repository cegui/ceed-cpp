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
#include "src/util/Utils.h"
#include "src/Application.h"
#include <CEGUI/GUIContext.h>
#include <CEGUI/WindowManager.h>
#include "qboxlayout.h"
#include "qgraphicsview.h"
#include "qtoolbar.h"
#include "qmenu.h"
#include "qmimedata.h"
#include "qclipboard.h"
#include <qdatetime.h>
#include <qstandardpaths.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qdesktopservices.h>
#include <qurl.h>
#include <qclipboard.h>
#include <qbuffer.h>
#include <unordered_set>
#include <qinputdialog.h>

void LayoutVisualMode::removeNestedManipulators(std::set<LayoutManipulator*>& manipulators)
{
    for (auto it = manipulators.begin(); it != manipulators.end(); /**/)
    {
        bool erased = false;
        const LayoutManipulator* manipulator = *it;
        for (LayoutManipulator* potentialParent : manipulators)
            if (potentialParent->isAncestorOf(manipulator))
            {
                it = manipulators.erase(it);
                erased = true;
                break;
            }

        if (!erased) ++it;
    }
}

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
    actionScreenshot = app->getAction("layout/screenshot");
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
    contextMenu->addAction(actionScreenshot);
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

void LayoutVisualMode::activate(MainWindow& mainWindow, bool editorActivated)
{
    IEditMode::activate(mainWindow, editorActivated);

    hierarchyDockWidget->setEnabled(true);

    scene->updatePropertySet();
    mainWindow.getPropertyDockWidget()->setEnabled(true);

    createWidgetDockWidget->setEnabled(true);
    mainWindow.getToolbar("Layout")->setEnabled(true);

    mainWindow.setEditorMenuEnabled(true);

    // Make sure all the manipulators are in sync to matter what
    // this is there mainly for the situation when you switch to live preview, then change resolution, then switch
    // back to visual editing and all manipulators are of different size than they should be
    scene->updateFromWidgets();

    createActiveStateConnections();
}

bool LayoutVisualMode::deactivate(MainWindow& mainWindow, bool editorDeactivated)
{
    disconnectActiveStateConnections();

    hierarchyDockWidget->setEnabled(false);
    mainWindow.getPropertyDockWidget()->setEnabled(false);
    createWidgetDockWidget->setEnabled(false);
    mainWindow.getToolbar("Layout")->setEnabled(false);

    mainWindow.setEditorMenuEnabled(false);

    return IEditMode::deactivate(mainWindow, editorDeactivated);
}

void LayoutVisualMode::setRootWidgetManipulator(LayoutManipulator* manipulator)
{
    // Remember selection
    std::set<QString> selectedPaths;
    std::set<LayoutManipulator*> selectedWidgets;
    scene->collectSelectedWidgets(selectedWidgets);
    for (LayoutManipulator* manipulator : selectedWidgets)
        selectedPaths.insert(manipulator->getWidgetPath());

    // Set new root
    auto oldRoot = getRootWidget();
    scene->setRootWidgetManipulator(manipulator);
    hierarchyDockWidget->setRootWidgetManipulator(manipulator);
    if (oldRoot) CEGUI::WindowManager::getSingleton().destroyWindow(oldRoot);

    // Restore selection
    if (!selectedPaths.empty())
    {
        scene->batchSelection(true);
        for (const QString& path : selectedPaths)
            if (auto manipulator = scene->getManipulatorByPath(path))
                manipulator->setSelected(true);
        scene->batchSelection(false);
        emit scene->selectionChanged();
    }
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
    editorMenu->addAction(actionScreenshot);
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
}

void LayoutVisualMode::createActiveStateConnections()
{
    _activeStateConnections.push_back(connect(hierarchyDockWidget, &WidgetHierarchyDockWidget::deleteRequested, scene, &LayoutScene::deleteSelectedWidgets));
    _activeStateConnections.push_back(connect(actionScreenshot, &QAction::triggered, this, &LayoutVisualMode::takeScreenshot));
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
    std::set<LayoutManipulator*> selectedWidgets;
    scene->collectSelectedWidgets(selectedWidgets);

    // We serialize only topmost selected widgets (and thus their entire hierarchies)
    removeNestedManipulators(selectedWidgets);

    if (selectedWidgets.empty()) return false;

    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    for (LayoutManipulator* manipulator : selectedWidgets)
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

bool LayoutVisualMode::moveWidgetsInHierarchy(QStringList&& paths, LayoutManipulator* newParentManipulator, size_t newChildIndex)
{
    if (!newParentManipulator || paths.empty()) return false;

    CEGUIUtils::removeNestedPaths(paths);

    std::vector<LayoutMoveInHierarchyCommand::Record> records;
    std::unordered_set<QString> usedNames;
    size_t addedChildCount = 0;

    for (const QString& widgetPath : paths)
    {
        auto manipulator = scene->getManipulatorByPath(widgetPath);
        if (!manipulator || manipulator == newParentManipulator) continue;

        auto oldParentManipulator = dynamic_cast<LayoutManipulator*>(manipulator->parentItem());
        const size_t oldChildIndex = manipulator->getWidgetIndexInParent();
        const QString oldWidgetName = manipulator->getWidgetName();
        QString suggestedName = oldWidgetName;

        if (newParentManipulator == oldParentManipulator)
        {
            // Already at the destination
            if (newChildIndex == oldChildIndex) continue;
        }
        else
        {
            ++addedChildCount;

            // Prevent name clashes at the new parent
            // When a name clash occurs, we suggest a new name to the user and
            // ask them to confirm it/enter their own.
            // The tricky part is that we have to consider the other widget renames
            // too (in case we're reparenting and renaming more than one widget)
            // and we must prevent invalid names (i.e. containing "/")
            QString error;
            while (true)
            {
                // Get a name that's not used in the new parent, trying to keep
                // the suggested name (which is the same as the old widget name at
                // the beginning)
                QString tempName = CEGUIUtils::getUniqueChildWidgetName(*newParentManipulator->getWidget(), suggestedName);

                // If the name we got is the same as the one we wanted...
                if (tempName == suggestedName)
                {
                    // ...we need to check our own usedNames list too, in case
                    // another widget we're reparenting has got this name.
                    int counter = 2;
                    while (usedNames.find(suggestedName) != usedNames.end())
                    {
                        // When this happens, we simply add a numeric suffix to
                        // the suggested name. The result could theoretically
                        // collide in the new parent but it's OK because this
                        // is just a suggestion and will be checked again when
                        // the 'while' loops.
                        suggestedName = tempName + QString::number(counter);
                        ++counter;
                        error = QString("Widget name is in use by another widget being processed");
                    }

                    // If we had no collision, we can keep this name!
                    if (counter == 2) break;
                }
                else
                {
                    // The new parent had a child with that name already and so
                    // it gave us a new suggested name.
                    suggestedName = tempName;
                    error = "Widget name already exists in the new parent";
                }

                // Ask the user to confirm our suggested name or enter a new one.
                // We do this in a loop because we validate the user input.
                while (true)
                {
                    bool ok = false;
                    suggestedName = QInputDialog::getText(this, error,
                                                          "New name for '" + oldWidgetName + "':",
                                                          QLineEdit::Normal, suggestedName, &ok);

                    // Abort everything if the user cancels the dialog
                    if (!ok) return false;

                    // Validate the entered name
                    suggestedName = CEGUIUtils::getValidWidgetName(suggestedName);
                    if (!suggestedName.isEmpty()) break;
                    error = "Invalid name, please try again";
                }
            }
        }

        usedNames.insert(suggestedName);

        LayoutMoveInHierarchyCommand::Record rec;
        rec.oldParentPath = oldParentManipulator->getWidgetPath();
        rec.oldChildIndex = oldChildIndex;
        rec.newChildIndex = newChildIndex;
        rec.oldName = oldWidgetName;
        rec.newName = suggestedName;
        records.push_back(std::move(rec));
    }

    if (records.empty()) return false;

    // FIXME: better to calculate addedChildCount, then do this check, then suggest renaming
    if (!newParentManipulator->canAcceptChildren(addedChildCount, true)) return false;

    getEditor().getUndoStack()->push(new LayoutMoveInHierarchyCommand(*this, std::move(records), newParentManipulator->getWidgetPath()));
    return true;
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

void LayoutVisualMode::takeScreenshot()
{
    if (!scene) return;

    QImage screenshot = scene->getCEGUIScreenshot();
    if (screenshot.isNull()) return;

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();

    // Save to file

    if (settings->getEntryValue("cegui/screenshots/save", true).toBool())
    {
        // TODO: add project subfolder (need name), optional through settings
        const QDir dir(QDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).filePath("CEED"));
        const QString fileName = QString("%1-%2x%3-%4.png")
                .arg(QFileInfo(getEditor().getFilePath()).baseName())
                .arg(static_cast<int>(scene->getContextWidth()))
                .arg(static_cast<int>(scene->getContextHeight()))
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
        const QString filePath = dir.filePath(fileName);

        dir.mkpath(".");
        if (screenshot.save(filePath, "PNG", 50))
        {
            const auto action = settings->getEntryValue("cegui/screenshots/after_save_action").toInt();
            switch (action)
            {
                // TODO: https://stackoverflow.com/questions/3490336/how-to-reveal-in-finder-or-show-in-explorer-with-qt
                case 0: QDesktopServices::openUrl(QUrl::fromLocalFile(dir.path())); break;
                case 1: QDesktopServices::openUrl(QUrl::fromLocalFile(filePath)); break;
                default: break;
            }
        }
    }

    // Copy to clipboard
    // NB: here we alter screenshot data, so saving to file must happen before this

    // Oh... -_-
    // I'm not surprised there are wars and all that sh*t in the world.
    // How could it be otherwise if people at 2019 A.D. can't come to an agreement
    // about pasting images to applications. Is it really so hard or rarely used?
    // Here is a table of what I discovered on Windows. Not even tested other systems.
    // Ability to paste opaque / transparent image:
    //       Windows 8.1      | MS Word 2016 | MS Paint | GIMP 2.10.12 | Slack 4.0.0
    // --------------------------------------------------------------------------------
    // Qt setImageData        |   Opaque     |  Opaque  |     ---      |  Opaque
    // PNG                    | Transparent  |   ---    | Transparent  |   ---
    // Qt setImageData + PNG  |     ---      |  Opaque  | Transparent  |  Opaque
    // --------------------------------------------------------------------------------
    // Please, do something with it, if you can. In the meantime I leave here a couple of
    // settings that can be used to paste to different software.

    QMimeData* data = new QMimeData();

    const auto checkerWidth = settings->getEntryValue("cegui/background/checker_width").toInt();
    const auto checkerHeight = settings->getEntryValue("cegui/background/checker_height").toInt();
    const auto checkerFirstColour = settings->getEntryValue("cegui/background/first_colour").value<QColor>();
    const auto checkerSecondColour = settings->getEntryValue("cegui/background/second_colour").value<QColor>();
    const bool needChecker = settings->getEntryValue("cegui/screenshots/bg_checker", true).toBool();

    // Save PNG. It keeps transparency if checker background is not explicitly requested.

    if (needChecker)
        Utils::fillTransparencyWithChecker(screenshot, checkerWidth, checkerHeight, checkerFirstColour, checkerSecondColour);

    {
        QByteArray imageData;
        QBuffer buffer(&imageData);
        if (buffer.open(QIODevice::WriteOnly))
        {
            screenshot.save(&buffer, "PNG", 100);
            buffer.close();
        }

        data->setData("PNG", imageData);
    }

    // Save with Qt. On Windows it expands into a whole bunch of formats inside a clipboard,
    // and we can't access them here. Qt doesn't handle transparency, all transparent pixels
    // become black. We fill the background with a checker instead.

    if (settings->getEntryValue("cegui/screenshots/use_qt_setimage", true).toBool())
    {
        if (!needChecker)
            Utils::fillTransparencyWithChecker(screenshot, checkerWidth, checkerHeight, checkerFirstColour, checkerSecondColour);

        data->setImageData(screenshot);
    }

    QApplication::clipboard()->setMimeData(data);
}
