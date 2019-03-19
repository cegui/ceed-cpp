#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "qtoolbar.h"
#include "qtoolbutton.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "qdesktopservices.h"
#include "qtabbar.h"
//#include "qopenglframebufferobject.h"
#include "src/Application.h"
#include "src/util/Settings.h"
#include "src/util/SettingsEntry.h"
#include "src/util/RecentlyUsed.h"
#include "src/proj/CEGUIProjectManager.h"
#include "src/proj/CEGUIProject.h"
#include "src/editors/NoEditor.h"
#include "src/editors/TextEditor.h"
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
    /*
        self.actionManager = action.ActionManager(self, self.app.settings)
    */

    settingsDialog = new SettingsDialog(this);

    recentlyUsedProjects = new RecentlyUsedMenuEntry("Projects", this);
    recentlyUsedFiles = new RecentlyUsedMenuEntry("Files", this);

    //if (!QOpenGLFramebufferObject::hasOpenGLFramebufferObjects())
    {
        /*
        ceed.messages.warning(self.app, self, "No FBO support!",
            "CEED uses OpenGL frame buffer objects for various tasks, "
            "most notably to support panning and zooming in the layout editor.\n\n"
            "FBO support was not detected on your system!\n\n"
            "The editor will run but you may experience rendering artifacts.",
            "no_fbo_support")
        */
    }

    editorFactories.push_back(std::make_unique<TextEditorFactory>());
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

    ui->setupUi(this);

    /*
        # we start CEGUI early and we always start it
        self.ceguiInstance = cegui.Instance()
        self.ceguiContainerWidget = cegui_container.ContainerWidget(self.ceguiInstance, self)
    */

    tabs = centralWidget()->findChild<QTabWidget*>("tabs");
    tabs->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabs->tabBar(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_tabBarCustomContextMenuRequested(QPoint)));

    projectManager = new ProjectManager();
    //projectManager->setVisible(false);
    connect(projectManager, &ProjectManager::itemOpenRequested, this, &MainWindow::openEditorTab);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, projectManager);

    fsBrowser = new FileSystemBrowser();
    //fsBrowser->setVisible(false);
    connect(fsBrowser, &FileSystemBrowser::fileOpenRequested, this, &MainWindow::openEditorTab);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, fsBrowser);

    undoViewer = new UndoViewer();
    undoViewer->setVisible(false);
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, undoViewer);

    //actionStatusbar->setChecked(statusBar()->isVisible());

    //setupActions()
    //setupMenus()
    setupToolbars();

    //restoreSettings()
}

MainWindow::~MainWindow()
{
    delete projectManager;
    delete ui;
}

void MainWindow::setupToolbars()
{
    // Here until I manage to create menu toolbutton in Qt Creator
    QToolBar* toolbar = createToolbar("Standard");
    QToolButton* newMenuBtn = new QToolButton(this);
    newMenuBtn->setText("New");
    newMenuBtn->setToolTip("New file");
    newMenuBtn->setIcon(QIcon(":/icons/actions/new_file.png"));
    newMenuBtn->setPopupMode(QToolButton::InstantPopup);
    newMenuBtn->setMenu(menuBar()->findChild<QMenu*>("menu_New"));
    toolbar->addWidget(newMenuBtn);
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

    //!!! true -> isProjectLoaded!
    /*
        # and enable respective actions
        self.saveProjectAction.setEnabled(True)
        self.closeProjectAction.setEnabled(True)
        self.projectSettingsAction.setEnabled(True)
        self.projectReloadResourcesAction.setEnabled(True)
    */
}

// Safely quits the editor, prompting user to save changes to files and the project.
void MainWindow::on_actionQuit_triggered()
{
    /*
        self.saveSettings()

    */

    // We remember last tab we closed to check whether user pressed Cancel in any of the dialogs
    QWidget* lastTab = nullptr;
    while (!activeEditors.empty())
    {
        QWidget* currTab = tabs->widget(0);
        if (currTab == lastTab)
        {
            // User pressed cancel on one of the tab editor 'save without changes' dialog,
            // cancel the whole quit operation!
            return; // false
        }

        lastTab = currTab;

        on_tabs_tabCloseRequested(0);
    }

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
    auto tabIdx = tabs->tabBar()->tabAt(pos);
    tabs->setCurrentIndex(tabIdx);

    QMenu* menu = new QMenu(this);
    /*
        menu.addAction(self.closeTabAction)
    */
    menu->addSeparator();
    /*
        menu.addAction(self.closeOtherTabsAction)
        menu.addAction(self.closeAllTabsAction)
    */

    if (tabIdx >= 0)
    {
        //auto tabWidget = tabs->widget(tabIdx);
        menu->addSeparator();
        QAction* dataTypeAction = new QAction("Data type: " /*+ (tabWidget.getDesiredSavingDataType())*/, this);
        dataTypeAction->setToolTip("Displays which data type this file will be saved to (the desired saving data type).");
        menu->addAction(dataTypeAction);
    }

    menu->exec(tabs->tabBar()->mapToGlobal(pos));
}

void MainWindow::on_tabs_currentChanged(int index)
{
    // To fight flicker
    tabs->setUpdatesEnabled(false);

    auto widget = tabs->widget(index);
    if (currentEditor)
        currentEditor->deactivate();

    // It's the tabbed editor's responsibility to handle these, we disable them by default,
    // also reset their texts in case the tabbed editor messed with them
    /*
        self.undoAction.setEnabled(False)
        self.redoAction.setEnabled(False)
        self.undoAction.setText("Undo")
        self.redoAction.setText("Redo")
    */

    // Set undo stack to None as we have no idea whether the previous tab editor
    // set it to something else
    undoViewer->setUndoStack(nullptr);

    statusBar()->clearMessage();

    if (widget)
    {
        /*
        self.revertAction.setEnabled(True)

        self.saveAction.setEnabled(True)
        self.saveAsAction.setEnabled(True)

        self.closeTabAction.setEnabled(True)
        self.closeOtherTabsAction.setEnabled(True)

        wdt.tabbedEditor.activate()
        */
    }
    else
    {
        // None is selected right now, lets disable appropriate actions
        /*
            self.revertAction.setEnabled(False)

            self.saveAction.setEnabled(False)
            self.saveAsAction.setEnabled(False)

            self.closeTabAction.setEnabled(False)
            self.closeOtherTabsAction.setEnabled(False)
        */
    }

    tabs->setUpdatesEnabled(true);
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
    on_tabs_tabCloseRequested(tabs->currentIndex());
}

// Opens editor tab. Creates new editor if such file wasn't opened yet and if it was opened,
// it just makes the tab current.
void MainWindow::openEditorTab(const QString& absolutePath)
{
    if (activateEditorTabByFilePath(absolutePath)) return;

    EditorBase* editor = createEditorForFile(absolutePath);
    if (!editor) return;

    tabs->setCurrentWidget(editor->getWidget());
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
    while (i < tabs->count())
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
    return getEditorForTab(tabs->widget(index));
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
            tabs->setCurrentWidget(editor->getWidget());
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

    tabs->addTab(ret->getWidget(), ret->getLabelText());

    activeEditors.push_back(std::move(ret));

    return activeEditors.back().get();
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
