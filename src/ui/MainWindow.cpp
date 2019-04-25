#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "qtoolbar.h"
#include "qtoolbutton.h"
#include "qpushbutton.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "qdesktopservices.h"
#include "qtabbar.h"
#include "qsettings.h"
#include "qevent.h"
#include "qundostack.h"
#include "src/Application.h"
#include "src/util/Settings.h"
#include "src/util/SettingsEntry.h"
#include "src/util/RecentlyUsed.h"
#include "src/cegui/CEGUIProjectManager.h"
#include "src/cegui/CEGUIProject.h"
#include "src/editors/NoEditor.h"
#include "src/editors/TextEditor.h"
#include "src/editors/BitmapEditor.h"
#include "src/editors/layout/LayoutEditor.h"
#include "src/editors/imageset/ImagesetEditor.h"
#include "src/ui/dialogs/AboutDialog.h"
#include "src/ui/dialogs/LicenseDialog.h"
#include "src/ui/dialogs/NewProjectDialog.h"
#include "src/ui/dialogs/ProjectSettingsDialog.h"
#include "src/ui/dialogs/MultiplePossibleFactoriesDialog.h"
#include "src/ui/dialogs/SettingsDialog.h"
#include "src/ui/ProjectManager.h"
#include "src/ui/FileSystemBrowser.h"
#include "src/ui/UndoViewer.h"
#include "PropertyWidget/PropertyWidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // We have to construct ActionManager before settings interface (as it alters the settings declaration)!
    //???is true for C++ version?
    /*
        self.actionManager = action.ActionManager(self, self.app.settings)
    */

    settingsDialog = new SettingsDialog(this);

    // Register factories

    editorFactories.push_back(std::make_unique<TextEditorFactory>());
    editorFactories.push_back(std::make_unique<BitmapEditorFactory>());
    editorFactories.push_back(std::make_unique<LayoutEditorFactory>());
    editorFactories.push_back(std::make_unique<ImagesetEditorFactory>());
    /*
        animation_list_editor.AnimationListTabbedEditorFactory(),           // Animation files
        looknfeel_editor.LookNFeelTabbedEditorFactory(),                    //
    */

    // Register file types from factories as filters

    QStringList allExt;
    for (const auto& factory : editorFactories)
    {
        QStringList ext = factory->getFileExtensions();
        QString filter = factory->getFileTypesDescription() + " (%1)";
        filter = filter.arg("*." + ext.join(" *."));
        editorFactoryFileFilters.append(filter);

        allExt.append(ext);
    }

    editorFactoryFileFilters.insert(0, "All known files (*." + allExt.join(" *.") + ")");
    editorFactoryFileFilters.insert(1, "All files (*)");

    // Setup UI

    ui->setupUi(this);

    ui->tabs->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tabs->tabBar(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_tabBarCustomContextMenuRequested(QPoint)));

    projectManager = new ProjectManager(this);
    //projectManager->setVisible(false);
    connect(projectManager, &ProjectManager::itemOpenRequested, this, &MainWindow::openEditorTab);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, projectManager);

    fsBrowser = new FileSystemBrowser(this);
    //fsBrowser->setVisible(false);
    connect(fsBrowser, &FileSystemBrowser::fileOpenRequested, this, &MainWindow::openEditorTab);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, fsBrowser);

    auto propertyWidget = new QtnPropertyWidget();
    propertyWidget->setParts(QtnPropertyWidgetPartsDescriptionPanel);
    propertyDockWidget = new QDockWidget("Properties", this);
    propertyDockWidget->setObjectName("Property dock widget");
    propertyDockWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum); // Make the dock take as much space as it can vertically
    propertyDockWidget->setWidget(propertyWidget);
    propertyDockWidget->setVisible(false);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, propertyDockWidget);

    undoViewer = new UndoViewer(this);
    undoViewer->setVisible(false);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, undoViewer);

    ui->actionStatusbar->setChecked(statusBar()->isVisible());

    setupToolbars();

    // Setup dynamic menus

    recentlyUsedProjects = new RecentlyUsedMenuEntry("Projects", this);
    recentlyUsedProjects->setParentMenu(ui->menuRecentProjects);
    connect(recentlyUsedProjects, &RecentlyUsed::triggered, this, &MainWindow::openRecentProject);
    recentlyUsedFiles = new RecentlyUsedMenuEntry("Files", this);
    recentlyUsedFiles->setParentMenu(ui->menuRecentFiles);
    connect(recentlyUsedFiles, &RecentlyUsed::triggered, this, &MainWindow::openRecentFile);

    connect(ui->menu_View, &QMenu::aboutToShow, [this]()
    {
        // Create default main window's popup with a list of all docks & toolbars
        docsToolbarsMenu = QMainWindow::createPopupMenu();
        docsToolbarsMenu->setTitle("&Docks && Toolbars");
        ui->menu_View->insertMenu(ui->actionStatusbar, docsToolbarsMenu);
    });
    connect(ui->menu_View, &QMenu::aboutToHide, [this]()
    {
        ui->menu_View->removeAction(docsToolbarsMenu->menuAction());
        docsToolbarsMenu = nullptr;
    });

    // Menu for the current editor
    ui->menuEditor->menuAction()->setVisible(false);

    connect(ui->menuTabs, &QMenu::aboutToShow, [this]()
    {
        tabsMenuSeparator = ui->menuTabs->addSeparator();

        // The items are taken from the 'self.tabEditors' list which always has the order
        // by which the files were opened (not the order of the tabs in the tab bar).
        // This is a feature, maintains the same mnemonic even if a tab is moved.
        int counter = 1;
        for (auto&& editor : activeEditors)
        {
            QString name = editor->getFilePath();

            // Trim if too long, for the first 10 tabs add mnemonic (1 to 9, then 0).
            // TODO: the next few lines are basically the same as the code in recentlyused.
            // Refactor so both places use the same (generic) code.
            if (name.length() > 40)
                name = "..." + name.right(37);
            if (counter <= 10)
                name = QString("&%1. %2").arg(counter % 10).arg(name);
            ++counter;

            QAction* action = new QAction(name, ui->menuTabs);
            action->setData(editor->getFilePath());
            ui->menuTabs->addAction(action);

            connect(action, &QAction::triggered, [this, action]()
            {
                activateEditorTabByFilePath(action->data().toString());
            });
        }
    });
    connect(ui->menuTabs, &QMenu::aboutToHide, [this]()
    {
        int index = ui->menuTabs->actions().indexOf(tabsMenuSeparator);
        assert(index >= 0);
        while (ui->menuTabs->actions().size() > index)
            ui->menuTabs->removeAction(ui->menuTabs->actions().back());
        tabsMenuSeparator = nullptr;
    });

    // Restore geometry and state of this window from QSettings

    auto&& settings = qobject_cast<Application*>(qApp)->getSettings()->getQSettings();
    if (settings->contains("window-geometry"))
        restoreGeometry(settings->value("window-geometry").toByteArray());
    if (settings->contains("window-state"))
        restoreState(settings->value("window-state").toByteArray());
}

