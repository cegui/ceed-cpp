#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <QDialog>

// Shows GPLv3 and related info in the UI of the application as FSF recommends.
// Almost all of it is in the .ui file, editable with QtDesigner

namespace Ui {
class LicenseDialog;
}

class LicenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LicenseDialog(QWidget *parent = nullptr);
    ~LicenseDialog();

private:
    Ui::LicenseDialog *ui;
};

#endif // LICENSEDIALOG_H
