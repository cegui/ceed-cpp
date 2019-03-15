#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class ProjectManager;
class FileSystemBrowser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void openEditorTab(const QString& absolutePath);

    void on_actionQuit_triggered();

    void on_actionStatusbar_toggled(bool arg1);

    void on_actionOpenProject_triggered();

    void on_actionFullScreen_triggered();

    void on_actionQuickstartGuide_triggered();

    void on_actionUserManual_triggered();

    void on_actionWikiPage_triggered();

    void on_actionSendFeedback_triggered();

    void on_actionReportBug_triggered();

    void on_actionCEGUIDebugInfo_triggered();

    void on_tabs_currentChanged(int index);

    void on_tabs_tabCloseRequested(int index);

    void slot_tabBarCustomContextMenuRequested(const QPoint& pos);

    void on_actionAbout_triggered();

    void on_actionLicense_triggered();

    void on_actionProjectSettings_triggered();

    void on_actionNewProject_triggered();

    void on_actionOpenFile_triggered();

private:

    void setupToolbars();
    QToolBar* createToolbar(const QString& name);

    void updateUIOnProjectChanged();
    bool closeAllTabsRequiringProject();

    Ui::MainWindow* ui;
    ProjectManager* projectManager = nullptr;
    FileSystemBrowser* fsBrowser = nullptr;
    bool wasMaximizedBeforeFullscreen = false;
};

#endif // MAINWINDOW_H
