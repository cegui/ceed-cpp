#include "src/ui/dialogs/ProjectSettingsDialog.h"
#include "ui_ProjectSettingsDialog.h"
#include "src/ui/widgets/FileLineEdit.h"
#include "src/cegui/CEGUIProject.h"
#include "qdir.h"
#include "qfileinfo.h"

ProjectSettingsDialog::ProjectSettingsDialog(CEGUIProject& project, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectSettingsDialog)
{
    ui->setupUi(this);

    CEGUIVersion = findChild<QComboBox*>("CEGUIVersion");
    CEGUIVersion->addItems(CEGUIProject::CEGUIVersions);
    CEGUIVersion->setEditText(project.CEGUIVersion);

    CEGUIDefaultResolution = findChild<QComboBox*>("CEGUIDefaultResolution");
    CEGUIDefaultResolution->setEditText(project.defaultResolution);

    baseDirectory = findChild<FileLineEdit*>("baseDirectory");
    baseDirectory->setMode(FileLineEdit::Mode::ExistingDirectory);
    baseDirectory->setText(project.getAbsolutePathOf(""));

    resourceDirectory = findChild<FileLineEdit*>("resourceDirectory");
    resourceDirectory->setMode(FileLineEdit::Mode::ExistingDirectory);

    imagesetsPath = findChild<FileLineEdit*>("imagesetsPath");
    imagesetsPath->setMode(FileLineEdit::Mode::ExistingDirectory);
    imagesetsPath->setText(project.getAbsolutePathOf(project.imagesetsPath));

    fontsPath = findChild<FileLineEdit*>("fontsPath");
    fontsPath->setMode(FileLineEdit::Mode::ExistingDirectory);
    fontsPath->setText(project.getAbsolutePathOf(project.fontsPath));

    looknfeelsPath = findChild<FileLineEdit*>("looknfeelsPath");
    looknfeelsPath->setMode(FileLineEdit::Mode::ExistingDirectory);
    looknfeelsPath->setText(project.getAbsolutePathOf(project.looknfeelsPath));

    schemesPath = findChild<FileLineEdit*>("schemesPath");
    schemesPath->setMode(FileLineEdit::Mode::ExistingDirectory);
    schemesPath->setText(project.getAbsolutePathOf(project.schemesPath));

    layoutsPath = findChild<FileLineEdit*>("layoutsPath");
    layoutsPath->setMode(FileLineEdit::Mode::ExistingDirectory);
    layoutsPath->setText(project.getAbsolutePathOf(project.layoutsPath));

    xmlSchemasPath = findChild<FileLineEdit*>("xmlSchemasPath");
    xmlSchemasPath->setMode(FileLineEdit::Mode::ExistingDirectory);
    xmlSchemasPath->setText(project.getAbsolutePathOf(project.xmlSchemasPath));
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
    delete ui;
}

// Applies values from this dialog to given project
void ProjectSettingsDialog::apply(CEGUIProject& project) const
{
    QDir absBaseDir(QDir::cleanPath(QDir(baseDirectory->text()).absolutePath()));
    project.baseDirectory = QFileInfo(project.filePath).dir().relativeFilePath(absBaseDir.path());

    project.CEGUIVersion = CEGUIVersion->currentText();
    project.defaultResolution = CEGUIDefaultResolution->currentText();

    project.imagesetsPath = absBaseDir.relativeFilePath(imagesetsPath->text());
    project.fontsPath = absBaseDir.relativeFilePath(fontsPath->text());
    project.looknfeelsPath = absBaseDir.relativeFilePath(looknfeelsPath->text());
    project.schemesPath = absBaseDir.relativeFilePath(schemesPath->text());
    project.layoutsPath = absBaseDir.relativeFilePath(layoutsPath->text());
    project.xmlSchemasPath = absBaseDir.relativeFilePath(xmlSchemasPath->text());
}

void ProjectSettingsDialog::on_resourceDirectoryApplyButton_pressed()
{
    QDir resourceDir(QDir::cleanPath(QDir(resourceDirectory->text()).absolutePath()));

    imagesetsPath->setText(resourceDir.filePath("imagesets"));
    fontsPath->setText(resourceDir.filePath("fonts"));
    looknfeelsPath->setText(resourceDir.filePath("looknfeel"));
    schemesPath->setText(resourceDir.filePath("schemes"));
    layoutsPath->setText(resourceDir.filePath("layouts"));
    xmlSchemasPath->setText(resourceDir.filePath("xml_schemas"));
}
