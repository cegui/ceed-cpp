#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ProjectManager.h"
#include "qtoolbar.h"
#include "qtoolbutton.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "qdesktopservices.h"
#include "qtabbar.h"
//#include "qopenglframebufferobject.h"
#include "src/proj/CEGUIProjectManager.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    /*
        # we have to construct ActionManager before settings interface (as it alters the settings declaration)!
        self.actionManager = action.ActionManager(self, self.app.settings)

        self.settingsInterface = settings.interface.QtSettingsInterface(self.app.settings)

        self.recentlyUsedProjects = recentlyused.RecentlyUsedMenuEntry(self.app.qsettings, "Projects")
        self.recentlyUsedFiles = recentlyused.RecentlyUsedMenuEntry(self.app.qsettings, "Files")
    */

    /*
    if (!QOpenGLFramebufferObject::hasOpenGLFramebufferObjects())
    {
        ceed.messages.warning(self.app, self, "No FBO support!",
            "CEED uses OpenGL frame buffer objects for various tasks, "
            "most notably to support panning and zooming in the layout editor.\n\n"
            "FBO support was not detected on your system!\n\n"
            "The editor will run but you may experience rendering artifacts.",
            "no_fbo_support")
    }
    */

    /*
        import ceed.editors.animation_list as animation_list_editor
        import ceed.editors.bitmap as bitmap_editor
        import ceed.editors.imageset as imageset_editor
        import ceed.editors.layout as layout_editor
        import ceed.editors.looknfeel as looknfeel_editor
        #import ceed.editors.property_mappings as property_mappings_editor
        import ceed.editors.text as text_editor

        self.editorFactories = [
            animation_list_editor.AnimationListTabbedEditorFactory(),
            bitmap_editor.BitmapTabbedEditorFactory(),
            imageset_editor.ImagesetTabbedEditorFactory(),
            layout_editor.LayoutTabbedEditorFactory(),
            looknfeel_editor.LookNFeelTabbedEditorFactory(),
            #property_mappings_editor.PropertyMappingsTabbedEditorFactory(),
            text_editor.TextTabbedEditorFactory()
        ]
        # File dialog filters, keep indices in sync with the list above
        self.editorFactoryFileFilters = [
            "Animation files (%s)" % ("*." + " *.".join(self.editorFactories[0].getFileExtensions())),
            "Bitmap files (%s)" % ("*." + " *.".join(self.editorFactories[1].getFileExtensions())),
            "Imageset files (%s)" % ("*." + " *.".join(self.editorFactories[2].getFileExtensions())),
            "Layout files (%s)" % ("*." + " *.".join(self.editorFactories[3].getFileExtensions())),
            #"Property Mapping files (%s)" % ("*." + " *.".join(self.editorFactories[4].getFileExtensions())),
            "Text files (%s)" % ("*." + " *.".join(self.editorFactories[4].getFileExtensions()))
        ]
        allExt = []
        for factory in self.editorFactories:
            allExt.extend(factory.getFileExtensions())
        self.editorFactoryFileFilters.insert(0, "All known files (*." + " *.".join(allExt) + ")")
        self.editorFactoryFileFilters.insert(1, "All files (*)")

    */

    ui->setupUi(this);

    /*
        # we start CEGUI early and we always start it
        self.ceguiInstance = cegui.Instance()
        self.ceguiContainerWidget = cegui_container.ContainerWidget(self.ceguiInstance, self)
    */

    auto tabs = centralWidget()->findChild<QTabWidget*>("tabs");
    tabs->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabs->tabBar(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_tabBarCustomContextMenuRequested(QPoint)));

    /*
        # stores all active tab editors
        self.tabEditors = []
    */

    projectManager = new ProjectManager();
    //projectManager.fileOpenRequested.connect(self.slot_openFile)
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, projectManager);

    /*
        self.fileSystemBrowser = filesystembrowser.FileSystemBrowser()
        self.fileSystemBrowser.setVisible(False)
        self.fileSystemBrowser.fileOpenRequested.connect(self.slot_openFile)
        self.addDockWidget(QtCore.Qt.RightDockWidgetArea, self.fileSystemBrowser)

        self.undoViewer = commands.UndoViewer()
        self.undoViewer.setVisible(False)
        self.addDockWidget(QtCore.Qt.RightDockWidgetArea, self.undoViewer)
    */

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
    QToolBar* toolbar = addToolBar(name);
    /*
            tb.setObjectName("%s toolbar" % (name))
            tbIconSizeEntry = self.app.settings.getEntry("global/ui/toolbar_icon_size")

            def updateToolbarIconSize(toolbar, size):
                if size < 16:
                    size = 16
                toolbar.setIconSize(QtCore.QSize(size, size))

            updateToolbarIconSize(tb, tbIconSizeEntry.value)
            tbIconSizeEntry.subscribe(lambda value: updateToolbarIconSize(tb, value))
    */
    return toolbar;
}

