#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>

// Dialog responsible for creation of entirely new projects

namespace Ui {
class NewProjectDialog;
}

class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:

    explicit NewProjectDialog(QWidget *parent = nullptr);
    virtual ~NewProjectDialog() override;

    bool isCreateResourceDirsSelected() const;

public slots:

    virtual void accept() override;

private:

    Ui::NewProjectDialog *ui;
};

#endif // NEWPROJECTDIALOG_H
