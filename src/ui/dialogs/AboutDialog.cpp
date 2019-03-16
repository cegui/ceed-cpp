#include "src/ui/dialogs/AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // Background, see the data/images directory for SVG source
    ui->aboutImage->setPixmap(QPixmap(":/images/splashscreen.png"));

    findChild<QLabel*>()->setText("Please report any issues to help this project.");
/*
    self.findChild(QtGui.QLabel, "CEEDVersion").setText("CEED: %s" % (version.CEED))
    self.findChild(QtGui.QLabel, "PySideVersion").setText("PySide: %s" % (version.PYSIDE))
    self.findChild(QtGui.QLabel, "QtVersion").setText("Qt: %s" % (version.QT))
    self.findChild(QtGui.QLabel, "PyCEGUIVersion").setText("PyCEGUI: %s" % (version.PYCEGUI))
*/
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