void MainWindow::updateUIOnProjectChanged()
{
    //bool isProjectLoaded = CEGUIProjectManager::Instance().isProjectLoaded();

    /*
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
    */
}

void MainWindow::on_actionQuit_triggered()
{
    /*
        """Safely quits the editor, prompting user to save changes to files and the project."""

        self.saveSettings()

        # we remember last tab we closed to check whether user pressed Cancel in any of the dialogs
        lastTab = None
        while len(self.tabEditors) > 0:
            currentTab = self.tabs.widget(0)
            if currentTab == lastTab:
                # user pressed cancel on one of the tab editor 'save without changes' dialog,
                # cancel the whole quit operation!
                return False
            lastTab = currentTab

            self.slot_tabCloseRequested(0)

        # Close project after all tabs have been closed, there may be tabs requiring a project opened!
        if self.project is not None:
            # if the slot returned False, user pressed Cancel
            if not self.slot_closeProject():
                # in case user pressed cancel the entire quitting processed has to be terminated
                return False
    */

    QApplication::quit();
}

void MainWindow::on_actionStatusbar_toggled(bool isChecked)
{
    statusBar()->setVisible(isChecked);
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

void MainWindow::on_tabs_currentChanged(int index)
{
    /*
        # to fight flicker
        self.tabs.setUpdatesEnabled(False)

        # FIXME: workaround for PySide 1.0.6, I suspect this is a bug in PySide! http://bugs.pyside.org/show_bug.cgi?id=988
        if index is None:
            index = -1

        elif isinstance(index, QtGui.QWidget):
            for i in xrange(0, self.tabs.count()):
                if index is self.tabs.widget(i):
                    index = i
                    break

            assert(not isinstance(index, QtGui.QWidget))
        # END OF FIXME

        wdt = self.tabs.widget(index)

        if self.activeEditor:
            self.activeEditor.deactivate()

        # it's the tabbed editor's responsibility to handle these,
        # we disable them by default
        self.undoAction.setEnabled(False)
        self.redoAction.setEnabled(False)
        # also reset their texts in case the tabbed editor messed with them
        self.undoAction.setText("Undo")
        self.redoAction.setText("Redo")
        # set undo stack to None as we have no idea whether the previous tab editor
        # set it to something else
        self.undoViewer.setUndoStack(None)

        # we also clear the status bar
        self.statusBar().clearMessage()

        if wdt:
            self.revertAction.setEnabled(True)

            self.saveAction.setEnabled(True)
            self.saveAsAction.setEnabled(True)

            self.closeTabAction.setEnabled(True)
            self.closeOtherTabsAction.setEnabled(True)

            wdt.tabbedEditor.activate()
        else:
            # None is selected right now, lets disable appropriate actions
            self.revertAction.setEnabled(False)

            self.saveAction.setEnabled(False)
            self.saveAsAction.setEnabled(False)

            self.closeTabAction.setEnabled(False)
            self.closeOtherTabsAction.setEnabled(False)

        self.tabs.setUpdatesEnabled(True)
    */
}

void MainWindow::on_tabs_tabCloseRequested(int index)
{
    /*
        wdt = self.tabs.widget(index)
        editor = wdt.tabbedEditor

        if not editor.hasChanges():
            # we can close immediately
            self.closeEditorTab(editor)
            return True

        else:
            # we have changes, lets ask the user whether we should dump them or save them
            result = QtGui.QMessageBox.question(self,
                                                "Unsaved changes!",
                                                "There are unsaved changes in '%s'. "
                                                "Do you want to save them? "
                                                "(Pressing Discard will discard the changes!)" % (editor.filePath),
                                                QtGui.QMessageBox.Save | QtGui.QMessageBox.Discard | QtGui.QMessageBox.Cancel,
                                                QtGui.QMessageBox.Save)

            if result == QtGui.QMessageBox.Save:
                # lets save changes and then kill the editor (This is the default action)
                # If there was an error saving the file, stop what we're doing
                # and let the user fix the problem.
                if not editor.save():
                    return False

                self.closeEditorTab(editor)
                return True

            elif result == QtGui.QMessageBox.Discard:
                # changes will be discarded
                # note: we don't have to call editor.discardChanges here

                self.closeEditorTab(editor)
                return True

            # don't do anything if user selected 'Cancel'
            return False
    */
}

void MainWindow::slot_tabBarCustomContextMenuRequested(const QPoint& pos)
{
    auto tabs = centralWidget()->findChild<QTabWidget*>("tabs");
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

        if atIndex != -1:
            tab = self.tabs.widget(atIndex)
            menu.addSeparator()
            dataTypeAction = QtGui.QAction("Data type: %s" % (tab.getDesiredSavingDataType()), self)
            dataTypeAction.setToolTip("Displays which data type this file will be saved to (the desired saving data type).")
            menu.addAction(dataTypeAction)

        menu.exec_(self.tabBar.mapToGlobal(point))
    */
    menu->exec(tabs->tabBar()->mapToGlobal(pos));
}
