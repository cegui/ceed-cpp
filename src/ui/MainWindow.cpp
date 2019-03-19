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
//#include "qopenglframebufferobject.h"
#include "src/Application.h"
#include "src/util/Settings.h"
#include "src/util/SettingsEntry.h"
#include "src/util/RecentlyUsed.h"
#include "src/util/DismissableMessage.h"
#include "src/proj/CEGUIProjectManager.h"
#include "src/proj/CEGUIProject.h"
#include "src/editors/NoEditor.h"
#include "src/editors/TextEditor.h"
#include "src/editors/BitmapEditor.h"
#include "src/ui/dialogs/AboutDialog.h"
#include "src/ui/dialogs/LicenseDialog.h"
#include "src/ui/dialogs//NewProjectDialog.h"
#include "src/ui/dialogs/ProjectSettingsDialog.h"
#include "src/ui/dialogs/MultiplePossibleFactoriesDialog.h"
#include "src/ui/dialogs/SettingsDialog.h"
#include "src/ui/ProjectManager.h"
#include "src/ui/FileSystemBrowser.h"
#include "src/ui/UndoViewer.h"

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

    // TODO: make the check work! Now crashes inside. Must setup OpenGL first?
    //if (!QOpenGLFramebufferObject::hasOpenGLFramebufferObjects())
    if (!ui) // to avoid 'code will never be executed' warning for now
    {
        DismissableMessage::warning(this, "No FBO support!",
            "CEED uses OpenGL frame buffer objects for various tasks, "
            "most notably to support panning and zooming in the layout editor.\n\n"
            "FBO support was not detected on your system!\n\n"
            "The editor will run but you may experience rendering artifacts.",
            "no_fbo_support");
    }

    // Register factories

    editorFactories.push_back(std::make_unique<TextEditorFactory>());
    editorFactories.push_back(std::make_unique<BitmapEditorFactory>());
    /*
        animation_list_editor.AnimationListTabbedEditorFactory(),           // Animation files
        bitmap_editor.BitmapTabbedEditorFactory(),                          // Bitmap files
        imageset_editor.ImagesetTabbedEditorFactory(),                      // Imageset files
        layout_editor.LayoutTabbedEditorFactory(),                          // Layout files
        looknfeel_editor.LookNFeelTabbedEditorFactory(),                    //
        #property_mappings_editor.PropertyMappingsTabbedEditorFactory(),    // Property Mapping files
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

    /*
        # we start CEGUI early and we always start it
        self.ceguiInstance = cegui.Instance()
        self.ceguiContainerWidget = cegui_container.ContainerWidget(self.ceguiInstance, self)
    */

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
    ui->menuEditor->setVisible(false);

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

void MainWindow::setupToolbars()
{
    // FIXME: here until I manage to create menu toolbutton in Qt Creator

    QToolBar* toolbar = ui->toolBarStandard; // createToolbar("Standard");
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

QToolBar* MainWindow::createToolbar(const QString& name)
{
    auto&& settings = qobject_cast<Application*>(qApp)->getSettings();
    auto tbIconSizeEntry = settings->getEntry("global/ui/toolbar_icon_size");
    assert(tbIconSizeEntry);

    QToolBar* toolbar = addToolBar(name);
    toolbar->setObjectName(name + " toolbar");

    const int iconSize = std::max(16, tbIconSizeEntry->value().toInt());
    toolbar->setIconSize(QSize(iconSize, iconSize));

    connect(tbIconSizeEntry, &SettingsEntry::valueChanged, [toolbar](const QVariant& newValue)
    {
        const int iconSize = std::max(16, newValue.toInt());
        toolbar->setIconSize(QSize(iconSize, iconSize));
    });

    return toolbar;
}

void MainWindow::updateUIOnProjectChanged()
{
    const bool isProjectLoaded = CEGUIProjectManager::Instance().isProjectLoaded();

    /*
        # view the newly opened project in the project manager
        self.projectManager.setProject(self.project)
    */

    if (isProjectLoaded)
    {
        auto project = CEGUIProjectManager::Instance().getCurrentProject();
        recentlyUsedProjects->addRecentlyUsed(project->filePath);

        // TODO: Maybe this could be configurable?
        auto baseDir = project->getAbsolutePathOf("");
        if (QFileInfo(baseDir).isDir())
            fsBrowser->setDirectory(baseDir);
    }
    else
    {
        fsBrowser->setDirectory(QDir::homePath());
    }

    ui->actionSaveProject->setEnabled(isProjectLoaded);
    ui->actionCloseProject->setEnabled(isProjectLoaded);
    ui->actionProjectSettings->setEnabled(isProjectLoaded);
    ui->actionReloadResources->setEnabled(isProjectLoaded);
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
        // If the slot returned False, user pressed Cancel
        /*
            if not self.slot_closeProject():
                # in case user pressed cancel the entire quitting processed has to be terminated
                return False
        */
    }

    QApplication::quit();

    return true;
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
    CEGUIProjectManager::Instance().unloadProject();

/*
        //!!!get settings from newProjectDialog!
        newProject = CEGUIProjectManager::Instance().createProject();
        newProject.save()

        # open the settings window after creation so that user can further customise their
        # new project file
        self.openProject(path = newProject.projectFilePath, openSettings = True)

        # save the project with the settings that were potentially set in the project settings dialog
        self.saveProject()
*/
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

        CEGUIProjectManager::Instance().unloadProject();
    }

    auto fileName = QFileDialog::getOpenFileName(this,
                                                "Open existing project file",
                                                "",
                                                "Project files (*.project)");

    if (!fileName.isEmpty())
    {
        CEGUIProjectManager::Instance().loadProject(fileName);
        updateUIOnProjectChanged();
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
        /*
            self.performProjectDirectoriesSanityCheck()
            self.syncProjectToCEGUIInstance()
        */
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
    //QDesktopServices::openUrl(QUrl("file://%s" % (os.path.join(paths.DOC_DIR, "quickstart-guide.pdf")), QUrl::TolerantMode));
}

