#include "src\ui\NewProjectDialog.h"
#include "ui_NewProjectDialog.h"
#include "qmessagebox.h"
#include "qcheckbox.h"
#include "qfileinfo.h"
#include "qdir.h"

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);

    auto projectFilePath = findChild<FileLineEdit*>("projectFilePath");
    projectFilePath->setFilter("Project file (*.project)");
    projectFilePath->setMode(FileLineEdit::Mode::NewFile);
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

bool NewProjectDialog::isCreateResourceDirsSelected() const
{
   auto createResourceDirs = findChild<QCheckBox*>("createResourceDirs");
   return createResourceDirs->checkState() == Qt::Checked;
}

void NewProjectDialog::accept()
{
    auto projectFilePath = findChild<FileLineEdit*>("projectFilePath");
    QString path = projectFilePath->text();
    if (path.isEmpty())
    {
        QMessageBox::critical(this, "Project file path empty!", "You must supply a valid project file path!");
        return;
    }

    //???create directory instead?
    QDir parentDir = QFileInfo(path).dir();
    if (!parentDir.exists())
    {
        QMessageBox::critical(this, "Project file path invalid!", "Its parent directory ('" + parentDir.path() + "') is inaccessible!");
        return;
    }

    QDialog::accept();
}
