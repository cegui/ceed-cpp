#include "src/ui/ProjectManager.h"
#include "ui_ProjectManager.h"

ProjectManager::ProjectManager(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::ProjectManager)
{
    ui->setupUi(this);
}

ProjectManager::~ProjectManager()
{
    delete ui;
}