MainWindow::~MainWindow()
{
    delete ui;
}

QMenu *MainWindow::getEditorMenu() const
{
    return ui->menuEditor;
}

void MainWindow::setupToolbars()
{
    setupToolbar(ui->toolBarStandard);
    setupToolbar(ui->toolBarEdit);
    setupToolbar(ui->toolBarView);
    setupToolbar(ui->toolBarProject);

    // FIXME: here until I manage to create menu toolbutton in Qt Creator
    QToolBar* toolbar = ui->toolBarStandard;
    QToolButton* newMenuBtn = new QToolButton(this);
    newMenuBtn->setText("New");
    newMenuBtn->setToolTip("New file");
    newMenuBtn->setIcon(QIcon(":/icons/actions/new_file.png"));
    newMenuBtn->setPopupMode(QToolButton::InstantPopup);
    newMenuBtn->setMenu(ui->menu_New);

    QAction* before = toolbar->actions().empty() ? nullptr : toolbar->actions().front();
    toolbar->insertWidget(before, newMenuBtn);

    // The menubutton does not resize its icon correctly unless we tell it to do so
    connect(toolbar, &QToolBar::iconSizeChanged, newMenuBtn, &QToolButton::setIconSize);
}

void MainWindow::setupToolbar(QToolBar* toolBar)
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    auto tbIconSizeEntry = settings->getEntry("global/ui/toolbar_icon_size");
    assert(tbIconSizeEntry);

    const int iconSize = std::max(16, tbIconSizeEntry->value().toInt());
    toolBar->setIconSize(QSize(iconSize, iconSize));

    connect(tbIconSizeEntry, &SettingsEntry::valueChanged, [toolBar](const QVariant& newValue)
    {
        const int iconSize = std::max(16, newValue.toInt());
        toolBar->setIconSize(QSize(iconSize, iconSize));
    });
}

QToolBar* MainWindow::createToolbar(const QString& name)
{
    QToolBar* toolBar = addToolBar(name);
    toolBar->setObjectName(name + " toolbar");
    setupToolbar(toolBar);
    return toolBar;
}

QAction* MainWindow::getActionDeleteSelected() const
{
    return ui->actionDelete;
}

QAction* MainWindow::getActionZoomIn() const
{
    return ui->actionZoomIn;
}

QAction* MainWindow::getActionZoomOut() const
{
    return ui->actionZoomOut;
}

QAction* MainWindow::getActionZoomReset() const
{
    return ui->actionZoomReset;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!on_actionQuit_triggered())
        event->ignore();
    else
        event->accept();
}

// Safely quits the editor, prompting user to save changes to files and the project.
bool MainWindow::on_actionQuit_triggered()
{
    // We remember last tab we closed to check whether user pressed Cancel in any of the dialogs
    QWidget* lastTab = nullptr;
    while (!activeEditors.empty())
    {
        QWidget* currTab = ui->tabs->widget(0);
        if (currTab == lastTab)
        {
            // User pressed cancel on one of the tab editor 'save without changes' dialog,
            // cancel the whole quit operation!
            return false;
        }

        lastTab = currTab;

        on_tabs_tabCloseRequested(0);
    }

    // Save geometry and state of this window to QSettings
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings()->getQSettings();
    settings->setValue("window-geometry", saveGeometry());
    settings->setValue("window-state", saveState());

    // Close project after all tabs have been closed, there may be tabs requiring a project opened!
    if (CEGUIProjectManager::Instance().isProjectLoaded())
    {
        // In case user pressed cancel the entire quitting processed has to be terminated
        if (!on_actionCloseProject_triggered())
            return false;
    }

    QApplication::quit();

    return true;
}

