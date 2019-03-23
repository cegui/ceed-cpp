#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    ~MainWindow();

    EditorBase* getCurrentEditor() const { return currentEditor; }
    QMenu* getEditorMenu() const;

private slots:

    void openEditorTab(const QString& absolutePath);
    void openRecentProject(const QString& path);
    void openRecentFile(const QString& path);

    void onUndoAvailable(bool available, const QString& text);
    void onRedoAvailable(bool available, const QString& text);
    void onEditorLabelChanged();
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
    QToolBar* createToolbar(const QString& name);

    void updateProjectDependentUI(CEGUIProject* newProject);

    EditorBase* createEditorForFile(const QString& absolutePath);
    bool activateEditorTabByFilePath(const QString& absolutePath);
    void closeEditorTab(EditorBase* editor);
    bool closeAllTabsRequiringProject();
    EditorBase* getEditorForTab(int index) const;
    EditorBase* getEditorForTab(QWidget* tabWidget) const;

    void createNewFile(const QString& title, const QStringList& filters, int currFilter, const QString& autoSuffix);

    Ui::MainWindow* ui;
    ProjectManager* projectManager = nullptr;
    FileSystemBrowser* fsBrowser = nullptr;
    UndoViewer* undoViewer = nullptr;
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
