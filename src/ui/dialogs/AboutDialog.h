#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

// About/Version dialog shown when user selects Help -> About.
// The main goal is to show versions of various things, we can then tell the
// user to just go to this dialog and tell us the versions when something
// goes wrong for them.

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
