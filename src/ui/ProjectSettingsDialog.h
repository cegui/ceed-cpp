#ifndef PROJECTSETTINGSDIALOG_H
#define PROJECTSETTINGSDIALOG_H

#include <QDialog>

// Dialog able to change various project settings

namespace Ui {
class ProjectSettingsDialog;
}

class CEGUIProject;
class FileLineEdit;
class QComboBox;

class ProjectSettingsDialog : public QDialog
{
    Q_OBJECT

public:

    explicit ProjectSettingsDialog(CEGUIProject& project, QWidget *parent = nullptr);
    virtual ~ProjectSettingsDialog() override;

    void apply(CEGUIProject& project) const;

private slots:

    void on_resourceDirectoryApplyButton_pressed();

private:

    Ui::ProjectSettingsDialog *ui;

    QComboBox* CEGUIVersion = nullptr;
    QComboBox* CEGUIDefaultResolution = nullptr;

    FileLineEdit* baseDirectory = nullptr;
    FileLineEdit* resourceDirectory = nullptr;
    FileLineEdit* imagesetsPath = nullptr;
    FileLineEdit* fontsPath = nullptr;
    FileLineEdit* looknfeelsPath = nullptr;
    FileLineEdit* schemesPath = nullptr;
    FileLineEdit* layoutsPath = nullptr;
    FileLineEdit* xmlSchemasPath = nullptr;
};

#endif // PROJECTSETTINGSDIALOG_H