void MainWindow::updateProjectDependentUI(CEGUIProject* newProject)
{
    const bool isProjectLoaded = !!newProject;

    projectManager->setProject(newProject);

    if (isProjectLoaded)
    {
        recentlyUsedProjects->addRecentlyUsed(newProject->filePath);

        // TODO: Maybe this could be configurable?
        auto baseDir = newProject->getAbsolutePathOf("");
        if (QFileInfo(baseDir).isDir())
            fsBrowser->setDirectory(baseDir);
    }
    else
    {
        // Since we are effectively unloading the project and potentially nuking resources of it
        // we should definitely unload all tabs that rely on it to prevent segfaults and other
        // nasty phenomena
        assert(closeAllTabsRequiringProject());

        fsBrowser->setDirectory(QDir::homePath());
    }

    fsBrowser->projectDirectoryButton()->setEnabled(isProjectLoaded);

    ui->actionSaveProject->setEnabled(isProjectLoaded);
    ui->actionCloseProject->setEnabled(isProjectLoaded);
    ui->actionProjectSettings->setEnabled(isProjectLoaded);
    ui->actionReloadResources->setEnabled(isProjectLoaded);
}

void MainWindow::on_actionNewProject_triggered()
{
    if (CEGUIProjectManager::Instance().isProjectLoaded())
    {
        // Another project is already opened!
        auto result = QMessageBox::question(this,
                              "Another project already opened!",
                              "Before creating a new project, you must close the one currently opened. "
                              "Do you want to close currently opened project? (all unsaved changes will be lost!)",
                              QMessageBox::Yes | QMessageBox::Cancel,
                              QMessageBox::Cancel);

        if (result != QMessageBox::Yes) return;

        // Don't close the project yet, close it after the user
        // accepts the New Project dialog below because they may cancel
    }

    NewProjectDialog newProjectDialog;
    if (newProjectDialog.exec() != QDialog::Accepted) return;

    // The dialog was accepted, close any open project
    updateProjectDependentUI(nullptr);
    CEGUIProjectManager::Instance().unloadProject();

    // Create a new project
    auto newProject = CEGUIProjectManager::Instance().createProject(newProjectDialog.getFilePath(), newProjectDialog.isCreateResourceDirsSelected());

    // Open project settings dialog
    on_actionProjectSettings_triggered();

    // Save the project with the settings that were potentially set in the project settings dialog
    newProject->save();
}

void MainWindow::on_actionOpenProject_triggered()
{
    if (CEGUIProjectManager::Instance().isProjectLoaded())
    {
        // Another project is already opened!
        auto result = QMessageBox::question(this,
                              "Another project already opened!",
                              "Before opening a project, you must close the one currently opened. "
                              "Do you want to close currently opened project? (all unsaved changes will be lost!)",
                              QMessageBox::Yes | QMessageBox::Cancel,
                              QMessageBox::Cancel);

        if (result != QMessageBox::Yes) return;

        updateProjectDependentUI(nullptr);
        CEGUIProjectManager::Instance().unloadProject();
    }

    auto fileName = QFileDialog::getOpenFileName(this,
                                                "Open existing project file",
                                                "",
                                                "Project files (*.project)");

    if (!fileName.isEmpty())
    {
        CEGUIProjectManager::Instance().loadProject(fileName);
        updateProjectDependentUI(CEGUIProjectManager::Instance().getCurrentProject());
    }
}

void MainWindow::on_actionProjectSettings_triggered()
{
    if (!CEGUIProjectManager::Instance().isProjectLoaded()) return;

    // Since we are effectively unloading the project and potentially nuking resources of it
    // we should definitely unload all tabs that rely on it to prevent segfaults and other
    // nasty phenomena
    if (!closeAllTabsRequiringProject())
    {
        QMessageBox::information(this,
                                 "Project dependent tabs still open!",
                                 "You can't alter project's settings while having tabs that "
                                 "depend on the project and its resources opened!");
        return;
    }

    ProjectSettingsDialog dialog(*CEGUIProjectManager::Instance().getCurrentProject(), this);
    if (dialog.exec() == QDialog::Accepted)
    {
        dialog.apply(*CEGUIProjectManager::Instance().getCurrentProject());
        CEGUIProjectManager::Instance().syncProjectToCEGUIInstance();
    }
}

void MainWindow::on_actionFullScreen_triggered()
{
    if (isFullScreen())
    {
        if (wasMaximizedBeforeFullscreen)
            showMaximized();
        else
            showNormal();
    }
    else
    {
        wasMaximizedBeforeFullscreen = isMaximized();
        showFullScreen();
    }
}

void MainWindow::on_actionStatusbar_toggled(bool isChecked)
{
    statusBar()->setVisible(isChecked);
}

void MainWindow::on_actionQuickstartGuide_triggered()
{
    QDir docDir(qobject_cast<Application*>(qApp)->getDocumentationPath());
    QDesktopServices::openUrl(QUrl("file://" + docDir.absoluteFilePath("quickstart-guide.pdf"), QUrl::TolerantMode));
}

void MainWindow::on_actionUserManual_triggered()
{
    QDir docDir(qobject_cast<Application*>(qApp)->getDocumentationPath());
    QDesktopServices::openUrl(QUrl("file://" + docDir.absoluteFilePath("user-manual.pdf"), QUrl::TolerantMode));
}

void MainWindow::on_actionWikiPage_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.cegui.org.uk/wiki/index.php/CEED", QUrl::TolerantMode));
}

void MainWindow::on_actionSendFeedback_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.cegui.org.uk/phpBB2/viewforum.php?f=15", QUrl::TolerantMode));
}

