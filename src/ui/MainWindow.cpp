#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ProjectManager.h"
#include "qtoolbar.h"
#include "qtoolbutton.h"
#include "qfiledialog.h"
#include "qmessagebox.h"
#include "src/proj/CEGUIProjectManager.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    projectManager = new ProjectManager();
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, projectManager);

    //actionStatusbar->setChecked(statusBar()->isVisible());

    setupToolbars();
}

MainWindow::~MainWindow()
{
    delete projectManager;
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_actionStatusbar_toggled(bool isChecked)
{
    statusBar()->setVisible(isChecked);
}

void MainWindow::setupToolbars()
{
    // Here until I manage to create menu toolbutton in Qt Creator
    QToolBar* toolbar = createToolbar("Standard");
    QToolButton* newMenuBtn = new QToolButton(this);
    newMenuBtn->setText("New");
    newMenuBtn->setToolTip("New file");
    newMenuBtn->setIcon(QIcon("../data/icons/actions/new_file.png"));
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
        CEGUIProjectManager::Instance().loadProject(fileName);
}
