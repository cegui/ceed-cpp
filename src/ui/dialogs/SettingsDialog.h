#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QAbstractButton;
class QDialogButtonBox;
class QTabWidget;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:

    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:

    void onButtonBoxClicked(QAbstractButton* button);

private:

    void checkIfRestartRequired();

    QDialogButtonBox* buttonBox = nullptr;
    QTabWidget* tabs = nullptr;
};

#endif // SETTINGSDIALOG_H