void MainWindow::on_actionReportBug_triggered()
{
    QDesktopServices::openUrl(QUrl("http://www.cegui.org.uk/mantis/bug_report_page.php", QUrl::TolerantMode));
}

void MainWindow::on_actionCEGUIDebugInfo_triggered()
{
    /*
        self.ceguiContainerWidget.debugInfo.show()
    */
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dlg;
    dlg.exec();
}

void MainWindow::on_actionLicense_triggered()
{
    LicenseDialog dlg;
    dlg.exec();
}

void MainWindow::on_actionQt_triggered()
{
    QApplication::aboutQt();
}

void MainWindow::on_actionPreferences_triggered()
{
    settingsDialog->show();
}

void MainWindow::slot_tabBarCustomContextMenuRequested(const QPoint& pos)
{
    auto tabIdx = ui->tabs->tabBar()->tabAt(pos);
    ui->tabs->setCurrentIndex(tabIdx);

    QMenu* menu = new QMenu(this);
    menu->addAction(ui->actionCloseTab);
    menu->addSeparator();
    menu->addAction(ui->actionCloseOtherTabs);
    menu->addAction(ui->actionCloseAllTabs);

    if (tabIdx >= 0)
    {
        /*
            auto tabWidget = tabs->widget(tabIdx);
        */
        menu->addSeparator();
        QAction* dataTypeAction = new QAction("Data type: " /*+ (tabWidget.getDesiredSavingDataType())*/, this);
        dataTypeAction->setToolTip("Displays which data type this file will be saved to (the desired saving data type).");
        menu->addAction(dataTypeAction);
    }

    menu->exec(ui->tabs->tabBar()->mapToGlobal(pos));
}

void MainWindow::on_tabs_currentChanged(int index)
{
    auto newEditor = getEditorForTab(index);
    if (currentEditor == newEditor) return;

    // To fight flicker
    ui->tabs->setUpdatesEnabled(false);

    if (currentEditor)
    {
        disconnect(currentEditor, &EditorBase::undoAvailable, this, &MainWindow::onUndoAvailable);
        disconnect(currentEditor, &EditorBase::redoAvailable, this, &MainWindow::onRedoAvailable);
        disconnect(currentEditor, &EditorBase::labelChanged, this, &MainWindow::onEditorLabelChanged);
        disconnect(currentEditor, &EditorBase::contentsChanged, this, &MainWindow::onEditorContentsChanged);
        disconnect(currentEditor, &EditorBase::fileChangedExternally, this, &MainWindow::onEditorFileChangedExternally);

        currentEditor->deactivate(*this);

        ui->menuEditor->clear();
        ui->menuEditor->menuAction()->setVisible(false);
        ui->menuEditor->menuAction()->setEnabled(false);

        // Also reset texts in case the tabbed editor messed with them
        ui->actionUndo->setEnabled(false);
        ui->actionRedo->setEnabled(false);
        ui->actionUndo->setText("Undo");
        ui->actionRedo->setText("Redo");

        undoViewer->setUndoStack(nullptr);
    }

    statusBar()->clearMessage();

    currentEditor = newEditor;

    const bool hasEditor = (currentEditor != nullptr);
    fsBrowser->activeFileDirectoryButton()->setEnabled(hasEditor);
    ui->actionRevert->setEnabled(hasEditor);
    ui->actionSave->setEnabled(hasEditor);
    ui->actionSaveAs->setEnabled(hasEditor);
    ui->actionCloseTab->setEnabled(hasEditor);
    ui->actionCloseOtherTabs->setEnabled(hasEditor);

    if (currentEditor)
    {
        // If the file was changed by an external program, ask the user to reload the changes
        if (currentEditor->isModifiedExternally())
            onEditorFileChangedExternally();

        currentEditor->activate(*this);

        auto undoStack = currentEditor->getUndoStack();
        if (undoStack)
        {
            undoViewer->setUndoStack(undoStack);

            ui->actionSave->setEnabled(undoStack->isClean());
            ui->actionUndo->setEnabled(undoStack->canUndo());
            ui->actionRedo->setEnabled(undoStack->canRedo());
            ui->actionUndo->setText("Undo " + undoStack->undoText());
            ui->actionRedo->setText("Redo " + undoStack->redoText());
        }

        connect(currentEditor, &EditorBase::undoAvailable, this, &MainWindow::onUndoAvailable);
        connect(currentEditor, &EditorBase::redoAvailable, this, &MainWindow::onRedoAvailable);
        connect(currentEditor, &EditorBase::labelChanged, this, &MainWindow::onEditorLabelChanged);
        connect(currentEditor, &EditorBase::contentsChanged, this, &MainWindow::onEditorContentsChanged);
        connect(currentEditor, &EditorBase::fileChangedExternally, this, &MainWindow::onEditorFileChangedExternally);
    }

    ui->tabs->setUpdatesEnabled(true);
}

