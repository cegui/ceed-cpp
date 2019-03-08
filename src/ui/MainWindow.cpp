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
}

MainWindow::~MainWindow()
{
    delete projectManager;
    delete ui;
}
