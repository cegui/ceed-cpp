#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "ProjectManager.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    projectManager = new ProjectManager();
    addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, projectManager);

    //actionStatusbar->setChecked(statusBar()->isVisible());
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