bool MainWindow::on_tabs_tabCloseRequested(int index)
{
    EditorBase* editor = getEditorForTab(index);

    // If it is not an editor tab, close it
    if (!editor) return true;

    if (!editor->hasChanges())
    {
        // We can close immediately
        closeEditorTab(editor);
        return true;
    }

    // We have changes, lets ask the user whether we should dump them or save them
    auto result = QMessageBox::question(this,
                                        "Unsaved changes!",
                                        tr("There are unsaved changes in '%1'. "
                                        "Do you want to save them? "
                                        "(Pressing Discard will discard the changes!)").arg(editor->getFilePath()),
                                        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                        QMessageBox::Save);

    if (result == QMessageBox::Save)
    {
        // Let's save changes and then kill the editor (This is the default action)
        // If there was an error saving the file, stop what we're doing
        // and let the user fix the problem.
        if (!editor->save())
            return false;

        closeEditorTab(editor);
        return true;
    }
    else if (result == QMessageBox::Discard)
    {
        // Changes will be discarded
        // NB: we don't have to call editor.discardChanges here
        closeEditorTab(editor);
        return true;
    }

    // Don't do anything if user selected 'Cancel'
    return false;
}

void MainWindow::on_actionCloseTab_triggered()
{
    on_tabs_tabCloseRequested(ui->tabs->currentIndex());
}

void MainWindow::on_actionCloseOtherTabs_triggered()
{
    auto currTab = ui->tabs->currentWidget();
    int i = 0;
    while (i < ui->tabs->count())
    {
        // Don't close current & cancelled by user due to unsaved changes
        if (ui->tabs->widget(i) == currTab)
            ++i;
        else if (!on_tabs_tabCloseRequested(i))
            ++i;
    }
}

void MainWindow::on_actionCloseAllTabs_triggered()
{
    int i = 0;
    while (i < ui->tabs->count())
    {
        // Don't close ones cancelled by user due to unsaved changes
        if (!on_tabs_tabCloseRequested(i))
            ++i;
    }
}

void MainWindow::on_actionPreviousTab_triggered()
{
    if (ui->tabs->count() < 2) return;
    int index = ui->tabs->currentIndex() - 1;
    if (index < 0)
        index += ui->tabs->count();
    ui->tabs->setCurrentIndex(index);
}

void MainWindow::on_actionNextTab_triggered()
{
    if (ui->tabs->count() < 2) return;
    int index = ui->tabs->currentIndex() + 1;
    if (index >= ui->tabs->count())
        index = 0;
    ui->tabs->setCurrentIndex(index);
}

// Opens editor tab. Creates new editor if such file wasn't opened yet and if it was opened,
// it just makes the tab current.
void MainWindow::openEditorTab(const QString& absolutePath)
{
    if (activateEditorTabByFilePath(absolutePath)) return;

    EditorBase* editor = createEditorForFile(absolutePath);
    if (!editor) return;

    ui->tabs->setCurrentWidget(editor->getWidget());
}

// Closes given editor tab.
void MainWindow::closeEditorTab(EditorBase* editor)
{
    auto it = std::find_if(activeEditors.begin(), activeEditors.end(), [editor](const EditorBasePtr& element)
    {
        return element.get() == editor;
    });
    if (it == activeEditors.end()) return;

    const int tabIndex = ui->tabs->indexOf(editor->getWidget());

    editor->finalize();
    editor->destroy();

    if (tabIndex >= 0)
    {
        // TODO: if this never asserts it can be safely removed
        assert(ui->tabs->widget(tabIndex) == editor->getWidget());
        ui->tabs->removeTab(tabIndex);
    }

    activeEditors.erase(it);
}

// Attempts to close all tabs that require a project opened.
// This is usually done when project settings are altered and CEGUI instance has to be reloaded
// or when project is being closed and we can no longer rely on resource availability.
bool MainWindow::closeAllTabsRequiringProject()
{
    int i = 0;
    while (i < ui->tabs->count())
    {
        auto editor = getEditorForTab(i);
        if (editor->requiresProject())
        {
            // If the method returns False user pressed Cancel so in that case
            // we cancel the entire operation
            if (!on_tabs_tabCloseRequested(i)) return false;

            continue;
        }

        ++i;
    }

    return true;
}

EditorBase* MainWindow::getEditorForTab(int index) const
{
    return getEditorForTab(ui->tabs->widget(index));
}

EditorBase* MainWindow::getEditorForTab(QWidget* tabWidget) const
{
    if (!tabWidget) return nullptr;

    auto it = std::find_if(activeEditors.begin(), activeEditors.end(), [tabWidget](const EditorBasePtr& element)
    {
        return element->getWidget() == tabWidget;
    });
    return (it == activeEditors.end()) ? nullptr : it->get();
}

// Activates (makes current) the tab for the path specified
bool MainWindow::activateEditorTabByFilePath(const QString& absolutePath)
{
    QString path = QDir::cleanPath(absolutePath);
    for (auto&& editor : activeEditors)
    {
        if (editor->getFilePath() == absolutePath)
        {
            ui->tabs->setCurrentWidget(editor->getWidget());
            return true;
        }
    }
    return false;
}

