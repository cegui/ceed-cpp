#include "src/ui/ProjectSettingsDialog.h"
#include "ui_ProjectSettingsDialog.h"

ProjectSettingsDialog::ProjectSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectSettingsDialog)
{
    ui->setupUi(this);
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
    delete ui;
}
