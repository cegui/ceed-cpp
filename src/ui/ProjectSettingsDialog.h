#ifndef PROJECTSETTINGSDIALOG_H
#define PROJECTSETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
class ProjectSettingsDialog;
}

class ProjectSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectSettingsDialog(QWidget *parent = nullptr);
    ~ProjectSettingsDialog();

private:
    Ui::ProjectSettingsDialog *ui;
};

#endif // PROJECTSETTINGSDIALOG_H