// Creates a new editor for file at given absolutePath. This always creates a new editor,
// it is not advised to use this method directly, use openEditorTab instead.
EditorBase* MainWindow::createEditorForFile(const QString& absolutePath)
{
    EditorBasePtr ret = nullptr;

    QString projectRelativePath;
    if (CEGUIProjectManager::Instance().isProjectLoaded())
        projectRelativePath = QDir(CEGUIProjectManager::Instance().getCurrentProject()->getAbsolutePathOf("")).relativeFilePath(absolutePath);
    else
        projectRelativePath = "<No project opened>";

    if (!QFileInfo(absolutePath).exists())
    {
        ret.reset(new NoEditor(absolutePath,
               tr("Couldn't find '%1' (project relative path: '%2'), please check that that your project's "
               "base directory is set up correctly and that you hadn't deleted "
               "the file from your HDD. Consider removing the file from the project.").arg(absolutePath).arg(projectRelativePath)));
    }
    else
    {
        std::vector<EditorFactoryBase*> possibleFactories;
        for (auto& factory : editorFactories)
        {
            if (factory->canEditFile(absolutePath))
                possibleFactories.push_back(factory.get());
        }

        // At this point if possibleFactories is [], no registered tabbed editor factory wanted
        // to accept the file, so we create MessageTabbedEditor that will simply
        // tell the user that given file can't be edited
        // IMO this is a reasonable compromise and plays well with the rest of
        // the editor without introducing exceptions, etc...
        if (possibleFactories.empty())
        {
            if (absolutePath.endsWith(".project"))
            {
                // Provide a more newbie-friendly message in case they are
                // trying to open a project file as if it were a file
                ret.reset(new NoEditor(absolutePath,
                    tr("You are trying to open '%1' (project relative path: '%2') which "
                    "seems to be a CEED project file. "
                    "This simply is not how things are supposed to work, please use "
                    "File -> Open Project to open your project file instead. "
                    "(CEED enforces proper extensions)").arg(absolutePath).arg(projectRelativePath)));
            }
            else
            {
                ret.reset(new NoEditor(absolutePath,
                    tr("No included tabbed editor was able to accept '%1' "
                    "(project relative path: '%2'), please check that it's a file CEED "
                    "supports and that it has the correct extension "
                    "(CEED enforces proper extensions)").arg(absolutePath).arg(projectRelativePath)));
            }
        }
        else
        {
            // One or more factories wants to accept the file
            EditorFactoryBase* factory = nullptr;
            if (possibleFactories.size() == 1)
            {
                // It's decided, just one factory wants to accept the file
                factory = possibleFactories[0];
            }
            else
            {
                // More than 1 factory wants to accept the file, offer a dialog and let user choose
                MultiplePossibleFactoriesDialog dialog(possibleFactories, this);
                if (dialog.exec() == QDialog::Accepted)
                    factory = dialog.getSelectedFactory();
            }

            if (factory)
                ret = factory->create(absolutePath);
            else
                ret.reset(new NoEditor(absolutePath,
                                       tr("You failed to choose an editor to open '%s' with (project relative path: '%s')."
                                          ).arg(absolutePath).arg(projectRelativePath)));
        }
    }

    assert(ret);
    if (!ret) return nullptr;

    if (!CEGUIProjectManager::Instance().isProjectLoaded() && ret->requiresProject())
    {
        ret.reset(new NoEditor(absolutePath,
            "Opening this file requires you to have a project opened!"));
    }

    // Will cleanup itself inside if something went wrong
    ret->initialize(/*this*/);

    // Intentionally before addTab for getEditorForTab()
    auto retPtr = ret.get();
    activeEditors.push_back(std::move(ret));

    const int newTabIdx = ui->tabs->addTab(retPtr->getWidget(), retPtr->getLabelText());
    ui->tabs->setTabToolTip(newTabIdx, retPtr->getFilePath());

    return retPtr;
}

void MainWindow::on_actionOpenFile_triggered()
{
    QString defaultDir;
    if (CEGUIProjectManager::Instance().isProjectLoaded())
        defaultDir = CEGUIProjectManager::Instance().getCurrentProject()->getAbsolutePathOf("");

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open File",
                                                    defaultDir,
                                                    editorFactoryFileFilters.join(";;"),
                                                    &editorFactoryFileFilters[0]);
    if (!fileName.isEmpty())
        openEditorTab(fileName);
}

void MainWindow::openRecentProject(const QString& path)
{
    if (QFileInfo(path).exists())
    {
        auto project = CEGUIProjectManager::Instance().getCurrentProject();
        if (project)
        {
            // Give user a chance to save changes if needed
            if (!on_actionCloseProject_triggered())
                return;
        }

        CEGUIProjectManager::Instance().loadProject(path);
        updateProjectDependentUI(CEGUIProjectManager::Instance().getCurrentProject());
    }
    else
    {
        QMessageBox msgBox(this);
        msgBox.setText("Project \"" + path + "\" was not found.");
        msgBox.setInformativeText("The project file does not exist; it may have been moved or deleted."
                                  " Do you want to remove it from the recently used list?");
        msgBox.addButton(QMessageBox::Cancel);
        auto removeButton = msgBox.addButton("&Remove", QMessageBox::YesRole);
        msgBox.setDefaultButton(removeButton);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.exec();

        if (msgBox.clickedButton() == removeButton)
            recentlyUsedProjects->removeRecentlyUsed(path);
    }
}

void MainWindow::openRecentFile(const QString& path)
{
    if (QFileInfo(path).exists())
    {
        openEditorTab(path);
    }
    else
    {
        QMessageBox msgBox(this);
        msgBox.setText("File \"" + path + "\" was not found.");
        msgBox.setInformativeText("The file does not exist; it may have been moved or deleted."
                                  " Do you want to remove it from the recently used list?");
        msgBox.addButton(QMessageBox::Cancel);
        auto removeButton = msgBox.addButton("&Remove", QMessageBox::YesRole);
        msgBox.setDefaultButton(removeButton);
        msgBox.exec();

        if (msgBox.clickedButton() == removeButton)
            recentlyUsedFiles->removeRecentlyUsed(path);
    }
}

