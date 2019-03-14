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

/*
class ProjectSettingsDialog(QtGui.QDialog):
    """Dialog able to change various project settings
    """

    def __init__(self, project):
        super(ProjectSettingsDialog, self).__init__()

        self.ui = ceed.ui.projectsettingsdialog.Ui_ProjectSettingsDialog()
        self.ui.setupUi(self)

        self.baseDirectory = self.findChild(qtwidgets.FileLineEdit, "baseDirectory")
        self.baseDirectory.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode

        self.CEGUIVersion = self.findChild(QtGui.QComboBox, "CEGUIVersion")
        for version in compatibility.CEGUIVersions:
            self.CEGUIVersion.addItem(version)

        self.CEGUIVersion.setEditText(project.CEGUIVersion)

        self.CEGUIDefaultResolution = self.findChild(QtGui.QComboBox, "CEGUIDefaultResolution")
        self.CEGUIDefaultResolution.setEditText(project.CEGUIDefaultResolution)

        self.resourceDirectory = self.findChild(qtwidgets.FileLineEdit, "resourceDirectory")
        self.resourceDirectory.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.resourceDirectoryApplyButton = self.findChild(QtGui.QPushButton, "resourceDirectoryApplyButton")
        self.resourceDirectoryApplyButton.pressed.connect(self.slot_applyResourceDirectory)

        self.imagesetsPath = self.findChild(qtwidgets.FileLineEdit, "imagesetsPath")
        self.imagesetsPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.fontsPath = self.findChild(qtwidgets.FileLineEdit, "fontsPath")
        self.fontsPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.looknfeelsPath = self.findChild(qtwidgets.FileLineEdit, "looknfeelsPath")
        self.looknfeelsPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.schemesPath = self.findChild(qtwidgets.FileLineEdit, "schemesPath")
        self.schemesPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.layoutsPath = self.findChild(qtwidgets.FileLineEdit, "layoutsPath")
        self.layoutsPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode
        self.xmlSchemasPath = self.findChild(qtwidgets.FileLineEdit, "xmlSchemasPath")
        self.xmlSchemasPath.mode = qtwidgets.FileLineEdit.ExistingDirectoryMode

        self.baseDirectory.setText(project.getAbsolutePathOf(""))
        self.imagesetsPath.setText(project.getAbsolutePathOf(project.imagesetsPath))
        self.fontsPath.setText(project.getAbsolutePathOf(project.fontsPath))
        self.looknfeelsPath.setText(project.getAbsolutePathOf(project.looknfeelsPath))
        self.schemesPath.setText(project.getAbsolutePathOf(project.schemesPath))
        self.layoutsPath.setText(project.getAbsolutePathOf(project.layoutsPath))
        self.xmlSchemasPath.setText(project.getAbsolutePathOf(project.xmlSchemasPath))

    def apply(self, project):
        """Applies values from this dialog to given project
        """

        absBaseDir = os.path.normpath(os.path.abspath(self.baseDirectory.text()))
        project.baseDirectory = os.path.relpath(absBaseDir, os.path.dirname(project.projectFilePath))

        project.CEGUIVersion = self.CEGUIVersion.currentText()
        project.CEGUIDefaultResolution = self.CEGUIDefaultResolution.currentText()

        project.imagesetsPath = os.path.relpath(self.imagesetsPath.text(), absBaseDir)
        project.fontsPath = os.path.relpath(self.fontsPath.text(), absBaseDir)
        project.looknfeelsPath = os.path.relpath(self.looknfeelsPath.text(), absBaseDir)
        project.schemesPath = os.path.relpath(self.schemesPath.text(), absBaseDir)
        project.layoutsPath = os.path.relpath(self.layoutsPath.text(), absBaseDir)
        project.xmlSchemasPath = os.path.relpath(self.xmlSchemasPath.text(), absBaseDir)

    def slot_applyResourceDirectory(self):
        resourceDir = os.path.normpath(os.path.abspath(self.resourceDirectory.text()))

        self.imagesetsPath.setText(os.path.join(resourceDir, "imagesets"))
        self.fontsPath.setText(os.path.join(resourceDir, "fonts"))
        self.looknfeelsPath.setText(os.path.join(resourceDir, "looknfeel"))
        self.schemesPath.setText(os.path.join(resourceDir, "schemes"))
        self.layoutsPath.setText(os.path.join(resourceDir, "layouts"))
        self.xmlSchemasPath.setText(os.path.join(resourceDir, "xml_schemas"))
*/
