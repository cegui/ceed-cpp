#include "src/ui/dialogs/LicenseDialog.h"
#include "ui_LicenseDialog.h"

LicenseDialog::LicenseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LicenseDialog)
{
    ui->setupUi(this);
}

LicenseDialog::~LicenseDialog()
{
    delete ui;
}