void MainWindow::onUndoAvailable(bool available, const QString& text)
{
    ui->actionUndo->setEnabled(available);
    ui->actionUndo->setText(text.isEmpty() ? "Undo" : "Undo " + text);
}

void MainWindow::onRedoAvailable(bool available, const QString& text)
{
    ui->actionRedo->setEnabled(available);
    ui->actionRedo->setText(text.isEmpty() ? "Redo" : "Redo " + text);
}

void MainWindow::onEditorLabelChanged()
{
    EditorBase* editor = qobject_cast<EditorBase*>(sender());
    assert(editor);
    if (!editor) return;

    const int tabIndex = ui->tabs->indexOf(editor->getWidget());
    ui->tabs->setTabText(tabIndex, editor->getLabelText());
    ui->tabs->setTabToolTip(tabIndex, editor->getFilePath());
}

void MainWindow::onEditorContentsChanged(bool isModified)
{
    EditorBase* editor = qobject_cast<EditorBase*>(sender());
    assert(editor);
    if (!editor) return;

    if (editor == currentEditor)
        ui->actionSave->setEnabled(isModified);

    const int tabIndex = ui->tabs->indexOf(editor->getWidget());
    if (isModified)
        ui->tabs->setTabIcon(tabIndex, QIcon(":/icons/tabs/has_changes.png"));
    else
        ui->tabs->setTabIcon(tabIndex, QIcon());
}

// Pops a alert menu open asking the user s/he would like to reload the
// file due to external changes being made
void MainWindow::onEditorFileChangedExternally()
{
    // If multiple file writes are made by an external program,
    // multiple pop-ups will appear. Prevent that with a switch.
    if (displayingReloadAlert) return;

    displayingReloadAlert = true;

    // Slot is called directly on editor activation, sender will be null,
    // and we should use currentEditor in that case.
    EditorBase* editor = qobject_cast<EditorBase*>(sender());
    if (!editor) editor = currentEditor;
    if (!editor) return;

    auto ret = QMessageBox::question(this,
                                     "File has been modified externally!",
                                     "The file that you have currently opened has been modified outside the CEGUI Unified Editor."
                                     "\n\nReload the file?\n\nIf you select Yes, ALL UNDO HISTORY WILL BE DESTROYED!",
                                     QMessageBox::No | QMessageBox::Yes,
                                     QMessageBox::No); // defaulting to No is safer IMO

    editor->resolveSyncConflict(ret == QMessageBox::Yes);

    displayingReloadAlert = false;
}

void MainWindow::on_actionZoomIn_triggered()
{
    if (currentEditor) currentEditor->zoomIn();
}

void MainWindow::on_actionZoomOut_triggered()
{
    if (currentEditor) currentEditor->zoomOut();
}

void MainWindow::on_actionZoomReset_triggered()
{
    if (currentEditor) currentEditor->zoomReset();
}

void MainWindow::on_actionUndo_triggered()
{
    if (currentEditor) currentEditor->undo();
}

void MainWindow::on_actionRedo_triggered()
{
    if (currentEditor) currentEditor->redo();
}

void MainWindow::on_actionRevert_triggered()
{
    if (!currentEditor) return;

    auto ret = QMessageBox::question(this,
                                    "Are you sure you want to revert to file on hard disk?",
                                    "Reverting means that the file will be reloaded to the "
                                    "state it is in on the HDD.\n\nRevert?\n\n"
                                    "If you select Yes, ALL UNDO HISTORY MIGHT BE DESTROYED!\n"
                                    "(though I will preserve it if possible)",
                                    QMessageBox::No | QMessageBox::Yes,
                                    QMessageBox::No); // defaulting to No is safer IMO

    if (ret == QMessageBox::Yes) currentEditor->revert();
}

void MainWindow::on_actionCut_triggered()
{
    if (currentEditor) currentEditor->cut();
}

void MainWindow::on_actionCopy_triggered()
{
    if (currentEditor) currentEditor->copy();
}

void MainWindow::on_actionPaste_triggered()
{
    if (currentEditor) currentEditor->paste();
}

void MainWindow::on_actionDelete_triggered()
{
    if (currentEditor) currentEditor->deleteSelected();
}

void MainWindow::on_actionSave_triggered()
{
    if (currentEditor) currentEditor->save();
}

void MainWindow::on_actionSaveAs_triggered()
{
    if (!currentEditor) return;

    QString filePath = QFileDialog::getSaveFileName(this, "Save as", QFileInfo(currentEditor->getFilePath()).dir().path());
    if (!filePath.isEmpty()) currentEditor->saveAs(filePath);
}

// Saves all opened tabbed editors and opened project (if any)
void MainWindow::on_actionSaveAll_triggered()
{
    auto project =  CEGUIProjectManager::Instance().getCurrentProject();
    if (project) project->save();

    for (auto&& editor : activeEditors)
        editor->save();
}

void MainWindow::on_actionSaveProject_triggered()
{
    auto project =  CEGUIProjectManager::Instance().getCurrentProject();
    if (project) project->save();
}

bool MainWindow::on_actionCloseProject_triggered()
{
    auto project = CEGUIProjectManager::Instance().getCurrentProject();
    if (!project) return true;

    if (project->isModified())
    {
        auto result = QMessageBox::question(this,
                                            "Project file has changes!",
                                            "There are unsaved changes in the project file "
                                            "Do you want to save them? "
                                            "(Pressing Discard will discard the changes!)",
                                            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                            QMessageBox::Save);

        if (result == QMessageBox::Save)
            project->save();
        else if (result == QMessageBox::Cancel)
            return false;
    }

    updateProjectDependentUI(nullptr);
    CEGUIProjectManager::Instance().unloadProject();

    return true;
}