void MainWindow::on_actionUserManual_triggered()
{
    //QDesktopServices::openUrl(QUrl("file://%s" % (os.path.join(paths.DOC_DIR, "user-manual.pdf")), QUrl::TolerantMode));
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
    //self.ceguiContainerWidget.debugInfo.show()
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
        //auto tabWidget = tabs->widget(tabIdx);
        menu->addSeparator();
        QAction* dataTypeAction = new QAction("Data type: " /*+ (tabWidget.getDesiredSavingDataType())*/, this);
        dataTypeAction->setToolTip("Displays which data type this file will be saved to (the desired saving data type).");
        menu->addAction(dataTypeAction);
    }

    menu->exec(ui->tabs->tabBar()->mapToGlobal(pos));
}

void MainWindow::on_tabs_currentChanged(int index)
{
    // To fight flicker
    ui->tabs->setUpdatesEnabled(false);

    auto widget = ui->tabs->widget(index);
    if (currentEditor)
        currentEditor->deactivate();

    // It's the tabbed editor's responsibility to handle these, we disable them by default,
    // also reset their texts in case the tabbed editor messed with them
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionUndo->setText("Undo");
    ui->actionRedo->setText("Redo");

    // Set undo stack to None as we have no idea whether the previous tab editor
    // set it to something else
    undoViewer->setUndoStack(nullptr);

    statusBar()->clearMessage();

    currentEditor = getEditorForTab(widget);

    const bool hasEditor = (currentEditor != nullptr);
    fsBrowser->activeFileDirectoryButton()->setEnabled(hasEditor);
    ui->actionRevert->setEnabled(hasEditor);
    ui->actionSave->setEnabled(hasEditor);
    ui->actionSaveAs->setEnabled(hasEditor);
    ui->actionCloseTab->setEnabled(hasEditor);
    ui->actionCloseOtherTabs->setEnabled(hasEditor);

    if (currentEditor)
    {
        /*
        wdt.tabbedEditor.activate()

        undoViewer.setUndoStack(self.getUndoStack())
        */
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

    editor->finalize();
    editor->destroy();

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

    ui->tabs->addTab(retPtr->getWidget(), retPtr->getLabelText());

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
        /*
                if self.project:
                    # give user a chance to save changes if needed
                    if not self.slot_closeProject():
                        return

                self.openProject(absolutePath)
        */
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

/*

class MainWindow(QtGui.QMainWindow):

    project = property(lambda self: self._project,
                       lambda self, value: self._setProject(value))

    def _setProject(self, value):
        self._project = value
        if self.fileSystemBrowser:
            self.fileSystemBrowser.projectDirectoryButton.setEnabled(True if value else False)

    def setupActions(self):
        # usage of a connection group in mainwindow may be unnecessary,
        # we never use disconnectAll and/or connectAll, it is just used as a convenient
        # way to group connections
        self.connectionGroup = action.ConnectionGroup(self.actionManager)

        #
        # get and connect all actions we care about
        #
        self.newFileAction = self.actionManager.getAction("files/new_file")
        self.connectionGroup.add(self.newFileAction, receiver = self.slot_newFileDialog)

        self.newLayoutAction = self.actionManager.getAction("files/new_layout")
        self.connectionGroup.add(self.newLayoutAction, receiver = self.slot_newLayoutDialog)

        self.newImagesetAction = self.actionManager.getAction("files/new_imageset")
        self.connectionGroup.add(self.newImagesetAction, receiver = self.slot_newImagesetDialog)

        self.openFileAction = self.actionManager.getAction("files/open_file")
        self.connectionGroup.add(self.openFileAction, receiver = self.slot_openFileDialog)

        self.saveAction = self.actionManager.getAction("files/save_file")
        self.saveAction.setEnabled(False)
        self.connectionGroup.add(self.saveAction, receiver = self.slot_save)

        self.saveAsAction = self.actionManager.getAction("files/save_file_as")
        self.saveAsAction.setEnabled(False)
        self.connectionGroup.add(self.saveAsAction, receiver = self.slot_saveAs)

        self.saveAllAction = self.actionManager.getAction("files/save_all")
        self.connectionGroup.add(self.saveAllAction, receiver = self.slot_saveAll)

        # tab bar context menu (but also added to the file menu so it's easy to discover)
        self.closeTabAction = self.actionManager.getAction("files/close_current_tab")
        self.closeTabAction.setEnabled(False)
        self.connectionGroup.add(self.closeTabAction, receiver = self.slot_closeTab)

        self.closeOtherTabsAction = self.actionManager.getAction("files/close_other_tabs")
        self.closeOtherTabsAction.setEnabled(False)
        self.connectionGroup.add(self.closeOtherTabsAction, receiver = self.slot_closeOtherTabs)

        self.closeAllTabsAction = self.actionManager.getAction("files/close_all_tabs")
        self.connectionGroup.add(self.closeAllTabsAction, receiver = self.slot_closeAllTabs)
        # end of tab bar context menu

        self.previousTabAction = self.actionManager.getAction("files/previous_tab")
        self.connectionGroup.add(self.previousTabAction, receiver = self.slot_previousTab)

        self.nextTabAction = self.actionManager.getAction("files/next_tab")
        self.connectionGroup.add(self.nextTabAction, receiver = self.slot_nextTab)

        self.undoAction = self.actionManager.getAction("all_editors/undo")
        self.undoAction.setEnabled(False)
        self.connectionGroup.add(self.undoAction, receiver = self.slot_undo)

        self.redoAction = self.actionManager.getAction("all_editors/redo")
        self.redoAction.setEnabled(False)
        self.connectionGroup.add(self.redoAction, receiver = self.slot_redo)

        self.revertAction = self.actionManager.getAction("files/revert_file")
        self.revertAction.setEnabled(False)
        self.connectionGroup.add(self.revertAction, receiver = self.slot_revert)

        self.cutAction = self.actionManager.getAction("all_editors/cut")
        self.connectionGroup.add(self.cutAction, receiver = self.slot_cut)

        self.copyAction = self.actionManager.getAction("all_editors/copy")
        self.connectionGroup.add(self.copyAction, receiver = self.slot_copy)

        self.pasteAction = self.actionManager.getAction("all_editors/paste")
        self.connectionGroup.add(self.pasteAction, receiver = self.slot_paste)

        self.deleteAction = self.actionManager.getAction("all_editors/delete")
        self.connectionGroup.add(self.deleteAction, receiver = self.slot_delete)

        self.projectSettingsAction = self.actionManager.getAction("project_management/project_settings")
        self.projectSettingsAction.setEnabled(False)
        self.connectionGroup.add(self.projectSettingsAction, receiver = self.slot_projectSettings)

        self.projectReloadResourcesAction = self.actionManager.getAction("project_management/reload_resources")
        self.projectReloadResourcesAction.setEnabled(False)
        self.connectionGroup.add(self.projectReloadResourcesAction, receiver = self.slot_projectReloadResources)

        self.preferencesAction = self.actionManager.getAction("general/application_settings")
        self.connectionGroup.add(self.preferencesAction, receiver = self.settingsInterface.show)

        self.newProjectAction = self.actionManager.getAction("project_management/new_project")
        self.connectionGroup.add(self.newProjectAction, receiver = self.slot_newProject)

        self.openProjectAction = self.actionManager.getAction("project_management/open_project")
        self.connectionGroup.add(self.openProjectAction, receiver = self.slot_openProject)

        self.saveProjectAction = self.actionManager.getAction("project_management/save_project")
        self.saveProjectAction.setEnabled(False)
        self.connectionGroup.add(self.saveProjectAction, receiver = self.slot_saveProject)

        self.closeProjectAction = self.actionManager.getAction("project_management/close_project")
        self.closeProjectAction.setEnabled(False)
        self.connectionGroup.add(self.closeProjectAction, receiver = self.slot_closeProject)

        self.quitAction = self.actionManager.getAction("general/quit")
        self.connectionGroup.add(self.quitAction, receiver = self.slot_quit)

        self.helpQuickstartAction = self.actionManager.getAction("general/help_quickstart")
        self.connectionGroup.add(self.helpQuickstartAction, receiver = self.slot_helpQuickstart)

        self.helpUserManualAction = self.actionManager.getAction("general/help_user_manual")
        self.connectionGroup.add(self.helpUserManualAction, receiver = self.slot_helpUserManual)

        self.helpWikiPageAction = self.actionManager.getAction("general/help_wiki_page")
        self.connectionGroup.add(self.helpWikiPageAction, receiver = self.slot_helpWikiPage)

        self.sendFeedbackAction = self.actionManager.getAction("general/send_feedback")
        self.connectionGroup.add(self.sendFeedbackAction, receiver = self.slot_sendFeedback)

        self.reportBugAction = self.actionManager.getAction("general/report_bug")
        self.connectionGroup.add(self.reportBugAction, receiver = self.slot_reportBug)

        self.ceguiDebugInfoAction = self.actionManager.getAction("general/cegui_debug_info")
        self.connectionGroup.add(self.ceguiDebugInfoAction, receiver = self.slot_ceguiDebugInfo)

        self.viewLicenseAction = self.actionManager.getAction("general/view_license")
        self.connectionGroup.add(self.viewLicenseAction, receiver = self.slot_license)

        self.aboutQtAction = self.actionManager.getAction("general/about_qt")
        self.connectionGroup.add(self.aboutQtAction, receiver = QtGui.QApplication.aboutQt)

        self.aboutAction = self.actionManager.getAction("general/about")
        self.connectionGroup.add(self.aboutAction, receiver = self.slot_about)

        self.zoomInAction = self.actionManager.getAction("all_editors/zoom_in")
        self.connectionGroup.add(self.zoomInAction, receiver = self.slot_zoomIn)
        self.zoomOutAction = self.actionManager.getAction("all_editors/zoom_out")
        self.connectionGroup.add(self.zoomOutAction, receiver = self.slot_zoomOut)
        self.zoomResetAction = self.actionManager.getAction("all_editors/zoom_reset")
        self.connectionGroup.add(self.zoomResetAction, receiver = self.slot_zoomReset)

        self.statusbarAction = self.actionManager.getAction("general/statusbar")
        self.statusbarAction.setChecked(True)
        self.connectionGroup.add(self.statusbarAction, receiver = self.slot_toggleStatusbar)

        self.fullScreenAction = self.actionManager.getAction("general/full_screen")
        self.connectionGroup.add(self.fullScreenAction, receiver = self.slot_toggleFullScreen)

        self.connectionGroup.connectAll()

    def syncProjectToCEGUIInstance(self, indicateErrorsWithDialogs = True):
        """Synchronises current project to the CEGUI instance.

        indicateErrorsWithDialogs - if True a dialog is opened in case of errors

        Returns True if the procedure was successful
        """

        try:
            self.ceguiInstance.syncToProject(self.project, self)

            return True

        except Exception as e:
            if indicateErrorsWithDialogs:
                QtGui.QMessageBox.warning(self, "Failed to synchronise embedded CEGUI to your project",
"""An attempt was made to load resources related to the project being opened, for some reason the loading didn't succeed so all resources were destroyed! The most likely reason is that the resource directories are wrong, this can be very easily remedied in the project settings.

This means that editing capabilities of CEED will be limited to editing of files that don't require a project opened (for example: imagesets).

Details of this error: %s""" % (e))

            return False

    def performProjectDirectoriesSanityCheck(self, indicateErrorsWithDialogs = True):
        try:
            self.project.checkAllDirectories()

            return True

        except IOError as e:
            if indicateErrorsWithDialogs:
                QtGui.QMessageBox.warning(self, "At least one of project's resource directories is invalid",
"""Project's resource directory paths didn't pass the sanity check, please check projects settings.

Details of this error: %s""" % (e))

            return False

    def openProject(self, path, openSettings = False):
        """Opens the project file given in 'path'. Assumes no project is opened at the point this is called.
        The slot_openProject method will test if a project is opened and close it accordingly (with a dialog
        being shown if there are changes to it)

        Errors aren't indicated by exceptions or return values, dialogs are shown in case of errors.

        path - Absolute path of the project file
        openSettings - if True, the settings dialog is opened instead of just loading the resources,
                       this is desirable when creating a new project
        """

        assert(self.project is None)

        # reset project manager to a clean state just in case
        self.projectManager.setProject(None)

        self.project = project.Project()
        try:
            self.project.load(path)
        except IOError:
            QtGui.QMessageBox.critical(self, "Error when opening project", "It seems project at path '%s' doesn't exist or you don't have rights to open it." % (path))

            self.project = None
            return

        self.performProjectDirectoriesSanityCheck()

        # view the newly opened project in the project manager
        self.projectManager.setProject(self.project)
        # and set the filesystem browser path to the base folder of the project
        # TODO: Maybe this could be configurable?
        projectBaseDirectory = self.project.getAbsolutePathOf("")
        if os.path.isdir(projectBaseDirectory):
            self.fileSystemBrowser.setDirectory(projectBaseDirectory)

        self.recentlyUsedProjects.addRecentlyUsed(self.project.projectFilePath)

        # and enable respective actions
        self.saveProjectAction.setEnabled(True)
        self.closeProjectAction.setEnabled(True)
        self.projectSettingsAction.setEnabled(True)
        self.projectReloadResourcesAction.setEnabled(True)

        if openSettings:
            self.slot_projectSettings()

        else:
            self.syncProjectToCEGUIInstance()

    def closeProject(self):
        """Closes currently opened project. Assumes one is opened at the point this is called.
        """

        assert(self.project is not None)

        # since we are effectively unloading the project and potentially nuking resources of it
        # we should definitely unload all tabs that rely on it to prevent segfaults and other
        # nasty phenomena
        if not self.closeAllTabsRequiringProject():
            return

        self.projectManager.setProject(None)
        # TODO: Do we really want to call this there? This was already called when the project was being opened.
        #       It doesn't do anything harmful but maybe is unnecessary.
        self.recentlyUsedProjects.addRecentlyUsed(self.project.projectFilePath)
        # clean resources that were potentially used with this project
        self.ceguiInstance.cleanCEGUIResources()

        self.project.unload()
        self.project = None

        # as the project was closed be will disable actions related to it
        self.saveProjectAction.setEnabled(False)
        self.closeProjectAction.setEnabled(False)
        self.projectReloadResourcesAction.setEnabled(False)

    def saveProject(self):
        """Saves currently opened project to the file it was opened from (or the last file it was saved to).
        """

        assert(self.project is not None)

        self.project.save()

    def saveProjectAs(self, newPath):
        """Saves currently opened project to a custom path. For best reliability, use absolute file path as newPath
        """

        self.project.save(newPath)
        # set the project's file path to newPath so that if you press save next time it will save to the new path
        # (This is what is expected from applications in general I think)
        self.project.projectFilePath = newPath

    def getFilePathsOfAllTabsRequiringProject(self):
        """Queries file paths of all tabs that require a project opened

        This is used to bring previously closed tabs back up when reloading project resources
        """

        ret = []
        i = 0
        while i < self.tabs.count():
            tabbedEditor = self.tabs.widget(i).tabbedEditor

            if tabbedEditor.requiresProject:
                ret.append(tabbedEditor.filePath)

            i += 1

        return ret

    def slot_saveProject(self):
        self.saveProject()

    def slot_closeProject(self):
        assert(self.project)

        if self.project.hasChanges():
            result = QtGui.QMessageBox.question(self,
                                                "Project file has changes!",
                                                "There are unsaved changes in the project file "
                                                "Do you want to save them? "
                                                "(Pressing Discard will discard the changes!)",
                                                QtGui.QMessageBox.Save | QtGui.QMessageBox.Discard | QtGui.QMessageBox.Cancel,
                                                QtGui.QMessageBox.Save)

            if result == QtGui.QMessageBox.Save:
                self.saveProject()

            elif result == QtGui.QMessageBox.Cancel:
                return False

        self.closeProject()
        return True

    def slot_projectReloadResources(self):
        # since we are effectively unloading the project and potentially nuking resources of it
        # we should definitely unload all tabs that rely on it to prevent segfaults and other
        # nasty phenomena

        # we will remember previously opened tabs requiring a project so that we can load them up
        # after we are done
        filePathsToLoad = self.getFilePathsOfAllTabsRequiringProject()
        activeEditorPath = self.activeEditor.filePath if self.activeEditor else ""

        if not self.closeAllTabsRequiringProject():
            QtGui.QMessageBox.information(self,
                                          "Project dependent tabs still open!",
                                          "You can't reload project's resources while having tabs that "
                                          "depend on the project and its resources opened!")
            return

        self.performProjectDirectoriesSanityCheck()
        self.syncProjectToCEGUIInstance()

        # load previously loaded tabs requiring a project opened
        for filePath in filePathsToLoad:
            self.openEditorTab(filePath)

        # previously active editor to be loaded last, this makes it active again
        if activeEditorPath != "":
            self.openEditorTab(activeEditorPath)

    def slot_newFileDialog(self, title = "New File", filtersList = None, selectedFilterIndex = 0, autoSuffix = False):
        defaultDir = ""
        if self.project:
            defaultDir = self.project.getAbsolutePathOf("")

        # Qt (as of 4.8) does not support default suffix (extension) unless you use
        # non-native file dialogs with non-static methods (see QFileDialog.setDefaultSuffix).
        # HACK: We handle this differently depending on whether a default suffix is required

        if filtersList is None or len(filtersList) == 0 or not autoSuffix:
            fileName, selectedFilter = QtGui.QFileDialog.getSaveFileName(self,
                                                                         title,
                                                                         defaultDir,
                                                                         ";;".join(filtersList) if filtersList is not None and len(filtersList) > 0 else None,
                                                                         filtersList[selectedFilterIndex] if filtersList is not None and len(filtersList) > selectedFilterIndex else None)
            if fileName:
                try:
                    f = open(fileName, "w")
                    f.close()
                except IOError as e:
                    QtGui.QMessageBox.critical(self, "Error creating file!",
                                                     "CEED encountered an error trying to create a new file.\n\n(exception details: %s)" % (e))
                    return

                self.openEditorTab(fileName)
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

    def slot_newLayoutDialog(self):
        self.slot_newFileDialog(title = "New Layout",
                                filtersList = ["Layout files (*.layout)"],
                                autoSuffix = True)

    def slot_newImagesetDialog(self):
        self.slot_newFileDialog(title = "New Imageset",
                                filtersList = ["Imageset files (*.imageset)"],
                                autoSuffix = True)

    def slot_closeOtherTabs(self):
        current = self.tabs.currentWidget()

        i = 0
        while i < self.tabs.count():
            if self.tabs.widget(i) == current:
                # we skip the current widget
                i += 1
            else:
                if not self.slot_tabCloseRequested(i):
                    # user selected Cancel, we skip this widget
                    i += 1

    def slot_closeAllTabs(self):
        i = 0
        while i < self.tabs.count():
            if not self.slot_tabCloseRequested(i):
                # user selected Cancel, we skip this widget
                i += 1

    def slot_previousTab(self):
        if self.tabs.count() <= 1:
            return
        index = self.tabs.currentIndex() - 1
        if index < 0:
            index = self.tabs.count() + index
        self.tabs.setCurrentIndex(index)

    def slot_nextTab(self):
        if self.tabs.count() <= 1:
            return
        index = (self.tabs.currentIndex() + 1) % self.tabs.count()
        self.tabs.setCurrentIndex(index)

    def slot_save(self):
        if self.activeEditor:
            self.activeEditor.save()

    def slot_saveAs(self):
        if self.activeEditor:
            filePath, _ = QtGui.QFileDialog.getSaveFileName(self, "Save as", os.path.dirname(self.activeEditor.filePath))
            if filePath: # make sure user hasn't cancelled the dialog
                self.activeEditor.saveAs(filePath)

    def slot_saveAll(self):
        """Saves all opened tabbed editors and opened project (if any)
        """

        if self.project is not None:
            self.project.save()

        for editor in self.tabEditors:
            editor.save()
*/
