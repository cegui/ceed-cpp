#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

// The central window of the application

namespace Ui {
class MainWindow;
}

class ProjectManager;
class FileSystemBrowser;
class UndoViewer;
class SettingsDialog;
class RecentlyUsedMenuEntry;
class CEGUIProject;
typedef std::unique_ptr<class EditorBase> EditorBasePtr;
typedef std::unique_ptr<class EditorFactoryBase> EditorFactoryBasePtr;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    QDockWidget* getPropertyDockWidget() const { return propertyDockWidget; }
    EditorBase* getCurrentEditor() const { return currentEditor; }
    QMenu* getEditorMenu() const;
    void setEditorMenuEnabled(bool enabled);
    QToolBar* createToolbar(const QString& name);
    QToolBar* getToolbar(const QString& name) const;

    // Common actions
    QAction* getActionCut() const;
    QAction* getActionCopy() const;
    QAction* getActionPaste() const;
    QAction* getActionDeleteSelected() const;
    QAction* getActionZoomIn() const;
    QAction* getActionZoomOut() const;
    QAction* getActionZoomReset() const;

private slots:

    void openEditorTab(const QString& absolutePath);
    void openRecentProject(const QString& path);
    void openRecentFile(const QString& path);

    void onUndoAvailable(bool available, const QString& text);
    void onRedoAvailable(bool available, const QString& text);
    void onEditorFilePathChanged(const QString& oldPath, const QString& newPath);
    void onEditorContentsChanged(bool isModified);
    void onEditorFileChangedExternally();

    bool on_actionQuit_triggered();
    void on_actionStatusbar_toggled(bool arg1);
    void on_actionOpenProject_triggered();
    void on_actionFullScreen_triggered();
    void on_actionQuickstartGuide_triggered();
    void on_actionUserManual_triggered();
    void on_actionWikiPage_triggered();
    void on_actionSendFeedback_triggered();
    void on_actionReportBug_triggered();
    void on_actionCEGUIDebugInfo_triggered();
    void on_actionAbout_triggered();
    void on_actionLicense_triggered();
    void on_actionQt_triggered();
    void on_actionPreferences_triggered();
    void on_actionProjectSettings_triggered();
    void on_actionNewProject_triggered();
    void on_actionOpenFile_triggered();
    void slot_tabBarCustomContextMenuRequested(const QPoint& pos);
    void on_tabs_currentChanged(int index);
    bool on_tabs_tabCloseRequested(int index);
    void on_actionCloseTab_triggered();
    void on_actionCloseOtherTabs_triggered();
    void on_actionCloseAllTabs_triggered();
    void on_actionPreviousTab_triggered();
    void on_actionNextTab_triggered();
    void on_actionZoomIn_triggered();
    void on_actionZoomOut_triggered();
    void on_actionZoomReset_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionRevert_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionDelete_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionSaveAll_triggered();
    void on_actionSaveProject_triggered();
    bool on_actionCloseProject_triggered();
    void on_actionReloadResources_triggered();
    void on_actionNewLayout_triggered();
    void on_actionNewImageset_triggered();
    void on_actionNewOtherFile_triggered();

private:

    virtual void closeEvent(QCloseEvent* event) override;

    void setupToolbars();
    void setupToolbar(QToolBar* toolBar);

    void updateProjectDependentUI(CEGUIProject* newProject);
    bool confirmProjectClosing(bool onlyModified);
    void loadProject(const QString& path);

    void openNewEditor(EditorBasePtr editor);
    EditorBasePtr createEditorForFile(const QString& absolutePath);
    bool activateEditorTabByFilePath(const QString& absolutePath);
    void closeEditorTab(EditorBase* editor);
    bool closeAllTabsRequiringProject();
    EditorBase* getEditorForTab(int index) const;
    EditorBase* getEditorForTab(QWidget* tabWidget) const;

    Ui::MainWindow* ui;
    QString _title;
    ProjectManager* projectManager = nullptr;
    FileSystemBrowser* fsBrowser = nullptr;
    UndoViewer* undoViewer = nullptr;
    QDockWidget* propertyDockWidget = nullptr;
    SettingsDialog* settingsDialog = nullptr;
    RecentlyUsedMenuEntry* recentlyUsedFiles = nullptr;
    RecentlyUsedMenuEntry* recentlyUsedProjects = nullptr;
    QMenu* docsToolbarsMenu = nullptr;
    QAction* tabsMenuSeparator = nullptr;

    bool wasMaximizedBeforeFullscreen = false;
    bool displayingReloadAlert = false;

    std::vector<EditorFactoryBasePtr> editorFactories;
    QStringList editorFactoryFileFilters;
    std::vector<EditorBasePtr> activeEditors;
    EditorBase* currentEditor = nullptr;
};

#endif // MAINWINDOW_H
