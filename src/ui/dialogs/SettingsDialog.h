#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QAbstractButton;
class QDialogButtonBox;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:

    void onButtonBoxClicked(QAbstractButton* button);

private:

    QDialogButtonBox* buttonBox = nullptr;
};

#endif // SETTINGSDIALOG_H
