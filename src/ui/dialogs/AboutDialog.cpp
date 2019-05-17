#include "src/ui/dialogs/AboutDialog.h"
#include "ui_AboutDialog.h"
#include "src/Application.h"
#include <CEGUI/Version.h>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // Background, see the data/images directory for SVG source
    ui->aboutImage->setPixmap(QPixmap(":/images/splashscreen.png"));

    ui->CEEDDescription->setText("Please report any issues to help this project.");

    ui->CEEDVersion->setText("CEED: " + qApp->applicationVersion());
    ui->QtVersion->setText("Qt: " + QString(qVersion()));
    ui->CEGUIVersion->setText(QString("CEGUI: %1.%2.%3").arg(CEGUI_VERSION_MAJOR).arg(CEGUI_VERSION_MINOR).arg(CEGUI_VERSION_PATCH));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