void MainWindow::on_actionReloadResources_triggered()
{
    // Since we are effectively unloading the project and potentially nuking resources of it
    // we should definitely unload all tabs that rely on it to prevent segfaults and other
    // nasty phenomena

    // We will remember previously opened tabs requiring a project so that we can load them up after we are done
    QStringList filePathsToLoad;
    int i = 0;
    while (i < ui->tabs->count())
    {
        auto editor = getEditorForTab(i);
        if (editor->requiresProject())
            filePathsToLoad.append(editor->getFilePath());

        ++i;
    }

    QString currEditorFilePath = currentEditor ? currentEditor->getFilePath() : "";

    if (!closeAllTabsRequiringProject())
    {
        QMessageBox::information(this,
                                 "Project dependent tabs still open!",
                                 "You can't reload project's resources while having tabs that "
                                 "depend on the project and its resources opened!");
        return;
    }

    CEGUIProjectManager::Instance().syncProjectToCEGUIInstance();

    // Load previously loaded tabs requiring a project opened
    for (auto& filePath : filePathsToLoad)
        openEditorTab(filePath);

    // Previously active editor to be loaded last, this makes it active again
    if (!currEditorFilePath.isEmpty())
        openEditorTab(currEditorFilePath);
}

void MainWindow::on_actionNewLayout_triggered()
{
    createNewFile("New Layout", { "Layout files (*.layout)" }, 0, "layout");
}

void MainWindow::on_actionNewImageset_triggered()
{
    createNewFile("New Imageset", { "Imageset files (*.imageset)" }, 0, "imageset");
}

void MainWindow::on_actionNewOtherFile_triggered()
{
    createNewFile("New File", QStringList(), 0, "");
}

void MainWindow::createNewFile(const QString& title, const QStringList& filters, int currFilter, const QString& autoSuffix)
{
    QString defaultDir;
    if (CEGUIProjectManager::Instance().isProjectLoaded())
        defaultDir = CEGUIProjectManager::Instance().getCurrentProject()->getAbsolutePathOf("");

/*
    # Qt (as of 4.8) does not support default suffix (extension) unless you use
    # non-native file dialogs with non-static methods (see QFileDialog.setDefaultSuffix).
    # HACK: We handle this differently depending on whether a default suffix is required

    if filtersList is None or len(filtersList) == 0 or not autoSuffix:
*/

    QString selectedFilter = (currFilter >= 0 && currFilter < filters.size()) ? filters[currFilter] : "";
    QString fileName;
    /*
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    title,
                                                    defaultDir,
                                                    filters.join(";;"),
                                                    &selectedFilter);
    */

    QFileDialog dialog(this, title, defaultDir, filters.join(";;"));
    dialog.selectNameFilter(selectedFilter);
    dialog.setDefaultSuffix(autoSuffix);

    if (dialog.exec())
        fileName = dialog.selectedFiles()[0];

    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(this, "Error creating file!",
                                  "CEED encountered an error trying to create a new file " + fileName);
            return;
        }
        file.close();
        openEditorTab(fileName);
    }

/*
    else:
        while True:
            fileName, selectedFilter = QtGui.QFileDialog.getSaveFileName(self,
                                                                         title,
                                                                         defaultDir,
                                                                         ";;".join(filtersList) if filtersList is not None and len(filtersList) > 0 else None,
                                                                         filtersList[selectedFilterIndex] if filtersList is not None and len(filtersList) > selectedFilterIndex else None,
                                                                         QtGui.QFileDialog.DontConfirmOverwrite)
            if not fileName:
                break

            # if there is no dot, append the selected filter's extension
            if fileName.find(".") == -1:
                # really ugly, handle with care
                # find last open paren
                i = selectedFilter.rfind("(")
                if i != -1:
                    # find next dot
                    i = selectedFilter.find(".", i)
                    if i != -1:
                        # find next space or close paren
                        k = selectedFilter.find(")", i)
                        l = selectedFilter.find(" ", i)
                        if l != -1 and l < k:
                            k = l
                        if k != -1:
                            selectedExt = selectedFilter[i:k]
                            if selectedExt.find("*") == -1 and selectedExt.find("?") == -1:
                                fileName += selectedExt

            # and now test & confirm overwrite
            try:
                if os.path.exists(fileName):
                    msgBox = QtGui.QMessageBox(self)
                    msgBox.setText("A file named \"%s\" already exists in \"%s\"." % (os.path.basename(fileName), os.path.dirname(fileName)))
                    msgBox.setInformativeText("Do you want to replace it, overwriting its contents?")
                    msgBox.addButton(QtGui.QMessageBox.Cancel)
                    replaceButton = msgBox.addButton("&Replace", QtGui.QMessageBox.YesRole)
                    msgBox.setDefaultButton(replaceButton)
                    msgBox.setIcon(QtGui.QMessageBox.Question)
                    msgBox.exec_()
                    if msgBox.clickedButton() != replaceButton:
                        continue
                f = open(fileName, "w")
                f.close()
            except IOError as e:
                QtGui.QMessageBox.critical(self, "Error creating file!",
                                                 "CEED encountered an error trying to create a new file.\n\n(exception details: %s)" % (e))
                return

            self.openEditorTab(fileName)
            break
*/
}
